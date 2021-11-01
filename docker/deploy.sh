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
    echo "--- Docker stop"
    docker stop paddleocr
fi
echo "--- Docker run"
if [[ $device = 'gpu' ]]; then
    docker run --rm -d --gpus all -p 8868:8868 --name paddleocr paddleocr
else
    docker run --rm -d -p 8868:8868 --name paddleocr paddleocr
fi
popd

# OpenStf
pushd openstf
if [[ -z "$(docker images -q openstf)" ]]; then
    echo "--- Docker build"
    docker build -t openstf .
fi
if [[ -n "$(docker ps -q -f name=openstf)" ]]; then
    echo "--- Docker stop"
    docker stop openstf
fi
# Show home
if [[ -x `command -v adb` ]]; then
    adb wait-for-device
    echo "--- Show home"
    if [[ ! `adb shell dumpsys window | grep mCurrentFocus` =~ home ]]; then
        adb shell input keyevent KEYCODE_HOME
    fi
    echo "--- Kill adb server"
    adb kill-server
fi
echo "--- Docker run"
docker run --rm -d --cap-add=ALL --device /dev/bus/usb:/dev/bus/usb -p 1111:1111 -p 1313:1313 --name openstf openstf
popd


