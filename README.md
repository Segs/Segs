
<img src="docs/segs-medallion-med.png" align="right" alt="SEGS Logo">

[![master Build Status](https://travis-ci.org/Segs/Segs.svg)](https://travis-ci.org/Segs/Segs) 
[![master Build Status](https://ci.appveyor.com/api/projects/status/github/segs/segs?svg=true)](https://ci.appveyor.com/project/nemerle/Segs)
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/SEGS/segs.svg)](http://isitmaintained.com/project/SEGS/segs "Average time to resolve an issue")
[![Percentage of issues still open](http://isitmaintained.com/badge/open/SEGS/segs.svg)](http://isitmaintained.com/project/SEGS/segs "Percentage of issues still open")

SEGS - Super Entity Game Server
======
SEGS is the Super Entity Game Server. It is a free, open source (under the terms of the BSD License), cross-platform, and object-oriented suite of protocols and services designed to interface with popular super hero themed MMORPG clients. SEGS is written in C++ and facilitates common MMORPG functionality such as account and character storage to a database, account and character retrieval from a database, secure client authentication, client movement in a virtual world, and client chat.

For legal information, please see [LICENSE.md](./LICENSE.md).

For a list of SEGS authors and contributors, please see [AUTHORS.md](./docs/AUTHORS.md).

**:exclamation: LOOKING FOR INSTALLATION INSTRUCTIONS? :exclamation:**
> NOTICE: This guide is for COMPILING SEGS FROM SOURCE. If you're looking for help installing and configuring SEGS from a release package, please view [the server installation README.md here](./Projects/CoX/docs/README.md) 

Some other useful links:
* Our [Discord](https://discord.segs.io/)
* Our [wiki](https://github.com/Segs/Segs/wiki)
* The [Issue Queue](https://github.com/Segs/Segs/issues)
* How Can I Help? [Specific Tasks to Help](https://github.com/Segs/Segs/issues/519)
* Our target [coding style](./docs/CONTRIBUTING.md#coding-styleguide)

Basic instructions for compiling SEGS in Linux and Windows are below, however for more detailed visual instructions, visit https://segs.io/developers


CONTENTS OF THIS FILE
------
- [Requirements and Notes](#requirements-and-notes)
- [Install C++ Tool Chain](#install-c-tool-chain)
- [Compile SEGS](#build-segs)
- [Contribute to Development](#contribute-to-development)
- [Setting up SEGS Server and Playing](#setting-up-segs-server-and-playing)
- [More Information](#help-and-more-information)
- [FAQs](#faqs)


REQUIREMENTS AND NOTES
------

Below are the utilities and libraries you'll need to compile SEGS in any environment. While it may be possible to use another toolset, the C++ Tool Chain below is the only one officially supported by the SEGS team. These packages are available for both Linux or Windows, via your distro's package manager, or via the links below:

   - **QT 5.9+** - A cross platform application framework utilized heavily by SEGS. http://download.qt.io/archive/qt/
   
   - **CMake 3.6+** - CMake is the cross-platform make utility. It generates makefiles for multiple platforms. https://cmake.org/download/
   - **Git** - A version control system for tracking changes in computer files and coordinating work on those files among multiple people. https://git-scm.com/download  

**ADVANCED:** By default SEGS uses a local SQLite database created by `SEGSAdmin` or `dbtool`, but optionally may be configured to use another database driver by manually configuring settings.cfg. SEGS is compatible with PostGreSQL or MySQL/MariaDB. These alternate configurations are optional and unsupported. YMMV.
   - _OPTIONAL:_ PostGreSQL - http://www.postgresql.org/download/
   - _OPTIONAL:_ MySQL - http://www.mysql.org/downloads/mysql

For more detailed version of this guide or forums for help see the links below:
- https://segs.io
- https://segs.io/developers
- https://forum.segs.io/

To report any bugs, please visit our github:
- https://github.com/Segs/Segs

Note that all directories mentioned in this document are always relative to the directory of your SEGS installation, and commands are meant to be run from this directory (except for the initial commands that create that directory).


INSTALL C++ TOOL CHAIN
------

**1. Installing in Windows**

Download dependencies and tools by clicking on the links listed in the Requirements section above. Make sure you download the correct version for your installation of Windows. Once downloaded, navigate to your downloads folder and run each installer.

:exclamation: NOTE: for QT, please install **mingw 32bit version**

**2. Installing in Linux**

Install all dependencies and tools by opening a terminal and typing:

**Ubuntu**
```
sudo apt-get install build-essential git qt5 qt5-devel 
```
**Fedora 26+**
```
sudo dnf install cmake gcc gcc-c++ git-core kernel-devel qt5 qt5-devel  
```
**CentOS/RHEL 6/7**
```
yum install git centos-release-scl-rh
yum install devtoolset-6-gcc-c++
source /opt/rh/devtoolset-6/enable
```

Your distro may come with some or all of these packages pre-installed, and while most versions of these libraries will work, you must have version 5.8+ of Qt installed. CentOS and RHEL are two such distributions that require downloading a more recent version of CMake and Qt and potentially having to build from source.

**3. Installing in FreeBSD 11.x**

```
portsnap fetch update
pkg update
pkg install cmake git
cd /usr/ports/devel/qt5-core && make install clean BATCH=yes && cd ../../databases/qt5-sql && make install clean BATCH=yes && cd ../qt5-sqldrivers-sqlite3 && make install clean BATCH=yes && cd ../../x11-toolkits/qt5-gui && make install clean BATCH=yes && cd ../qt5-widgets && make install clean BATCH=yes && cd ../../graphics/qt5-opengl && make install clean BATCH=yes
```

FreeBSD should have clang/llvm already installed.


BUILD SEGS
------

**1. Start QTCreator and Clone Git Repository**

Start QTCreator and select New Project. The _New Project_ dialog box will appear. From the list on the left, select `Import Project`, and from the middle column `Git Clone`, then press the `Choose...` button at the bottom right of the dialog window.
![New Project Dialog Screenshot](https://segs.io/user/pages/02.developers/newProject.png "New Project Dialog Screenshot")

The dialog box will change to show the _Git Clone_ dialog, which contains configuration options for importing a project from Git. 

> Git is the version control system used by the SEGS Project. SEGS uses the popular Github website to store it's git repository. More information on GitHub and working with git can be found here: https://guides.github.com/

In the new Git Clone dialog box enter in the GitHub repository for SEGS `https://github.com/Segs/Segs.git`, and select the local folder where you'd like to store  your copy of the SEGS repository.

Select `Next` to clone a copy of the SEGS repo, and when completed select `Finish` to complete this step.


**2. Configure Project**

Next, QTCreator will ask you to configuration your project. This is where you'll select which development _kit_ to use and define the paths for temporary build files.

> QTCreator uses kits to group various settings for building and running projects.

Begin by selecting MinGW 32bit.
![Project Configuration](https://segs.io/user/pages/02.developers/ProjectConfiguration.png "Project Configuration")

You may select the `Details` button next to MinGW if you'd like to customize the paths where QTCreator will store temporary build files.

Finally, click `Configure Project`.

> You may change your configuration, or select additional build options by selecting `Projects` identified by the wrench icon located at the far left of QTCreator. From here you can modify settings to build the COX Map Viewer and enable the scripting engine.

**3. Build All**

If everything went well, you'll see the SEGS file tree in the _Projects Pane_ of QTCreator.
![Projects Pane](https://segs.io/user/pages/02.developers/ConfiguredProject.png "Projects Pane")

Now, select `Build > Build All` from the menu at the top of QTCreator. The bottom half of the window will show compiling output as SEGS builds. The first time you build SEGS it may take several minutes and the output window may show hundreds of warnings, but if compiled successfully, you'll see the following in the _Compile Output_ window.
```
[100%] Built target authserver
00:00:00: The process 'cmake' exited normally.
```

**ADVANCED: Doing it the CLI Way**

If you're on a headless Linux or FreeBSD server, run the following commands:
```
git clone https://github.com/Segs/Segs.git
mkdir Segs/bld
cd Segs/bld
cmake ..
make
```

**ADVANCED: Using Build Scripts**

If you're using docker, run the following commands

```
cp activate.env.template activate.env
source activate.env # set ENV variables and add ./scripts to PATH

docker-build Dockerfile # build the docker image (only needed once)
docker-up # Ctrl-D to run in background

docker-forward segs bootstrap # set up cmake / clean build
docker-forward segs build # build all components
docker-forward segs build gameserver_lib # build a single component
```

This will also run on Ubuntu 18.04 with the following:

```
cp activate.env.template activate.env
source activate.env # set ENV variables and add ./scripts to PATH

sudo segs install_deps_ubuntu # install development tools (only needed once)

segs bootstrap # set up cmake / clean build
segs build # build all components
segs build gameserver_lib # build a single component
```

Note: if you choose not to `source activate.env`, you can run the commands via `scripts/segs` instead


CONTRIBUTE TO DEVELOPMENT
------

**SEGS needs your help!** From editing README's like this one, to coding the MapServer, there are tasks that can be tackled by contributors of all skill level!

Please read [CONTRIBUTING.md](./docs/CONTRIBUTING.md) and see the links below to begin:

* Our GitHub: https://github.com/Segs/Segs
* The Issue Queue: https://github.com/Segs/Segs/issues
* How Can I Help Issue: https://github.com/Segs/Segs/issues/519
* Our target [coding style](./docs/CONTRIBUTING.md#coding-styleguide))
* Join us on [Chat!](./docs/CONTRIBUTING.md#i-dont-want-to-read-this-whole-thing-i-just-have-a-question)


SETTING UP SEGS SERVER AND PLAYING
------

After compiling, you'll need to setup your SEGS Server and Databases. You'll also need to download a specific version of the client compatible with SEGS.

In order to run SEGSAdmin, certain OpenSSL DLLs are required: `libeay32.dll` and `ssleay32.dll`. These must be placed in the directory that the `SEGSAdmin.exe` application resides, which will typically be your output directory. If you do not have them, you can download them here: https://slproweb.com/products/Win32OpenSSL.html. Depending on whether your system is 32-bit or 64-bit, you will either download `Win32 OpenSSL v1.0.2p Light` or `Win64 OpenSSL v1.0.2p Light`. Simply navigate to the directory you install it to, and copy/paste the necessary DLLs to the output directory.	

**IMPORTANT:** You can find information on setting up and running your SEGS server by reading the README.md located in your output directory (typically `out`). You can also read a copy of that [README.md here](./Projects/CoX/docs/README.md)

- Don't forget! You'll need a copy of the required CoH client from Issue 0 (release) version 0.22, which can be found through various sources online, or via magnet link:  
  `magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f`


HELP AND MORE INFORMATION
------

For Help with installation and configuration of your local SEGS, please see visit us on discord at https://discord.segs.io/


FAQs
------

Visit our FAQ at https://github.com/Segs/Segs/blob/master/docs/FAQ.md
