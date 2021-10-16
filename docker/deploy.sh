#!/usr/bin/env bash

pushd paddleocr
if [[ -z "$(docker images -q paddleocr)" ]]; then
    docker build -t paddleocr .
fi
if [[ -n "$(docker ps -q -f name=paddleocr)" ]]; then
    docker stop paddleocr
fi
docker run --rm -d --gpus all -p 8868:8868 --name paddleocr paddleocr
popd

pushd openstf
if [[ -z "$(docker images -q openstf)" ]]; then
    docker build -t openstf .
fi
if [[ -n "$(docker ps -q -f name=openstf)" ]]; then
    docker stop openstf
fi
docker run --rm -d --cap-add=ALL --device /dev/bus/usb:/dev/bus/usb -p 1111:1111 -p 1313:1313 --name openstf openstf
popd

