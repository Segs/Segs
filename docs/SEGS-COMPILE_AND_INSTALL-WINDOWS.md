BUILD AND INSTALL SEGS ON WINDOWS
------
- [Requirements and Notes](#requirements-and-notes)
- [Install C++ Tool Chain](#install-c-tool-chain)
- [Compile SEGS](#build-segs)
- [Prepare SEGS Server](#prepare-segs-server)
- [Configure SEGS Server with SEGSADMIN](#configure-segs-using-segsadmin)

REQUIREMENTS AND NOTES
------

Below are the utilities and libraries you'll need to compile SEGS in any environment. While it may be possible to use another toolset, the C++ Tool Chain below is the only one officially supported by the SEGS team. These packages are available for both Linux or Windows via the links below:

   - **QT 5.9+** - A cross platform application framework utilized heavily by SEGS. http://download.qt.io/archive/qt/

   - **CMake 3.6+** - CMake is the cross-platform make utility. It generates makefiles for multiple platforms. https://cmake.org/download/
   - **Git** - A version control system for tracking changes in computer files and coordinating work on those files among multiple people. https://git-scm.com/download

**ADVANCED:** By default SEGS uses a local SQLite database created by `SEGSAdmin` or `dbtool`, but optionally may be configured to use another database driver by manually configuring settings.cfg. SEGS is compatible with PostGreSQL or MySQL/MariaDB. These alternate configurations are optional and unsupported. YMMV.
   - _OPTIONAL:_ PostGreSQL - http://www.postgresql.org/download/
   - _OPTIONAL:_ MySQL - http://www.mysql.org/downloads/mysql

Note that all directories mentioned in this document are always relative to the directory of your SEGS installation, and commands are meant to be run from this directory (except for the initial commands that create that directory).


INSTALL C++ TOOL CHAIN
------

**1. Installing**

Download dependencies and tools by clicking on the links listed in the Requirements section above. Make sure you download the correct version for your installation of Windows. Once downloaded, navigate to your downloads folder and run each installer.

:exclamation: NOTE: for QT, please install **mingw 32bit version**


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

PREPARE SEGS SERVER
------

After compiling, you'll need to setup your SEGS Server and Databases. You'll also need to download a specific version of the client compatible with SEGS.

In order to run SEGSAdmin, certain OpenSSL DLLs are required: `libeay32.dll` and `ssleay32.dll`. These must be placed in the directory that the `SEGSAdmin.exe` application resides, which will typically be your output directory. If you do not have them, you can download them here: https://slproweb.com/products/Win32OpenSSL.html. Depending on whether your system is 32-bit or 64-bit, you will either download `Win32 OpenSSL v1.0.2p Light` or `Win64 OpenSSL v1.0.2p Light`. Simply navigate to the directory you install it to, and copy/paste the necessary DLLs to the output directory.

**IMPORTANT:** You can find information on setting up and running your SEGS server by reading the README.md located in your output directory (typically `out`).  

- Don't forget! You'll need a copy of the required CoH client from Issue 0 (release) version 0.22, which can be found through various sources online, or via magnet link:
  `magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f`

Configure SEGS using SEGSADMIN
------

1. **Run SEGSAdmin**  

   In your SEGS folder locate and run `SEGSAdmin`. This is the central setup and administration tool for SEGS.  
   From this tool, you can install and configure SEGS, and start or stop your server.  


2. **Server Setup**  

   In the SEGSAdmin window, there is a section titled `Server Setup`.  
   To setup your copy of SEGS click each of the following buttons and follow the on screen instructions.  
   If setup is successful, a green checkmark will appear next to the button.  


   1. **Generate Config File**  

      Clicking this button will open a dialog box to configure the settings.cfg file SEGS needs to run.  
      It will ask you to enter a name for your server and the IP address of your computer.  

      > This will be the same IP you used during Section 2 of 'Configure The COX Client'

      Next to the IP address field, clicking a small button with a magnifying glass will look up your computer's IP address.  


   2. **Create New Databases**  

      Clicking this button will create the `segs` and `segs_game` SQL database files that SEGS needs to run.  
      It will also ask you to enter a username and password for your Admin account.  


   3. **Setup CoH Data Files**  

      SEGS requires several CoH files to run. Clicking this button will prompt you to enter the path to your CoX directory.  
      Once selected, it will copy several files from the CoX directory, and extract a portion of those files to your `SEGS/data/` subdirectory.  
      These `.bin` files are required by SEGS to load CoX maps, and spawn NPCs. Once completed, SEGSAdmin will clean up any unnecessary files.  

3. **Ensure proper ports are open on any firewalls**  

    Make sure the following Network Ports are open: TCP 443, TCP 2104, TCP 2016, and UDP 7000-7200  
    These are the same ports that the original game client and now, SEGS uses as well.  

4. **(Optional) Add User Account**  

   You may elect to add additional user accounts by clicking this button on the left side of the SEGSAdmin window.  
   SEGSAdmin will prompt you to enter a username, password, and access level for each account.  
   A user's access level is used to determine their permissions within your server. Please reference this chart:

   | Access Level    | Permissions                          |
   | -------------   | ------------------------------------ |
   | 0               | basic user with no special authority |
   | 1               | game master, has limited authority   |
   | 2-8             | reserved for future use              |
   | 9               | administrator with full access       |



START THE SERVER AND PLAY
------

At this point you're ready to start the server by clicking the `Start Server` button in SEGSAdmin.  
Once running, start the CoX client and log in using your username and password.

Enjoy!


HELP AND MORE INFORMATION
------

Make sure the following Network Ports are open: TCP 443, TCP 2104, TCP 2016, and UDP 7000-7200  
For Help with installation and configuration of your local SEGS, please see visit us on discord at https://discord.segs.io/


FAQs
------

Visit our FAQ at https://github.com/Segs/Segs/blob/master/docs/FAQ.md

