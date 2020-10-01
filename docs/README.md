SEGS - INFO & INSTALLATION GUIDE
======
> :exclamation: NOTE: this guide is for running your own server, if you're only looking to login and play on a public server, then please read the play-guide located here: https://github.com/Segs/Segs/blob/develop/README.md 

CONTENTS OF THIS FILE
------
- About SEGS
- Requirements and Notes
- Install & Configure the CoX Client
- Install & Configure SEGS using SEGSAdmin tool
- Start the Server and Play
- Help and More information
- FAQs


ABOUT SEGS
------
SEGS is the Super Entity Game Server. It is a free, open source (under the terms of the BSD License), cross-platform, and object-oriented suite of protocols and services designed to interface with popular super hero themed MMORPG clients. SEGS is written in C++ and facilitates common MMORPG functionality such as account and character storage to a database, account and character retrieval from a database, secure client authentication, client movement in a virtual world, and client chat.

For legal information, please see LICENSE.md

For a list of SEGS authors and contributors, please see AUTHORS.md.


REQUIREMENTS AND NOTES
------

SEGS requires:

- A copy of the CoH client from Issue 1 (beta) version 0.22. This can be found through various sources online, or via magnet link:
  `magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f`

For help, visit the SEGS Community Discord by using the links below:
- https://segs.dev/
- https://discord.segs.dev/

To report any bugs, please visit our GitHub:
- https://github.com/Segs/Segs

Note that all directories mentioned in this document are always relative to the directory of your SEGS installation, and commands are meant to be run from this directory.


INSTALL & CONFIGURE THE COX CLIENT
------

1. **Install CoX Client version 0.22**

   SEGS will only communicate with the CoH client from Issue 1 (beta) version 0.22. This can be found through various sources online, or via the following magnet link provided the City of Heroes community:

   ```
   magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f
   ```

   After the download completes, navigate to the download folder and run CoX-Setup.exe. Follow the prompts to install the Issue 0 CoX client.


2. **Point the CoX client towards your server IP**

   In your CoX Client folder, locate and run `CoX-Auth.exe` to set the IP address to that of the server you'd like to play on. For a list of public servers available to test on, please visit the `#welcome` channel on our https://discord.segs.dev
   
   If running your own server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network.
   
   > For Windows users, a guide to determining your IP can be found here: https://support.microsoft.com/en-us/help/15291/windows-find-pc-ip-address
   
   > ADVANCED: run `cmd` and type `ipconfig`. Your IP will probably start with `192.168.xxx.xxx` or `10.0.xx.xx`
   
   If you have trouble running Cox-Auth.exe then you can do this step manually by opening regedit, and creating the registry key `Computer\HKEY_CURRENT_USER\Software\Cryptic\Segs` then create a string called "Auth" with the value as your server IP.


3. **How to correctly run the CoX Client?**

   The CoX Client must be run with a special command line argument telling the client that it's not connecting to the official game servers. Create a shortcut to CoX.exe and place it on your desktop. Right click the link to access its properties, and in the `Target:` field, add the following to the end of the line: ` -project SEGS`.

   > Your line should end something like this: `.../CoX.exe" -project SEGS`. Please note the space between the quotation mark and the dash.
      
   If you would like to run the client in windowed mode, you may also add these optional command line arguments: `-fullscreen 0 -screen 1024 768`.
   
   When you want to launch the client, you will need to use this shortcut.


INSTALL & CONFIGURE SEGS USING SEGSADMIN TOOL
------

This is a simple installation guide for those looking to get up and running with their own SEGS Server as quickly as possible. It assumes that you will download the latest release package from https://github.com/Segs/Segs/releases and assumes a basic knowledge of computers and computer idioms.

If you're looking to compile SEGS from source, or need to do an advanced manual setup (NOT RECOMMENDED) then please visit our GitHub https://github.com/Segs/Segs


1. **Download and extract SEGS from a stable release.**

   You can obtain the latest stable SEGS release from https://github.com/Segs/Segs/releases
   
   _Files are available in .tar.gz and .zip formats and can be extracted using most common compression tools._

   Extract the release files to a local directory on your machine. For Windows users it is recommended to extract SEGS to your `C:\` to ensure that SEGS has all the permissions it needs to properly run.

   Once extracted, you will have a new directory `SEGS` containing all SEGS files and subdirectories.
   
   
2. **Run SEGSAdmin**

   In your SEGS folder locate and run `SEGSAdmin`. This is the central setup and administration tool for SEGS. From this tool, you can install and configure SEGS, and start or stop your server.
   
   
3. **Server Setup**

   In the SEGSAdmin window, there is a section titled `Server Setup`.
   
   To setup your copy of SEGS click each of the following buttons and follow the on screen instructions. If setup is successful, a green checkmark will appear next to the button.
   
   
   1. **Generate Config File**

      Clicking this button will open a dialog box to configure the settings.cfg file SEGS needs to run. It will ask you to enter a name for your server and the IP address of your computer.
      
      > This will be the same IP you used during Section 2 of 'Configure The COX Client'
      
      Next to the IP address field, clicking a small button with a magnifying glass will look up your computer's IP address.
   
      
   2. **Create New Databases**

      Clicking this button will create the `segs` and `segs_game` SQL database files that SEGS needs to run. It will also ask you to enter a username and password for your Admin account.


   3. **Setup CoH Data Files**

      SEGS requires several CoH files to run. Clicking this button will prompt you to enter the path to your CoX directory. Once selected, it will copy several files from the CoX directory, and extract a portion of those files to your `SEGS/data/` subdirectory. These `.bin` files are required by SEGS to load CoX maps, and spawn NPCs. Once completed, SEGSAdmin will clean up any unnecessary files.

  4. **Ensure proper ports are open on any firewalls**
      Make sure the following Network Ports are open: TCP 443, TCP 2104, TCP 2016, and UDP 7000-7200
     These are the same ports that the original game client and now, SEGS uses as well.
      
5. **(Optional) Add User Account**

   Optionally, you may elect to add additional user accounts by clicking this button on the left side of the SEGSAdmin window. SEGSAdmin will prompt you to enter a username, password, and access level for each account. A user's access level is used to determine their permissions within your server. Please reference this chart:
   
   | Access Level    | Permissions                          |
   | -------------   | ------------------------------------ |
   | 0               | basic user with no special authority |
   | 1               | game master, has limited authority   |
   | 2-8             | reserved for future use              |
   | 9               | administrator with full access       |



START THE SERVER AND PLAY
------

At this point you're ready to start the server by clicking the `Start Server` button in SEGSAdmin. Once running, start the CoX client using the shortcut we set up earlier and log in using your username and password.

Enjoy!


HELP AND MORE INFORMATION
------

Make sure the following Network Ports are open: TCP 443, TCP 2104, TCP 2016, and UDP 7000-7200

For Help with installation and configuration of your local SEGS, please see visit us on discord at https://discord.segs.dev/


FAQs
------

Visit our FAQ at https://github.com/Segs/Segs/blob/master/docs/FAQ.md
