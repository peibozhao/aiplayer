#!/usr/bin/env bash

# adb init
adb -a nodaemon server start &
sleep 1
adb wait-for-device
if [[ -z $? ]]; then
    exit -1
fi

# STFService
adb shell am start-foreground-service --user 0 \
    -a jp.co.cyberagent.stf.ACTION_START \
    -n jp.co.cyberagent.stf/.Service
# adb forward tcp:1100 localabstract:stfservice

APK=$(adb shell pm path jp.co.cyberagent.stf | \
    tr -d '\r' | awk -F: '{print $2}')
adb shell export CLASSPATH="$APK"\; \
    exec app_process /system/bin jp.co.cyberagent.stf.Agent &
sleep 1
# adb forward tcp:1090 localabstract:stfagent

# minicap
pushd /minicap
./run.sh -P 1080x2340@1080x2340/90 &
adb forward tcp:1313 localabstract:minicap &
popd

# minitouch
pushd /minitouch
./run.sh &
adb forward tcp:1111 localabstract:minitouch &
popd

sleep 9999999999
