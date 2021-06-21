#run script in Agora's top-level directory
#!/bin/bash

source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64
ue_out_file="./test/test_agora/client.txt"
bs_out_file="./test/test_agora/base_station.txt"

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
{  # try to start radio at msot three times

    ./build/user data/ue-ul-hw.json > $ue_out_file &
    pid=$!
    sleep 1
    timeout 8 tail -f -n0 $ue_out_file | grep -qe "radio start done!"
    if [ $? != 0 ]; then
        if [ $i == 3 ]; then
            echo "==========================================="
            echo "cannot start radio, exiting..."
            echo "==========================================="
            kill $pid >/dev/null
            exit
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

echo ""
echo "==========================================="
echo "Starting base stations"
echo "==========================================="
./build/agora data/bs-ul-hw.json > $bs_out_file &
