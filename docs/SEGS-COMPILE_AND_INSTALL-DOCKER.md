USE SEGS WITH DOCKER
-------
Note: if you choose not to `source activate.env`, you can run the commands via `scripts/segs` instead

Clone the GitHub Repository and move into the directory to do:
```
cp activate.env.template activate.env
source activate.env # set ENV variables and add ./scripts to PATH

docker-build Dockerfile # build the docker image (only needed once)
docker-up # Ctrl-D to run in background

docker-forward segs bootstrap # set up cmake / clean build
docker-forward segs build # build all components
docker-forward segs build gameserver_lib # build a single component
```


**IMPORTANT:** You can find information on setting up and running your SEGS server by reading the README.md located in your output directory (typically `out`). You can also read a copy of that [README.md here](./Projects/CoX/docs/README.md)

- Don't forget! You'll need a copy of the required CoH client from Issue 0 (release) version 0.22, which can be found through various sources online, or via magnet link:
  `magnet:?xt=urn:btih:c2e7e0aa72004594337d73ae6a305242e23d815f`

