#!/bin/bash

mkdir mt-kahypar/build
cd mt-kahypar/build

cmake .. -DCMAKE_BUILD_TYPE=Release -DKAHYPAR_DOWNLOAD_TBB=ON -DKAHYPAR_DOWNLOAD_BOOST=ON -DKAHYPAR_ENABLE_THREAD_PINNING=OFF -DCMAKE_INSTALL_PREFIX=$(pwd)/../../mt_kahypar_local
make -j16 MtKaHyPar

cmake .. -DCMAKE_BUILD_TYPE=Release -DKAHYPAR_DOWNLOAD_TBB=ON -DMT_KAHYPAR_DISABLE_BOOST=ON -DKAHYPAR_ENABLE_THREAD_PINNING=OFF -DCMAKE_INSTALL_PREFIX=$(pwd)/../../mt_kahypar_local
make -j16 install.mtkahypar
