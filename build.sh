LOC_PWD=${PWD}

cd ${LOC_PWD}/apps/ezapp_template/port/$1
mkdir build
cd ./build
cmake ..
make -j8

cd ${LOC_PWD}/examples
mkdir build
cd ./build
cmake ..
make -j8