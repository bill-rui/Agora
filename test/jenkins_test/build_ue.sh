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