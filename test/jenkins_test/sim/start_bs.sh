#!/bin/bash -l
# start simulated uplink test on base station (receiver end)
# run program in agora directory
# default output file is in test/jenkins_test/sim/bs_out.txt, add argument to specify otherwise

source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null
<<<<<<< HEAD
out_file='test/jenkins_test/sim/bs_out.txt'

for i in 1 2 3; do
{  # try to start radio at most three times
    echo "==========================================="
    echo "starting base station"
    echo "==========================================="
    ./build/agora data/bs-ul-hw.json > $out_file &
=======
if [ -z "$1" ]; then
    out_file='test/jenkins_test/sim/bs_out.txt'
    else
        out_file=$1
fi
>>>>>>> 21399e990e3503fc98ca6c64e4d1fc3331293347

echo "==========================================="
echo "starting receiver..."
echo "==========================================="
./build/agora --conf_file data/tddconfig-sim-ul.json > $out_file
pid=$!

<<<<<<< HEAD

#timeout 30 tail -f -n0 $out_file | grep -qe "Agora: terminating"
#if [ $? != 0 ]; then
#    echo "[Warning] Agora timed out, exiting..."
#    exit 21
#    else
#        exit
#fi
=======
timeout 30 tail -f -n0 $out_file | grep -qe "Agora: terminating"
if [ $? != 0 ]; then
    echo "[Warning] Agora timed out, exiting..."
    exit 20
    else
        kill $pid
        exit
fi
>>>>>>> 21399e990e3503fc98ca6c64e4d1fc3331293347
