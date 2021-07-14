#!/bin/bash -el
# build agora, run script from agora base directory
# add flag -UE to generate data file
source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null

echo "==========================================="
echo "Run cmake... ($1)"
echo "==========================================="
cmake -S ./ -B ./build
echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "execute make... ($1)"
echo "==========================================="
make -j -C ./build
echo -e "-------------------------------------------------------\n\n\n"

if [ "$1" == "-UE" ]; then
    echo "==========================================="
    echo "Generating data for test... ($1)"
    echo "==========================================="
    ./build/data_generator --conf_file data/tddconfig-sim-ul.json
    echo -e "-------------------------------------------------------\n\n\n"
fi
