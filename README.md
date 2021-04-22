Nume: Olaru Gabriel Iulian

Grupă: 334CC

# Assignment 1 SO LOADER

General Structure
-

***Code structure***
1. The goal of the project is to implement a loader which supports the Page-on-Demand mechanism.
2. The implementation is platform dependant.There is support for Windows (win32 SDK for windows 7) and Linux.

* ├── exec
* │   └── exec.c
* ├── loader
* │   ├── debug.h
* │   ├── exec_parser.c
* │   ├── exec_parser.h
* │   ├── loader.c
* │   └── loader.h
* ├── Makefile
* ├── Makefile.example
* ├── README.md
* └── test_prog
*    └── hello.S

The main Logic is implemented in loader.c

***Flow***

When an executable tries to access addresses from pages that have not been loaded in memory yet,
a SIGSEGV signal will be triggered. When the signal will get cought, the loader will handle it
by loading a new page in memory.

In order to catch the SIGSEGV signals:
* A mask is set to catch the signal in the init function.
* A handler is set to act when the signal is cought.

In order to load a new page:
* The segment which contains the faulty address will be found.
* The page containing the address will be found.
  * if the page is already mapped, no new page will be mapped (the error is caused by permissions).
* A beginning offset for the new page will be calculated.
* A new page will be mapped, with the required permissions.

Implementation Details
-

All the required features were Implemented.

In order to keep track of the pages of each memory segment, a structure was used:
* The first field is an array that keeps track of which pages were already mapped.
* The second field is the number of pages each segment has.

Function | Details
------------ | -------------
so_init_loader | set up the mask to catch sigsegv signals and the handler
so_execute | Initialize all memory segments for an executable and open the file
init_pages | Initialize structure mentioned above
my_handler | Haandle the SIGSEGV signal in the manner described above

Fun Facts:
  * C89 sucks.
  * VSCode has a nice feature for turning spaces to tabs
  * On windows, \r\n characters are not supported :(

How to build?
-

The makefile was provided along side the code skelet.
To compile the library: make
To compile a test program make -f Makefile.example

Bibliography
-

* Useful Links
    * https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-04
    * https://ocw.cs.pub.ro/courses/so/curs/virt-mem
    * http://csapp.cs.cmu.edu/2e/ch9-preview.pdf

Git
-

1. https://github.com/IulianOlaru249/SO_2020_2021_Homework_3/
