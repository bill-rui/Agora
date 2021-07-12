#!/bin/bash -l
# start simulated uplink test on base station (receiver end)
# run program in agora directory
# default output file is in test/jenkins_test/sim/bs_out.txt, add argument to specify otherwise

source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null
if [ -z "$1" ]; then
    out_file='test/jenkins_test/sim/bs_out.txt'
    else
        out_file=$1
fi

echo "==========================================="
echo "starting receiver..."
echo "==========================================="
./build/agora --conf_file data/tddconfig-sim-ul.json &
pid=$!

timeout 50 tail -f -n0 $out_file | grep -qe "Agora: terminating"
if [ $? != 0 ]; then
    echo "[Warning] Agora timed out, exiting..."
    exit 20
    else
        kill $pid
        exit
fi