#!/bin/bash -el
source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null

echo "==========================================="
echo "Run cmake... with -DUSE_ARGOS=on -DUSE_UHD=off (UE)"
echo "==========================================="
cmake -S ./ -B ./build -DUSE_ARGOS=on -DUSE_UHD=off
echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "execute make (UE)"
echo "==========================================="
make -j -C ./build
echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "Generating data for test (UE)"
echo "==========================================="
./build/data_generator --conf_file data/ue-ul-hw.json
echo -e "-------------------------------------------------------\n\n\n"