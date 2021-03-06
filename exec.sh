#! /usr/bin/env bash

if [[ $1 == "sender" ]]; then
    echo "./sender 7001 local 7002"
    ./sender 7001 local 7002
elif [[ $1 == "receiver" ]]; then
    echo "./receiver 7003 local 7002"
    ./receiver 7003 local 7002
elif [[ $1 == "agent" ]]; then 
    if [[ -z $2 ]]; then
        echo "Usage: ./exec.sh agent <loss_rate>"
    else
        echo "./agent local local 7001 7002 7003 ${2}"
        ./agent local local 7001 7002 7003 ${2}
    fi
else
    echo "No target!"
fi
