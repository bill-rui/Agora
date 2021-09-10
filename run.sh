#!/bin/bash -el

./build/data_generator --conf_file data/ue-mac-dl-sim.json
./build/user --conf_file data/ue-mac-dl-sim.json &
sleep 1
./build/chsim --bs_threads 1 --ue_threads 1 --worker_threads 2 --core_offset 28 --bs_conf_file data/bs-mac-dl-sim.json --ue_conf_file data/ue-mac-dl-sim.json &
sleep 1
./build/macuser --enable_slow_start 1 --conf_file data/ue-mac-dl-sim.json &
./build/agora --conf_file  data/bs-mac-dl-sim.json &
./build/macbs --enable_slow_start 1 --conf_file data/bs-mac-dl-sim.json