#!/bin/bash -l
# start simulated uplink test on client (sender end)
# run program in agora directory
# default output file is in test/jenkins_test/sim/ue_out.txt, add argument to specify otherwise

source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null
if [ -z "$1" ]; then
    out_file='test/jenkins_test/sim/ue_out.txt'
    else
        out_file=$1
fi

echo "==========================================="
echo "starting sender..."
echo "==========================================="
./build/sender --num_threads=2 --core_offset=1 --frame_duration=5000 --enable_slow_start=1 --conf_file=data/tddconfig-sim-ul.json &
pid=$!