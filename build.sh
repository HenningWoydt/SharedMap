#!/bin/bash

function get_num_cores {
  grep -c ^processor /proc/cpuinfo;
}

ROOT=${PWD}
if [ -d .git ]; then
#  SharedMap is build from a git repository
  git submodule update --init --recursive
fi

# install mt_kahypar
mkdir extern/mt_kahypar_local
mkdir extern/mt-kahypar/build
cd extern/mt-kahypar/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DKAHYPAR_DOWNLOAD_TBB=ON -DMT_KAHYPAR_DISABLE_BOOST=ON -DKAHYPAR_ENABLE_THREAD_PINNING=OFF -DKAHYPAR_DISABLE_ASSERTIONS=ON -DCMAKE_INSTALL_PREFIX=$(pwd)/../../mt_kahypar_local
make -j16 install.mtkahypar
cd ${ROOT}

# build SharedMap
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && cd ${ROOT}
cmake --build build --parallel "$(get_num_cores)" --target SharedMap
cd ${ROOT}
