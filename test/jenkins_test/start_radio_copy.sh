# exit code 20 if start fails, 21 if timed out after radio start, 22 if can't find bs out file
#!/bin/bash -el
source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64 > /dev/null
exit_code=0
bs_out_file='test/jenkins_test/bs_out.txt'
if [ "$1" == "-BS" ]; then
    out_file=$bs_out_file
    else   
        out_file='test/jenkins_test/ue_out.txt'
fi

for i in 1 2 3; do
{  # try to start radio at most three times

    if [ "$1" == "-BS" ]; then
        echo "==========================================="
        echo "starting base station"
        echo "==========================================="
        ./build/agora data/bs-ul-hw.json > $out_file &
        else
            echo "==========================================="
            echo "starting client"
            echo "==========================================="
            ./build/user data/ue-ul-hw.json > $out_file &
    fi
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
            export UE='false'
        fi
        echo "==========================================="
        echo "radio did not start, trying again..."
        echo "===========================================" 
        kill $pid >/dev/null
        else
            export UE='true'
            sleep 2
            break
    fi
} done