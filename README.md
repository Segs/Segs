SEGS - COMPILATION GUIDE
======
SEGS is the Super Entity Game Server. It is a free, open source (under the terms of the BSD License), cross-platform, and object-oriented suite of protocols and services designed to interface with popular super hero themed MMORPG clients. SEGS is written in C++ and facilitates common MMORPG functionality such as account and character storage to a database, account and character retrieval from a database, secure client authentication, client movement in a virtual world, and client chat.

For legal information, please see License.txt.

For a list of SEGS authors and contributors, please see Authors.txt.

* Our [wiki](https://github.com/Segs/Segs/wiki)
* Our [redmine](http://redmine.nemerle.eu/projects/segs "redmine")
* Our target [coding style](http://github.com/Segs/Segs/blob/master/docs/CodingStyle.txt.md "style")

Basic instructions for compiling SEGS in Linux and Windows are below, however for more detailed instructions, visit https://segs.nemerle.eu/developers

CONTENTS OF THIS FILE
------
- Requirements and Notes
- Compile in Linux
- Compile in Windows
- Contribute to Development
- Setting up SEGS Server and Playing
- More information
- FAQs


REQUIREMENTS AND NOTES
------

SEGS requires:

Below are the utilities and libraries you'll need to compile SEGS under Linux. These are available via most distro's package manager, or via the links below:

   - QT >5.8 - https://www.qt.io/download-qt-for-application-development
   - CMake >3.6 - https://cmake.org/download/
   - Git - https://git-scm.com/download
   - Boost - A set of portable C++ libraries that SEGS makes use of.
     Available here: http://sourceforge.net/project/showfiles.php?group_id=7586
     
     Additional information on setting up a development environment on Linux can be found here: 

For more detailed version of this guide or forums for help see the links below:
- https://segs.nemerle.eu/
- https://segs.nemerle.eu/developers
- https://segs.nemerle.eu/forumz/index.php

To report any bugs, please visit our github:
- https://github.com/Segs/Segs

Note that all directories mentioned in this document are always relative to the directory of your SEGS installation, and commands are meant to be run from this directory (except for the initial commands that create that directory).


COMPILE SEGS IN LINUX
------

1. Install Dependencies

    The libraries you'll need to install are listed below and are availble from your distro's package manager, or via the links below:

   - QT >5.8 - https://www.qt.io/download-qt-for-application-development
   - CMake >3.6 - https://cmake.org/download/
   - Git - https://git-scm.com/download


COMPILE SEGS IN WINDOWS
------

1. Install Dependencies


CONTRIBUTE TO DEVELOPMENT
------

At this point you're ready to start the server by running the program `authserver`. Once running, start the CoX client we setup earlier and log in using any username and password combination (users are autocreated).

Enjoy!


SETTING UP SEGS SERVER & PLAYING
------

After compiling, you'll need to setup your SEGS Server and Databases. You'll also need to download a specific version of the client compatible with SEGS.

**IMPORTANT:** You can find information on setting up your SEGS server and logging in by reading the README.md located in your output directory (typically `out`). You can also read a copy of that [README.md here](./Projects/CoX/docs/README.md)

- Don't forget! You'll need a copy of the required CoH client from Issue 0 (release) version 0.22, which can be found through various sources online, or via magnet link: 
  `magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f`


MORE INFORMATION
------

Additional information will be available here, including optional steps to take after configuration and where to find additional help.


FAQs
------

Frequently asked questions will go here.
