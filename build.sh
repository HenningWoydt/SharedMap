#!/bin/bash

# ----- pick a reasonable parallelism (leave 2 cores free) -----
calc_jobs() {
  local cores
  cores=$( nproc 2>/dev/null \
        || getconf _NPROCESSORS_ONLN 2>/dev/null \
        || sysctl -n hw.ncpu 2>/dev/null \
        || echo 4 )
  local j=$(( cores - 2 ))
  if [ "$j" -lt 1 ]; then j=1; fi
  echo "$j"
}
JOBS="${MAX_THREADS:-$(calc_jobs)}"
echo "Building with $JOBS parallel jobs (override with MAX_THREADS)."

ROOT=${PWD}
GCC=$(which gcc || true)

echo "Root          : ${ROOT}"
echo "Using C compiler: ${GCC:-<system default>}"

rm -rf extern
mkdir -p extern/local

# --- Download KaHIP 3.19 ---
echo "Downloading KaHIP 3.19..."
if (
  cd extern \
  && rm -f v3.19.tar.gz \
  && rm -rf KaHIP \
  && wget -q https://github.com/KaHIP/KaHIP/archive/refs/tags/v3.19.tar.gz \
  && tar -xzf v3.19.tar.gz \
  && mv KaHIP-3.19 KaHIP \
  && rm -f v3.19.tar.gz
); then
  echo "KaHIP 3.19 downloaded and extracted successfully."
else
  echo "Failed to download KaHIP!" >&2
  exit 1
fi
cd "${ROOT}"

# --- Clone Mt-KaHyPar 1.4 (with submodules) ---
echo "Cloning Mt-KaHyPar 1.4..."
if (
  cd extern \
  && rm -rf MtKaHyPar \
  && git clone --branch v1.4 --depth 1 --recurse-submodules \
       https://github.com/kahypar/mt-kahypar.git MtKaHyPar
); then
  echo "Mt-KaHyPar v1.4 cloned successfully (including submodules)."
else
  echo "Failed to clone Mt-KaHyPar v1.4!" >&2
  exit 1
fi
cd "${ROOT}"

# --- build KaHIP ---
echo "Building KaHIP 3.19..."
if (
  cd "${ROOT}/extern/KaHIP" \
  && mkdir -p build && cd build \
  && cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${ROOT}/extern/local/kahip" \
    -DNOMPI=ON \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  && make install -j "$JOBS" > /dev/null 2>&1
); then
  echo "KaHIP 3.19 build completed successfully."
else
  echo "KaHIP 3.19 build failed!" >&2
  exit 1
fi
cd "${ROOT}"

# --- build Mt-KaHyPar 1.4 ---
echo "Building Mt-KaHyPar 1.4..."
rm -rf extern/MtKaHyPar/build && mkdir -p extern/MtKaHyPar/build
cd extern/MtKaHyPar/build
cmake .. \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DKAHYPAR_DOWNLOAD_TBB=ON \
  -DKAHYPAR_DOWNLOAD_BOOST=ON \
  -DKAHYPAR_ENABLE_THREAD_PINNING=OFF \
  -DKAHYPAR_DISABLE_ASSERTIONS=ON \
  -DCMAKE_INSTALL_PREFIX="${ROOT}/extern/local/mt-kahypar"
make -j"$JOBS" install.mtkahypar
cd "${ROOT}"

# --- build SharedMap ---
echo "Building SharedMap..."
rm -rf build && mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSHAREDMAP_DOWNLOAD_TBB=ON
cmake --build . --parallel "$JOBS" --target SharedMap
cmake --build . --parallel "$JOBS" --target sharedmap
cd "${ROOT}"
