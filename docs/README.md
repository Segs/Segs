SEGS - INFO & INSTALLATION GUIDE
======

CONTENTS OF THIS FILE
------
- About SEGS
- Requirements and notes
- Installation
- Setup Database
- Configuration
- More information
- FAQs


ABOUT SEGS
------
SEGS is the Super Entity Game Server. It is a free, open source (under the terms of the BSD License),
cross-platform, and object-oriented suite of protocols and services. SEGS is written in C++ and allows
developers to write MMORPG clients with a focus on the superhero-theme. These clients can easily interface
with SEGS' provided services. SEGS also facilitates common MMORPG functionality such as account and
character storage to a database, account and character retrieval from a database, secure client 
authentication, client movement in a virtual world, and client chat.

SEGS currently runs under a variety of platforms including Windows, Linux, *BSD, and probably other UNIX
derivatives.

For legal information, please see License.txt.

For a list of SEGS authors and contributors, please see Authors.txt.


REQUIREMENTS AND NOTES
------

SEGS requires:

- A copy of the CoH client from Issue 0 (release) version 0.22. This can be found through 
  various sources online, or via magnet link: 
  - magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f

For more detailed version of this guide or forums for help see the links below:
- https://segs.nemerle.eu/
- https://segs.nemerle.eu/developers
- https://segs.nemerle.eu/forumz/index.php

To report any bugs, please visit our github:
- https://github.com/Segs/Segs

Note that all directories mentioned in this document are always relative to the
directory of your SEGS installation, and commands are meant to be run from
this directory (except for the initial commands that create that directory).


INSTALLATION
------

1. Download and extract SEGS

   You can obtain the latest stable SEGS release from https://segs.nemerle.eu/download 
   -- the files are available in .tar.gz and .zip formats and can be extracted using most
   compression tools.

   To download and extract the files on a typical Unix/Linux command line, use
   the following commands:

   ```
   wget https://segs.nemerle.eu/download/segs-latest.tar.gz
   tar -zxvf segs-latest.tar.gz
   ```

   This will create a new directory segs/ containing all SEGS files and
   directories. Then, proceed to the section of this README titled **Configuration**

2. Optionally, download and compile from source.

   SEGS will compile in either Linux or Windows. The libraries you'll need to install are 
   listed below and are availble from your distro's package manager, or via the links below:

   - QT >5.8		# https://www.qt.io/download-qt-for-application-development
   - CMake >3.6		# https://cmake.org/download/
   - Git		# https://git-scm.com/download

   For detailed instructions, visit https://segs.nemerle.eu/developers

3. Install CoX Client version 0.22

   SEGS will only communicate with the CoH client from Issue 0 (release) version 0.22. 
   This can be found through various sources online, or via magnet link:

   - magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f

   After the download completes, navigate to the download folder and run CoX-Setup.exe
   Follow the prompts to install the Issue 0 CoX client.

4. Copy the PIGG files to SEGS

   The CoX client uses files ending `.pigg` to store important game information. 
   SEGS uses these same files to run the game world. Once installed, navigate to the CoX 
   folder and copy the contents the `/piggs/` subfolder to the `/data/` directory of SEGS.

5. Using the piggtool located in the main SEGS folder, extract BIN files from bin.pigg

   SEGS requires that we extract bin.pigg to the `/data/bin/` folder. From the console:

   ```
   ./piggtool -x PATH_TO_COH_INSTALL/piggs/bin.pigg
   ```


SETUP DATABASE
------

SEGS requires two databases in order to function: `segs` and `segs_game`. These instructions 
assume that you're using the included SQLite databases, as this is the recommended setup for 
new installations. Configuration of the SQLite database may be easier with the help of a GUI 
tool like the popular SQLite Browser:

- SQLite Browser  # http://sqlitebrowser.org/

*Optionally, SEGS can be utilize a remote database server setup utilizing PostgreSQL.*

- PostgreSQL 8.3 (or greater) (http://www.postgresql.org/).


CONFIGURATION
------

1. Setup ./settings.cfg

   The settings.cfg file contains all of the configuration options for your private SEGS 
   server. This file contains several sections which are broken down below:

   #### [AdminServer]

   #### [AuthServer]

   #### [GameServer]

   #### [MapServer]

2. Create Admin Account



MORE INFORMATION
------

Additional information available here.


FAQs
------

FAQs here.
