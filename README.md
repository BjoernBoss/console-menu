# Console Menu
![C++](https://img.shields.io/badge/language-c%2B%2B-blue?style=flat-square)
[![License](https://img.shields.io/badge/license-BSD--3--Clause-brightgreen?style=flat-square)](LICENSE.txt)

Simple and small library to construct and manage a page based interface in the console.

Primary use case is to build more complex interfaces in the console and being able to structure/navigate these in a tree like manner.  Additional features include being able to navigate the interface when not focused (optional) as well as repeatedly updating a page until input has been detected. This library currently only works under Windows, as it uses Windows specific system calls to implement the additional features. 

## Preparing/Compiling the library
This library uses premake5 as its build system. Clone the repository into your project, include the library in your premake5.lua file, and link against it. 

The lua file checks for some variables to determine the locations for its file generation:

    rootLocationPath: Location of the actual build files (if variable is undefined, defaults to ./build)
    rootTargetDir:    Location of the target binary files (if variable is undefined, defaults to ./bin/{config})
    rootObjectDir:    Location of the intermediate object files (if variable is undefined, defaults to ./build/{config})

An alternative to using premake5 is to just include the files into the project:

    console-menu.h
    console-menu.cpp

No configuration is needed, appart from using C++17.

## Using the library
In order to use the library the `menu::Instance` interface needs to be implemented:

    menu::Instance
        /* called on starting of the menu session */
        - virtual void init();

        /* called on termination of the menu session */
        - virtual void teardown();

        /* called to identify the root page to be used as entry point by menu::Host */
        - virtual const char* root() = 0;

This interface describes one active menu session.
Every page in your interface needs to implement the `menu::Page` interface:

    menu::Page:
        /* called on first attachment of page to menu::Host */
        - virtual void init();

        /* called on release of page from menu::Host */
        - virtual void teardown();

        /* called when page is opened into the active page stack */
        - virtual void load();

        /* called when page is being removed from active page stack */
        - virtual void unload();

        /* called if the page implements logic between frames, needs to be activated in the menu::Layout (will be called until it returns false or the user has selected an option) */
        - virtual bool update();

        /* called to construct the actual layout of the page */
        - virtual menu::Layout construct() = 0;

        /* called to evaluate the selection of the user */
        - virtual menu::Behavior evaluate(EntryId id) = 0;

Afterwards a `menu::Host` object needs to be constructed, which will run the menu:

    menu::Host* host = menu::Host::acquire(false);

    MyInterface interface;
    host->run(&interface);

    host->release();

## Example of the layout of one page
    +------------------------ some header -------------------------+
     Some textual information
     This is still part of that information
    +--------------------------------------------------------------+
      [00] - exit
      [01] - return
      [02] - option 1
      [03] - option 2
      [04] - option 3
      [05] - another option
    +--------------------------------------------------------------+

     Response from the last Page.
     This is some result...
    
    select an option (CTRL + NUMPAD; abort: '+'; backspace: '-'):