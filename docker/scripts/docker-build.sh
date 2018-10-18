#!/bin/bash

docker build -t "$DOCKER_FORWARD_NAME" . -f $1
