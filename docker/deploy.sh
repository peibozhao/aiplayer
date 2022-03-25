#!/usr/bin/env bash

set -e

ARGS=`getopt -o 'd:' -l 'device:' -- $@`
if [[ $? != 0 ]]; then
    echo 'Param error'
    exit -1
fi

eval set -- ${ARGS}

device='gpu'

while true; do
    case $1 in
        -d|--device)
            device=$2
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

# paddle ocr
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

# scrcpy
pushd scrcpy
if [[ -z "$(docker images -q scrcpy)" ]]; then
    echo "--- Docker build"
    docker build -t scrcpy .
fi
if [[ -n "$(docker ps -q -f name=scrcpy)" ]]; then
    echo "--- Docker running"
elif [[ -n "$(docker ps -a -q -f name=scrcpy)" ]]; then
    echo "--- Docker start"
    docker start scrcpy
else
    echo "--- Docker run"
    docker run -d --cap-add=ALL --device /dev/bus/usb:/dev/bus/usb -p 22331:22331 -p 22332:22332 --name scrcpy scrcpy
fi
popd

