#!/bin/bash

ROOT=${PWD}
if [ -d .git ]; then
#  SharedMap is build from a git repository
  git submodule update --init --recursive
fi

# install mt_kahypar
mkdir -p extern/mt_kahypar_local
mkdir -p extern/mt-kahypar/build
cd extern/mt-kahypar/build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release -DKAHYPAR_DOWNLOAD_TBB=ON -DKAHYPAR_DOWNLOAD_BOOST=ON -DKAHYPAR_ENABLE_THREAD_PINNING=OFF -DKAHYPAR_DISABLE_ASSERTIONS=ON -DCMAKE_INSTALL_PREFIX=$(pwd)/../../mt_kahypar_local
make -j$(nproc) install.mtkahypar
cd ${ROOT}

# build SharedMap
rm -rf build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel $(nproc) --target SharedMap # the executable
cmake --build . --parallel $(nproc) --target sharedmap # the library