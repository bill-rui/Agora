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
            exit 20
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