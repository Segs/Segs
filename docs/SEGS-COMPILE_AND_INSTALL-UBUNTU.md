INSTALL SEGS ON A HEADLESS UBUNTU SERVER 18.04
------
This has been tested several times and works fine on a fresh Ubuntu 18.04 LXD container.  
Please be aware that segs may pose security risks, and that permissions and firewall may  
need to be adjusted for use on the internet.  

**Requirements**
- Ubuntu 18.04 Server installed with default settings.
- CoX Client Issue 0 for the PIGG files (```magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f```)

**1. Install dependencies**  

```
sudo -i
add-apt-repository multiverse
apt update && apt upgrade -y
apt install build-essential cmake git qt5-default libqt5websockets5-dev qtdeclarative5-dev
```
*NOTE: everything is done as root!*  

**2. Build SEGS-Server binarys**

```
cd /usr/src
git clone https://github.com/Segs/Segs.git
mkdir Segs/build
cd Segs/build
cmake ..
make
```

**3. Move SEGS binarys and edit the settings to fit your needs**

```
mv out /opt/segs
cd /opt/segs/
cp settings_template.cfg settings.cfg
vi settings.cfg
```

**4. Copy COX content to the SEGS-Server data directory**

Upload or copy the *.pigg files from your COH-Clients piggs directory (ex. ```"C:\Program Files (x86)\CoX\piggs"```)  
over to the segs data directory (ex. ```"/opt/segs/data"```) and use the line below to extract them.

```
find /opt/segs/data/ -name '*.pigg' -exec /opt/segs/piggtool -x {} /opt/segs/data/ \;
```

Optionally Copy powers.json from https://github.com/segs-community/cox-scripts/blob/master/powers.json into data folder.

**5. Create gamedb and users**

Inside the SEGS-Server directory ```"/opt/segs"``` use dbtool to create the gamedb and admin user.

```
./dbtool -f create
./dbtool adduser -l USERNAME -p PASSWORD -a 9 adduser
```

**6. Configure SEGS as SystemD service and start**

Switch to root (or sudo user) for the last steps and create system service.

```
vi /etc/systemd/system/segs.service
```
```
[Unit]
Description=SEGS Server
After=network.target

[Service]
Type=simple
User=segs
Group=segs
WorkingDirectory=/opt/segs
ExecStart=/opt/segs/segs_server --config /opt/segs/settings.cfg
KillMode=process
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

**7. Create SEGS user and set permissions**

```
useradd -s /usr/sbin/nologin -r segs
chown segs:segs -R /opt/segs
```
**8. Start SEGS**

```
systemctl start segs
```

Watch the log
```
journalctl -u segs -f
```

