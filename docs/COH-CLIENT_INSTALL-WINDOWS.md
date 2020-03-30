CoH Client Installation
------

**1. Install CoH Client version 0.22**

SEGS will only communicate with the CoH client from Issue 0 (version 0.22).  
This can be found through various sources online, or via the following magnet link provided by the City of Heroes community:  

```
magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f
```

After the download completes, navigate to the download folder and run CoX-Setup.exe.  
Follow the prompts to install the Issue 0 CoX client.  


**2. Point the CoX client towards your server IP**

In your CoX Client folder, locate and run `CoX-Auth.exe` to set the IP address to that of your server.  
If running your server locally this CANNOT be 127.0.0.1 (localhost) and must point to your actual IP address on the network.  
   
> For Windows users, a guide to determining your IP can be found here: https://support.microsoft.com/en-us/help/15291/windows-find-pc-ip-address
   
> ADVANCED: run `cmd` and type `ipconfig`. Your IP will probably start with `192.168.xxx.xxx` or `10.0.xx.xx`

If you have trouble running Cox-Auth.exe then you can do this step manually by opening regedit, and creating the registry key "Computer\HKEY_CURRENT_USER\Software\Cryptic\Segs" then create a string called "Auth" with the value as your server IP.


**3. How to correctly run the CoX Client?**

The CoX Client must be run with a special command line argument telling the client that it's not connecting  
to the official game servers. Create a shortcut to CoX.exe and place it on your desktop. Right click the link  
to access its properties, and in the `Target:` field, add the following to the end of the line:` -project SEGS`  

> Your line should end something like this: `.../CoX.exe" -project SEGS`.  
> Please note the space between the quotation mark and the dash.

If you would like to run the client in windowed mode, you may also add these optional command line arguments:  

> `-fullscreen 0 -screen 1024 768`

When you want to launch the client, you will need to use this shortcut. After you have entered the game,   
you can get a list of supported server commands by typing `/cmdlist` into the chat window.  


**4. Common Issues and Troubleshooting**

There is a known issue with Intel/AMD/Radeon graphics cards.
The client version we use is not capable of handling those graphics cards.

https://github.com/Segs/Segs/issues/736

For Help with installation and configuration of your local SEGS, please see visit us on discord.
A patch is in the works but we need a skilled debugger that's capable of testing/debugging on an
Intel or Radeon card. If you're experienced in debugging on Windows we would be pleased if you contact us.

**Chat with us on Discord!**

Come and join us in `#pocket-d` channel for general chat or `#troubleshooting` for support.  

 [**Click here to join SEGS Discord**](https://discord.segs.dev/)  

