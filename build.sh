#!/bin/bash -el

cmake -DENALBE_MAC=true -S ./ -B ./build

echo -e "-------------------------------------------------------\n\n\n"

echo "==========================================="
echo "execute make... ($1)"
echo "==========================================="
make -j -C ./build