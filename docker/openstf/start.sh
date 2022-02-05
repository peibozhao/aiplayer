#!/usr/bin/env bash

# adb init
adb -a nodaemon server start &
sleep 1
adb wait-for-device
if [[ -z $? ]]; then
    exit -1
fi

# Show home
echo "--- Show home"
if [[ ! `adb shell dumpsys window | grep mCurrentFocus` =~ home ]]; then
    adb shell input keyevent KEYCODE_HOME
fi

# STFService
# TODO start-foreground-service or startservice
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
screen_size=`adb shell wm size | awk '{print $3}'`
pushd /minicap
# ATTENTION: 0 horizon, 90 vertical
./run.sh -P ${screen_size}@${screen_size}/90 &
adb forward tcp:1313 localabstract:minicap &
popd

# minitouch
pushd /minitouch
./run.sh &
adb forward tcp:1111 localabstract:minitouch &
popd

sleep 9999999999
