cd ./platform/bsp/$1
mkdir build
cd ./build
cmake ..
make V=1
