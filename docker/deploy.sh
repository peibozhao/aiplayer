#!/usr/bin/env bash

set -e

device='gpu'
if [[ $# -ge 1 ]]; then
    device=$1
fi
if [[ ! $device =~ ^(cpu|gpu)$ ]]; then
    echo "No such device $device"
    exit -1
fi

# Ocr
pushd paddleocr
if [[ -z "$(docker images -q paddleocr)" ]]; then
    echo "--- Docker build"
    if [[ $device = 'gpu' ]]; then
        docker build -t paddleocr .
    else
        docker build -t paddleocr -f Dockerfile.cpu .
    fi
fi
if [[ -n "$(docker ps -q -f name=paddleocr)" ]]; then
    echo "--- Docker running"
elif [[ -n "$(docker ps -q -a -f name=paddleocr)" ]]; then
    echo "--- Docker start"
    docker start paddleocr
else
    echo "--- Docker run"
    if [[ $device = 'gpu' ]]; then
        docker run -d --gpus all -p 8868:8868 --name paddleocr paddleocr
    else
        docker run -d -p 8868:8868 --name paddleocr paddleocr
    fi
fi
popd

# OpenStf
pushd openstf
if [[ -z "$(docker images -q openstf)" ]]; then
    echo "--- Docker build"
    docker build -t openstf .
fi
# Show home
if [[ -x `command -v adb` ]]; then
    echo "--- Kill adb server"
    adb start-server
    adb kill-server
fi
if [[ -n "$(docker ps -q -f name=openstf)" ]]; then
    # minitouch must restart
    echo "--- Docker restart"
    docker restart openstf
elif [[ -n "$(docker ps -a -q -f name=openstf)" ]]; then
    echo "--- Docker start"
    docker start openstf
else
    echo "--- Docker run"
    docker run -d --cap-add=ALL --device /dev/bus/usb:/dev/bus/usb -p 1111:1111 -p 1313:1313 --name openstf openstf
fi
popd

