#!/usr/bin/env bash

function Usage {
    echo "Usage: '$0 MODE'"
    exit 0
}

if [[ $# -lt 1 ]]; then
    Usage
fi

root_path=`dirname $0`
root_path=`dirname $root_path`
root_path=`dirname $root_path`

pushd docker
./deploy.sh
popd

pushd tool/web
./web.py
if [[ $? -ne 0 ]]; then
    echo ERROR: dash web is not setup
fi
popd

pushd build/demo
sleep 10  # Waitting for openstf
docker exec openstf adb shell input keyevent 82
docker exec openstf adb shell input swipe 500 1000 500 500 100
docker exec openstf adb shell am start com.bilibili.azurlane/com.manjuu.azurlane.MainActivity
./aiplayer_demo --config ../../config/config_blhx.yaml --mode $1
popd
