#!/bin/bash

docker kill $DOCKER_FORWARD_NAME
docker rm $DOCKER_FORWARD_NAME
docker run -v ${PWD}/:/segs -i --restart unless-stopped --name "$DOCKER_FORWARD_NAME" -t $DOCKER_FORWARD_NAME /bin/bash
