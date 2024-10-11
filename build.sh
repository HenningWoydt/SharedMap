#!/bin/bash

function get_num_cores {
  grep -c ^processor /proc/cpuinfo;
}

ROOT=${PWD}
if [ -d .git ]; then
  # SharedMap is build from a git repository
  git submodule update --init --recursive
fi


cd extern && ./install_mtkahypar.sh
cd ${ROOT}

cd extern/ProcessMappingVerifier && ./build.sh
cd ${ROOT}

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DMTRECPROMAP_ENABLE_ASSERTIONS=OFF -DMTRECPROMAP_ENABLE_STATCOLLECTOR=OFF && cd ${ROOT}
cmake --build build --parallel "$(get_num_cores)" --target recpromap
cd ${ROOT}
