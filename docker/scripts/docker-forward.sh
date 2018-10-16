#!/bin/bash

docker_id=$(docker ps -aqf "name=$DOCKER_FORWARD_NAME")
docker exec -i $docker_id $@
