/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "exec_parser.h"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)

static so_exec_t *exec;
struct sigaction old_handle;
struct sigaction new_handle;
int file = -1;

int contains(int low, int high, int val)
{
	return val < high && val >= low;
}

static void my_handler(int signum, siginfo_t *info, void *context)
{
	int page_size = getpagesize();
	int i = 0;
	so_seg_t *segment = NULL;
	struct_pages *pages = NULL;
	void *res = NULL;
	int ret = 0;
	int upper_bound = 0;
	int lower_bound = 0;
	int page_offset = 0;
	int current_total_size = 0;
	int needs_handle = 0;

	/* Find the page from the segment that contains the faulty adderss */
	for (i = 0; i < exec->segments_no; i++) {
		segment = &exec->segments[i];
		pages = (struct_pages *)segment->data;

		/* Get the bounds of the current segment */
		lower_bound = segment->vaddr;
		upper_bound = lower_bound + segment->mem_size;

		/* Check if the address is in the current segment */
		if (contains (lower_bound, upper_bound, (int)info->si_addr)) {
			/* Get the position of the page in the segment */
			page_offset = ((int)info->si_addr -
				segment->vaddr) / page_size;

			/* If the page is mapped */
			if (pages->appears[page_offset] != 0)
				break;

			needs_handle = 1;
			break;
		}
	}

	/* Handle the page that needs handling, if any */
	if (needs_handle) {
		/* Get the total size of all current pages */
		current_total_size = (((int)info->si_addr -
			segment->vaddr) / page_size) * page_size;

		/* Map new page */
		switch (current_total_size < segment->file_size) {

		case 0:
			res = mmap(((void *) segment->vaddr +
				current_total_size),
				page_size,
				PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				file,
				0);
			DIE(res == NULL, "mmap failure!\n");
			break;
		case 1:
			res = mmap(((void *) segment->vaddr +
				current_total_size),
				page_size,
				PROT_WRITE,
				MAP_PRIVATE | MAP_FIXED,
				file,
				segment->offset +
				current_total_size);
			DIE(res == NULL, "mmap failure!\n");
			break;
		}

		/* Clear junk from memory area of new page */
		if ((current_total_size + page_size >=
				segment->file_size)) {

			/* Compute the bounds of the page */
			lower_bound = (segment->vaddr +
				segment->file_size);
			upper_bound = ((int)(info->si_addr -
				segment->vaddr)) / page_size + 1;
			upper_bound = upper_bound * page_size -
				segment->file_size;

			memset((void *)lower_bound,
				0,
				upper_bound);
		}

		/* Give permissions */
		mprotect(res, page_size, segment->perm);
		DIE(ret != 0, "mprotect failure!\n");

		/* Mark as mapped */
		page_offset = ((int)info->si_addr - segment->vaddr) /
			page_size;
		pages->appears[page_offset] = 1;
	} else {
		/* Call old handler */
		old_handle.sa_sigaction(signum, info, context);
	}
}

struct_pages *init_pages(int no_pages)
{
	struct_pages *pages = NULL;

	/* Initiate array to keep track of mapped pages for each segment */
	pages = (void *)malloc(sizeof(struct _struct_pages));
	DIE(pages == NULL, "Malloc failure!\n");
	pages->size = no_pages;
	pages->appears = (int *)calloc(pages->size, sizeof(int));
	DIE(pages->appears == NULL, "Calloc failure!\n");

	return pages;
}

int so_init_loader(void)
{
	int res = 0;

	/* Empty mask to catch signals */
	res = sigemptyset(&(new_handle.sa_mask));
	DIE(res != 0, "sigemptyset failure!\n");

	/* Set mask to catch SIGSEGV */
	res = sigaddset(&new_handle.sa_mask, SIGSEGV);
	DIE(res != 0, "sigaddset failure!\n");

	/* Set custom function to be called by handler */
	new_handle.sa_sigaction = my_handler;

	/* Specify the handlers for the signals */
	res = sigaction(SIGSEGV, &new_handle, &old_handle);
	DIE(res != 0, "sigaddset failure!\n");

	return -1;
}

int so_execute(char *path, char *argv[])
{
	int i = 0;
	so_seg_t *segment = NULL;

	/* Parse the executable */
	exec = so_parse_exec(path);
	DIE(!exec, "Open failure!\n");

	/* Open file for reading */
	file = open(path, O_RDONLY, 0646);
	DIE(open < 0, "Open failure!\n");

	/* Init segments */
	for (i = 0; i < exec->segments_no; i++) {
		segment = &exec->segments[i];

		/* Initiate a new structure to keep
		 * track of the pages and store the data
		 * about the pages in the memory segment
		 */
		segment->data = init_pages(segment->mem_size /
					getpagesize() + 1);
	}

	/* Start the executable */
	so_start_exec(exec, argv);

	return 0;
}
