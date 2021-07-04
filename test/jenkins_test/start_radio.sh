# exit code 20 if start fails, 21 if timed out after radio start, 22 if can't find bs out file
#!/bin/bash -el
source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null

bs_out_file='test/jenkins_test/bs_out.txt'

if [ $1 == "-BS" ]; then
    out_file=$bs_out_file
    echo "==========================================="
    echo "starting base station"
    echo "==========================================="
    else   
        out_file='test/jenkins_test/ue_out.txt'
        echo "==========================================="
        echo "starting user"
        echo "==========================================="
fi

for i in 1 2 3; do
{  # try to start radio at most three times

    if [ $1 == "-BS" ]; then
        ./build/agora data/bs-ul-hw.json > $out_file &
        else   
            ./build/user data/ue-ul-hw.json > $out_file &
    fi

    pid=$!
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

i=0
while ! [ -f $bs_out_file ]; do
    ((i++))
    if [ $i == 10 ]; then
        echo "[Warning] cannot find base station output file, exiting..."
        kill $pid
        exit 22
    fi
    sleep 1
done

timeout 20 tail -f -n0 $bs_out_file | grep -qe "Agora: terminating"
if [ $? == 0 ]; then
    echo "[Warning] Agora timed out, exiting..."
    exit_code=21
fi

kill $pid
rm $out_file
exit $exit_code