#!/usr/bin/env bash


while true; do
  adb wait-for-device
  scrcpy -m 1000 --forward-port 22331 --remote-control-port 22332 --no-display -V verbose
  sleep 5
done

