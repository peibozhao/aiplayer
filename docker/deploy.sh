#!/usr/bin/env bash

set -e

ARGS=`getopt -o 'd:m:' -l 'device:,miao:' -- $@`
if [[ $? != 0 ]]; then
    echo 'Param error'
    exit -1
fi

eval set -- ${ARGS}

device='gpu'
miao_key=''

while true; do
    case $1 in
        -d|--device)
            device=$2
            shift 2
            ;;
        -m|--miao)
            miao_key=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "No such param " $1
            exit -1
            ;;
    esac
done

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

# Aiplayer
pushd aiplayer
if [[ -z "$(docker images -q aiplayer)" ]]; then
    echo "--- Docker build"
    docker build -t aiplayer -f .
fi
if [[ -n "$(docker ps -q -f name=aiplayer)" ]]; then
    echo "--- Docker restart"
    docker restart aiplayer
elif [[ -n "$(docker ps -q -a -f name=aiplayer)" ]]; then
    echo "--- Docker start"
    docker start aiplayer
else
    echo "--- Docker run"
    docker run -d -p 8050:8050 --name aiplayer aiplayer
fi
popd
