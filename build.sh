cd ./apps/ezapp_template/port/$1
mkdir build
cd ./build
cmake ..
make -j8