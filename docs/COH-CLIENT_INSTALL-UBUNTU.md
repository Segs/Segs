CoH Client Installation on Ubuntu 18.04 using PlayOnLinux  
------  
  
1. **Install CoH Client version 0.22**  
  
	SEGS will only communicate with the CoH client from Issue 0 (version 0.22).  
	This can be found through various sources online, or via the following Torrent-magnet link provided the City of Heroes community:  
	Use a torrent-client like uTorrent or Transmission to download the client via the magnet link.  
  
	```
	magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f  
	```
  
	An easy way play Windows games on Linux using Wine is PlayOnLinux.  
  
	Install PlayOnLinux  
  
	```
	sudo apt update  
	sudo apt install playonlinux -y  
	```
  
	Run PlayOnLinux and choose `Install a Program` followed by `Install non-listed Program`.  
	Follow the promts to initiate the client installation.  
  
	> Keep the default settings unless you want to do something fancy and know what you are doing.  
  
	When asked, choose 32Bit Windows Installation and browse for your CoX-Setup.exe file downloaded earlier.  
	Follow the prompts to install the Issue 0 CoX client.  
	In the last step of the PlayOnLinux prompt you can create Desktop shortcuts for CoX and/or CoX-Auth.  
	When done select `I don't want to make another shortcut` and hit Next button.  
  
1.1 **Configuring Wine for CoX and CoX-Auth**  
  
	Set the correct Windows Version in the Wine configuration:  
	Run PlayOnLinux select CoX and go to Configure, select the Wine tab and go to Configure Wine.  
	In the Wine configuration window select the Applications tab and set the Windows Version to `Windows XP` and hit the OK button.  
  
	The CoX-client requires the Gecko(32bit) for Wine module.  
	The CoX-Auth, which is used to set the server IP the client connects to, requires the Mono for Wine module.  
  
	Download the latest versions of the modules:  
	
	- Wine-Mono.msi from https://dl.winehq.org/wine/wine-mono/  
	- Gecko(32bit) Win-Gecko-x86.msi from https://wiki.winehq.org/Gecko  
  
	Run PlayOnLinux, select CoX and go to Configure, select the Wine tab, go to Control Panel and open Add/remove Programs.  
	Click the Install button and browse to the earlier downloaded `Wine-Mono.msi` and click OK in the Add/Remove Programs window.  
	Repeat the procedure for the Installation of `Win-Gecko-x86.msi`.  
  
2. **Point the CoX client towards your server IP**  
  
	Run `CoX-Auth` either from previously created shortcut or in the PlayOnLinux-GUI to set the IP address to that of your server.  
	If running your server locally this CANNOT be `127.0.0.1` (localhost) and must point to your actual IP address  
	on the network (`192.168.xxx.xxx` or `10.0.xx.xx`).  
  
	> ADVANCED: Open a Terminal run `ip a`. Your IP will probably start with `192.168.xxx.xxx` or `10.0.xx.xx`  
  
3. **How to correctly run the CoX Client?**  
  
	The CoX Client must be run with a special command line argument telling the client that it's not connecting to the official game servers.  
	Run PlayOnLinux, select CoX go to Configuration.  
	Open the General tab and in the Arguments: field add `-project SEGS`.  
	If you would like to run the client in windowed mode, you may also add `-fullscreen 0 -screen 1024 768` to the arguments.  
  
	Launch Cox either from previously created shortcut or in the PlayOnLinux-GUI.  
