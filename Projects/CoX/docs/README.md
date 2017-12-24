SEGS - INFO & INSTALLATION GUIDE
======

CONTENTS OF THIS FILE
------
- About SEGS
- Requirements and Notes
- Installation
- Setup Database
- Configuration
- Start the Server and Play
- More information
- FAQs


ABOUT SEGS
------
SEGS is the Super Entity Game Server. It is a free, open source (under the terms of the BSD License), cross-platform, and object-oriented suite of protocols and services designed to interface with popular super hero themed MMORPG clients. SEGS is written in C++ and facilitates common MMORPG functionality such as account and character storage to a database, account and character retrieval from a database, secure client authentication, client movement in a virtual world, and client chat.

For legal information, please see License.txt.

For a list of SEGS authors and contributors, please see Authors.txt.


REQUIREMENTS AND NOTES
------

SEGS requires:

- A copy of the CoH client from Issue 0 (release) version 0.22. This can be found through various sources online, or via magnet link: 
  `magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f`

For more detailed version of this guide or forums for help see the links below:
- https://segs.nemerle.eu/
- https://segs.nemerle.eu/developers
- https://segs.nemerle.eu/forumz/index.php

To report any bugs, please visit our github:
- https://github.com/Segs/Segs

Note that all directories mentioned in this document are always relative to the directory of your SEGS installation, and commands are meant to be run from this directory (except for the initial commands that create that directory).


INSTALLATION
------

1. Choose one of two methods to download SEGS.

    a. Download and compile from source. (recommended)

    SEGS will compile in either Linux or Windows. The libraries you'll need to install are listed below and are availble from your distro's package manager, or via the links below:

   - QT >5.8 - https://www.qt.io/download-qt-for-application-development
   - CMake >3.6 - https://cmake.org/download/
   - Git - https://git-scm.com/download

    For detailed instructions, visit https://segs.nemerle.eu/developers
   
    b. Download and extract SEGS from a stable release.

    You can obtain the latest stable SEGS release from https://github.com/Segs/Segs/releases
   
    _Files are available in .tar.gz and .zip formats and can be extracted using most compression tools._

    To download and extract the files on a typical Unix/Linux command line, use the following commands:

    ```
    wget https://github.com/Segs/Segs/releases/segs-latest.tar.gz
    tar -zxvf segs-latest.tar.gz
    ```

    This will create a new directory segs/ containing all SEGS files and directories. Then, proceed to the section of this README titled _Configuration_.

2. Install CoX Client version 0.22.

   SEGS will only communicate with the CoH client from Issue 0 (release) version 0.22. This can be found through various sources online, or via magnet link:

   ```
   magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f
   ```

   After the download completes, navigate to the download folder and run CoX-Setup.exe. Follow the prompts to install the Issue 0 CoX client.

3. Copy the PIGG files to SEGS.

   The CoX client uses files ending in `.pigg` to store important game information. SEGS uses these same files to run the game world.
   
   Once installed, navigate to the SEGS folder. If you compiled from source, this will be in the `/out/` folder wherever you saved your Qt project. If you downloaded a release version, this will be wherever you extracted it to. Create a new folder and name it `data`.
   
   Navigate to the CoX folder and copy the contents of the `/piggs/` subfolder to the `/data/` directory of SEGS.

4. Copy over required DLL files to SEGS.

   NOTE: This step is only required on Windows.

   The utilities present in the SEGS folder require several `.dll` files to operate. Navigate to your Qt installation folder and follow this path: `./Qt/YOUR_VERSION_NUMBER/mingw53_32/bin/`. Copy the following files over to the SEGS folder:
   
   - libgcc_s_dw2-1.dll
   - libstdc++-6.dll
   - libwinpthread-1.dll
   - Qt5Core.dll
   - Qt5Gui.dll
   - Qt5Network.dll
   - Qt5Sql.dll
   - Qt5Widgets.dll

5. Using the piggtool located in the main SEGS folder, extract the BIN files from bin.pigg.

   SEGS requires that we extract bin.pigg to the `/data/bin/` folder. Open a command prompt console. Using the `cd` command, navigate to the SEGS folder, which should contain `piggtool.exe`. To confirm you are in the correct directory, simply enter `piggtool` in the console. If you receive a help menu about its usage, you are in the correct folder. Type the following into the console and press Enter:

   ```
   piggtool -x ./data/bin.pigg
   ```
   
   You should then see a folder called `bin` be created in your SEGS directory. Simply move it into the `data` folder.
   
6. Point the CoX client towards your server.

   Run `CoX-Auth.exe` from the CoX directory and set the IP address to that of your server. If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network.

7. Bypass the CohUpdater requirement.

   The Issue 0 client requires that you launch the game with a program called `CohUpdater.exe`. To bypass this, create a shortcut to CoX.exe and place it wherever you'd like. Access its properties, and in the `Target:` field, add the following: `-project SEGS`.
   
   If you would like to run the client in windowed mode for debugging purposes, add these optional instructions: `-fullscreen 0 -screen 1024 768`.
   
   When you want to launch the client, you will need to use this shortcut.

8. Continue Server Setup
   
   The client is now ready for play, but continue the server setup by following along below.


SETUP DATABASE
------

SEGS requires two databases in order to function: `segs` and `segs_game`. These files come premade with SEGS and are located in SEGS base directory. These instructions assume that you're using these included SQLite databases, as this is the recommended setup for new installations. If these two files are not present in your SEGS directory, or if you need to reset your server, then please run the included program `dbtool`.

_**ADVANCED:** Manual configuration of the SQLite databases is possible with the help of a GUI tool like the popular SQLite Browser (http://sqlitebrowser.org)_

_Optionally, SEGS can utilize a remote database server setup utilizing PostgreSQL. This method is only recommended for advanced users and requires PostgreSQL 8.3 (or greater) (http://www.postgresql.org/)._


CONFIGURATION
------

Configuration of your SEGS server is done using the `settings.cfg` file saved in the SEGS folder, and must be completed before running your server for the first time. See below for an exhaustive breakdown of server configuration.

1. Acquire required DLL file.

   NOTE: This step is only required on Windows.

   Much like the utilities, the authserver requires a `.dll` file to run. Navigate to the folder containing the build of your project. It should be named something like this: `build-Segs-Desktop_Qt_YOUR_VERSION_NUMBER_MinGW_32bit-Default`. From this folder, navigate to: `./3rd_party/built/lib/`. Copy the file `libACE.dll` to the SEGS directory.

2. Setup ./settings.cfg.

   The settings.cfg file contains all of the configuration options for your private SEGS server. Within this file are several sections:

   ##### [AdminServer]
   This section contains information for connecting to the SQL databases required by the server. It's broken down into two sections `AccountDatabase` and `CharacterDatabase` with the same setting variables for both. These are set to default values and likely don't need to be changed unless your configuration is advanced. A brief summary is below:
   - `db_driver` -- The database engine. `QSQLITE` is the default. 
   - `db_host` -- The IP of the database host. Using the default sqlite files, this is `127.0.0.1` (localhost)
   - `db_port` -- The port to access the database. Almost always `5432`
   - `db_name` -- The name of the database. By default this is `segs` for AccountDatabase and `segs_game` for CharacterDatabase.
   - `db_user` -- An administrative username for this database 
   - `db_pass` -- The password for the database user

   ##### [AuthServer]
   This section contains settings specific to the authentication server `authserver`. 
   - `listen_addr` -- This is the IP of the server that clients will connect to, followed by `:2106` which is the default listening port. If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network. The port never changes.

   ##### [GameServer]
   This section contains settings 
   - `server_name` -- The name of your private server. This will show up on the server select screen in the client.
   - `listen_addr` -- This is the IP of the server that clients will connect to, followed by `:7002` which is the default listening port. If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network. The port never changes.
   - `location_addr` -- This is the IP of the server where the clients will receive from the server, followed by `:7002` which is the default listening port. If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network. The port never changes.
   - `max_players` -- The maximum number of concurrent connections allowed by the server.
   - `max_account_slots` -- This is the default number of character slots allowed for each account.

   ##### [MapServer]
   - `listen_addr` -- This is the IP of the server that clients will connect to, followed by `:7003` which is the default listening port. If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network. The port never changes.
   - `location_addr` -- This is the IP of the server where the clients will receive from the server, followed by `:7003` which is the default listening port. If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network. The port never changes.

3. Setup Admin Account
   SEGS comes with a default admin account setup with the login details below:
   ```
   Username: Admin
   Password: Segs123
   ```
   It is advised to change this login information immediately.
   
   __**// FINISH SETTING UP AN ADMIN ACCOUNT GOES HERE //**__


START THE SERVER AND PLAY
------

At this point you're ready to start the server by running the program `authserver`. Once running, start the CoX client we set up earlier and log in using any username and password combination (users are autocreated).

Enjoy!


MORE INFORMATION
------

Additional information will be available here, including optional steps to take after configuration and where to find additional help.


FAQs
------

Frequently asked questions will go here.
