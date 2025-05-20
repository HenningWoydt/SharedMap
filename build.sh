#!/bin/bash

MAX_THREADS=8  # Set the desired number of threads here

if [ -d .git ]; then
  # SharedMap is built from a git repository
  git submodule update --init --recursive --remote
fi

# install mt_kahypar
rm -rf extern/mt_kahypar_local && mkdir -p extern/mt_kahypar_local
rm -rf extern/mt-kahypar/build && mkdir -p extern/mt-kahypar/build
cd extern/mt-kahypar/build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
         -DCMAKE_BUILD_TYPE=Release \
         -DKAHYPAR_DOWNLOAD_TBB=ON \
         -DKAHYPAR_DOWNLOAD_BOOST=ON \
         -DKAHYPAR_ENABLE_THREAD_PINNING=OFF \
         -DKAHYPAR_DISABLE_ASSERTIONS=ON \
         -DCMAKE_INSTALL_PREFIX=$(pwd)/../../mt_kahypar_local
make -j"$MAX_THREADS" install.mtkahypar
cd ../../..

# build SharedMap
rm -rf build && mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSHAREDMAP_DOWNLOAD_TBB=ON
cmake --build . --parallel "$MAX_THREADS" --target SharedMap  # the executable
cmake --build . --parallel "$MAX_THREADS" --target sharedmap  # the library