#!/bin/bash

set -e

mkdir build
cd build

cmake ..
make
sudo make install

cd ..

make lint