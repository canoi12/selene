#!/bin/bash

PLATFORM=$1

IMAGE_NAME="selene-$PLATFORM"

linux() {
    docker build -t selene-$PLATFORM -f cross/Dockerfiles/Dockerfile.linux .
    docker run --rm -v ./dist:/out selene-$PLATFORM cp -r /dist /out/$PLATFORM
}

case $1 in
    "linux" )
        linux ;;
esac