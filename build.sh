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

echo "Root            : ${ROOT}"
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

# --- Download Mt-KaHyPar 1.5.3 ---
echo "Downloading Mt-KaHyPar 1.5.3..."
if (
  cd extern \
  && rm -f v1.5.3.tar.gz \
  && rm -rf MtKaHyPar \
  && wget -q https://github.com/kahypar/mt-kahypar/archive/refs/tags/v1.5.3.tar.gz \
  && tar -xzf v1.5.3.tar.gz \
  && mv mt-kahypar-1.5.3 MtKaHyPar \
  && rm -f v1.5.3.tar.gz
); then
  echo "Mt-KaHyPar 1.5.3 downloaded and extracted successfully."
else
  echo "Failed to download Mt-KaHyPar 1.5.3!" >&2
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
  && make install -j "$JOBS"
); then
  echo "KaHIP 3.19 build completed successfully."
else
  echo "KaHIP 3.19 build failed!" >&2
  exit 1
fi
cd "${ROOT}"

# --- build Mt-KaHyPar 1.5.3 (C library install) ---
echo "Building Mt-KaHyPar 1.5.3..."
rm -rf extern/MtKaHyPar/build && mkdir -p extern/MtKaHyPar/build
cd extern/MtKaHyPar/build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DKAHYPAR_DOWNLOAD_TBB=ON \
  -DKAHYPAR_DOWNLOAD_BOOST=ON \
  -DKAHYPAR_ENABLE_THREAD_PINNING=OFF \
  -DKAHYPAR_DISABLE_ASSERTIONS=ON \
  -DCMAKE_INSTALL_PREFIX="${ROOT}/extern/local/mt-kahypar"

# This is the important part for the *library*
cmake --build . --parallel "$JOBS" --target install-mtkahypar
cd "${ROOT}"

# --- bundle Mt-KaHyPar downloaded runtime deps into the install prefix ---
echo "Bundling Mt-KaHyPar downloaded Boost/TBB into prefix..."

MTK_BUILD="${ROOT}/extern/MtKaHyPar/build"
MTK_PREFIX="${ROOT}/extern/local/mt-kahypar"

# Mt-KaHyPar may install into lib64 on some systems (common on HPC)
if [ -d "${MTK_PREFIX}/lib64" ]; then
  MTK_LIBDIR="${MTK_PREFIX}/lib64"
else
  MTK_LIBDIR="${MTK_PREFIX}/lib"
fi
mkdir -p "${MTK_LIBDIR}"

# Locate downloaded shared libs in the build tree
BOOST_PO=$(find "${MTK_BUILD}" -type f -name "libboost_program_options.so*" | head -n 1)
TBB_SO=$(find "${MTK_BUILD}" -type f -name "libtbb.so*" | head -n 1)
TBBMALLOC_SO=$(find "${MTK_BUILD}" -type f -name "libtbbmalloc.so*" | head -n 1)

if [ -z "${BOOST_PO}" ] || [ -z "${TBB_SO}" ] || [ -z "${TBBMALLOC_SO}" ]; then
  echo "ERROR: Could not locate required downloaded libs in ${MTK_BUILD}" >&2
  echo "  BOOST_PO=${BOOST_PO}" >&2
  echo "  TBB_SO=${TBB_SO}" >&2
  echo "  TBBMALLOC_SO=${TBBMALLOC_SO}" >&2
  echo "Hint: inspect what was built with: find ${MTK_BUILD} -name 'libboost_*.so*' -o -name 'libtbb*.so*'" >&2
  exit 1
fi

# Copy ALL boost shared libs from the directory that contains program_options
BOOST_DIR="$(dirname "${BOOST_PO}")"
cp -a "${BOOST_DIR}"/libboost_*.so* "${MTK_LIBDIR}/"

# Copy TBB + tbbmalloc (with symlinks)
cp -a "$(dirname "${TBB_SO}")"/libtbb.so* "${MTK_LIBDIR}/"
cp -a "$(dirname "${TBBMALLOC_SO}")"/libtbbmalloc.so* "${MTK_LIBDIR}/"

# Optional: ensure mt-kahypar itself searches next to itself at runtime
if command -v patchelf >/dev/null 2>&1; then
  if [ -f "${MTK_LIBDIR}/libmtkahypar.so.1.5.3" ]; then
    echo "Setting RPATH on libmtkahypar to \$ORIGIN"
    patchelf --set-rpath '$ORIGIN' "${MTK_LIBDIR}/libmtkahypar.so.1.5.3" || true
  fi
fi

echo "Bundled libs in ${MTK_LIBDIR}:"
ls -1 "${MTK_LIBDIR}" | egrep 'mtkahypar|libboost_|libtbb'


# --- build SharedMap ---
echo "Building SharedMap..."
rm -rf build && mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel "$JOBS" --target SharedMap
cmake --build . --parallel "$JOBS" --target sharedmap
cd "${ROOT}"
