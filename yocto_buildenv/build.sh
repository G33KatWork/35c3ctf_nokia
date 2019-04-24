#!/bin/bash

cd $(dirname $0)

source ./config

# Build the docker container
docker build --no-cache --rm -t $IMAGE_NAME .

exit $?
