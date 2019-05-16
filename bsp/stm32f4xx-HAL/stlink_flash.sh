#!/bin/bash

file=$1

~/work/github/stlink/build/Release/st-flash --debug --reset --format binary --flash=1m write $file 0x08000000
