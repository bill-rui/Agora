#!/bin/bash -el
# exit code 20 if start fails, 21 if timed out after radio start, 22 if can't find bs out file

source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null

sleep 5
echo $UE_STARTED
while [ "$UE_STARTED" == "false" ]; do
{
    if [ i == 10 ]; then
        echo 'UE did not start, exiting...'
        exit 20
    fi
    sleep 1
    ((i++))
} done

for i in 1 2 3; do
{  # try to start radio at most three times
    echo "==========================================="
    echo "starting base station"
    echo "==========================================="
    ./build/agora data/bs-ul-hw.json > $out_file &

    pid=$!
    export PID=$pid
    sleep 1
    timeout 5 tail -f -n0 $out_file | grep -qe "radio start done!"
    if [ $? != 0 ]; then
        if [ $i == 3 ]; then
            echo "==========================================="
            echo "cannot start radio, exiting..."
            echo "==========================================="
            kill $pid >/dev/null
            rm $out_file
            exit 20
        fi
        echo "==========================================="
        echo "radio did not start, trying again..."
        echo "===========================================" 
        kill $pid >/dev/null
        else
            sleep 2
            break
    fi
} done