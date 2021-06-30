# run script in Agora's top-level directory
# test_hw.sh [output file] [threshold BER value (0.005 by default)]
#!/bin/bash

ue_out_file="./ue_out.txt"
THRESHOLD=$1

if [ -z "$1" ]; then
{
    THRESHOLD="0.005"
}
fi

echo "==========================================="
echo "Build cmake... with -DUSE_ARGOS=on -DUSE_UHD=off"
echo "==========================================="
cmake -S ./ -B ./build -DUSE_ARGOS=on -DUSE_UHD=off
echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "execute make"
echo "==========================================="
make -j -C ./build
echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "Generating data for test"
echo "==========================================="
./build/data_generator --conf_file data/ue-ul-hw.json
echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "Starting clients"
echo "==========================================="
echo -e "-------------------------------------------------------\n\n\n"

for i in 1 2 3; do
{  # try to start radio at most three times

    ./build/user data/ue-ul-hw.json > $ue_out_file &
    pid_ue=$!
    sleep 1
    timeout 8 tail -f -n0 $ue_out_file | grep -qe "radio start done!"
    if [ $? != 0 ]; then
        if [ $i == 3 ]; then
            echo "==========================================="
            echo "cannot start radio, exiting..."
            echo "==========================================="
            kill $pid_ue >/dev/null
            rm $ue_out_file
            exit
        fi
        echo "==========================================="
        echo "radio did not start, trying again..."
        echo "===========================================" 
        kill $pid_ue >/dev/null
        else
            sleep 2
            break
    fi
} done