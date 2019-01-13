CoH Client Installation
------

1. **Install CoH Client version 0.22**  

   SEGS will only communicate with the CoH client from Issue 0 (version 0.22).  
   This can be found through various sources online, or via the following magnet link provided the City of Heroes community:  

   ```
   magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f
   ```

   After the download completes, navigate to the download folder and run CoX-Setup.exe.  
   Follow the prompts to install the Issue 0 CoX client.  


2. **Point the CoX client towards your server IP**  

   In your CoX Client folder, locate and run `CoX-Auth.exe` to set the IP address to that of your server.  
   If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address  
   on the network (`192.168.xxx.xxx` or `10.0.xx.xx`).  
   
   > For Windows users, a guide to determining your IP can be found here: https://support.microsoft.com/en-us/help/15291/windows-find-pc-ip-address
   
   > ADVANCED: run `cmd` and type `ipconfig`. Your IP will probably start with `192.168.xxx.xxx` or `10.0.xx.xx`


3. **How to correctly run the CoX Client?**  

   The CoX Client must be run with a special command line argument telling the client that it's not connecting
   to the official game servers.  
   Create a shortcut to CoX.exe and place it on your desktop. Right click the link to access its properties,  
   and in the `Target:` field, add the following to the end of the line: ` -project SEGS`.  

   > Your line should end something like this: `.../CoX.exe" -project SEGS`.  
   Please note the space between the quotation mark and the dash.  
   
   If you would like to run the client in windowed mode, you may also add these optional command line arguments:  

   `-fullscreen 0 -screen 1024 768`.  
   
   When you want to launch the client, you will need to use this shortcut.

   NOTE: This is a known issue with Intel or AMD/Radeon graphics cards.  
         https://github.com/Segs/Segs/issues/736

