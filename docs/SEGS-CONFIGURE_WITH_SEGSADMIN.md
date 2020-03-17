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


    4. **(Optional) Add Community Power Data**
      Copy powers.json from https://github.com/segs-community/cox-scripts/blob/master/powers.json into data folder.

3. **Ensure proper ports are open on any firewalls**  

    See the table below for a list of Network Ports that SEGS needs open to run properly. These are the same ports that the original game client and now, SEGS uses as well.
    
    | Protocol | Ports      | For Use By                         |
    | TCP      | 2106       | AuthServer                         |
    | TCP      | 6001       | AdminRPC                           |
    | UDP      | 7002       | GameServer                         |
    | UDP      | 7003-7050  | MapServer, each MapServer Instance |
    | UDP      | 7003-7050  | MapServer, each MapServer Instance |

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

For Help with installation and configuration of your local SEGS, please see visit us on discord at https://discord.segs.dev/


FAQs
------

Visit our FAQ at https://github.com/Segs/Segs/blob/master/docs/FAQ.md

