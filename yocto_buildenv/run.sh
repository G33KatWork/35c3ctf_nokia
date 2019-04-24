#!/bin/bash
#set -x

cd $(dirname $0)

source ./config

if docker inspect $CONTAINER >/dev/null 2>&1; then
    echo -e "\nINFO: Reattaching to running container $CONTAINER\n"
    docker start -i $CONTAINER
else
    echo -e "\nINFO: Creating a new container from image $IMAGE_NAME\n"
    docker run -t -i \
        --volume=$PROJECTDIR:/project \
        --name=$CONTAINER \
        -e USER=$USER -e USERID=$UID -e ROOT=TRUE \
        -e DISPLAY=$DISPLAY \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        --privileged \
        -v /dev/bus/usb:/dev/bus/usb \
        $IMAGE_NAME
fi

exit $?
