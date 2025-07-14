FROM debian:bookworm-slim

RUN apt update && apt install -y \
    build-essential curl wget git xz-utils bison flex \
    texinfo libgmp-dev libmpfr-dev libmpc-dev \
    cmake ninja-build unzip gperf file python3.11 sshfs openssh-client \
    pkg-config

WORKDIR /build

RUN mkdir /opt/cross
RUN mkdir /opt/cross/solaris9

ADD solaris9_sysroot.tar /opt/cross/solaris9
RUN cd /opt/cross

ENV SYSROOT=/opt/cross/solaris9
ENV PATH="/opt/cross/bin:$PATH"
ENV PKG_CONFIG_PATH="${SYSROOT}/opt/csw/lib/pkgconfig"

RUN wget https://ftp.man.poznan.pl/gnu//binutils/binutils-2.22.tar.gz && \
    tar xf binutils-2.22.tar.gz && \
    mkdir binutils-build && cd binutils-build && \
    CFLAGS="-Wno-error -O2" \
    CXXFLAGS="-Wno-error -O2" \
    ../binutils-2.22/configure \
        --target=sparc-sun-solaris2.9 \
        --prefix=/opt/cross \
        --with-sysroot=${SYSROOT} \
        --disable-nls \
        --with-cpu=v8 \
        --disable-v8plus && \
    make -j$(nproc) && make install

RUN wget https://ftp.man.poznan.pl/gnu/gcc/gcc-4.8.1/gcc-4.8.1.tar.gz && \
    tar xf gcc-4.8.1.tar.gz && \
    cd gcc-4.8.1 && \
    ./contrib/download_prerequisites

# Fix GCC build issues
RUN cd gcc-4.8.1/gcc/cp && \
    echo "/* Simplified cfns.h */" > cfns.h && \
    echo "const char * libc_name_p(const char *str, unsigned int len) { return 0; }" >> cfns.h && \
    sed -i 's/#include "cfns.gperf"/#include "cfns.h"/' ../except.c

# Clean conflicting C++ headers
RUN rm -rf ${SYSROOT}/usr/include/c++* || true && \
    rm -rf ${SYSROOT}/usr/lib/libstdc++* || true && \
    rm -rf ${SYSROOT}/opt/csw/include/c++* || true && \
    rm -rf ${SYSROOT}/opt/csw/lib/libstdc++* || true

# Configure and build GCC
RUN mkdir gcc-build && cd gcc-build && \
    ../gcc-4.8.1/configure \
        --target=sparc-sun-solaris2.9 \
        --prefix=/opt/cross \
        --enable-languages=c,c++ \
        --disable-nls \
        --disable-libssp \
        --disable-doc \
        --with-cpu=v8 \
        --disable-v8plus \
        --with-sysroot=${SYSROOT} \
        --disable-comdat \
        --disable-werror \
        --disable-multilib \
        --enable-libstdcxx \
        --with-gnu-ld \
        --with-gnu-as \
        --disable-libitm \
        --disable-libgomp \
        --disable-libquadmath \
        --with-gxx-include-dir=/opt/cross/sparc-sun-solaris2.9/include/c++/4.8.1 \
        --enable-shared=yes \
        --enable-static=yes && \
    make CFLAGS="-w -fgnu89-inline" CXXFLAGS="-w -std=gnu++98" MAKEINFO=true -j$(nproc) all-gcc all-target-libgcc all-target-libstdc++-v3 && \
    make MAKEINFO=true install-gcc install-target-libgcc install-target-libstdc++-v3

# Compatibility headers
RUN cat > ${SYSROOT}/usr/include/solaris_compat.h << 'EOF'
#ifndef SOLARIS_COMPAT_H
#define SOLARIS_COMPAT_H

#include <sys/types.h>
#include <sys/int_types.h>

#ifndef _INT64_T
typedef long long int64_t;
typedef unsigned long long uint64_t;
#define _INT64_T
#endif

typedef int64_t Sint64;
typedef uint64_t Uint64;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STRTOLL_DECLARED
extern long long strtoll(const char *nptr, char **endptr, int base);
extern unsigned long long strtoull(const char *nptr, char **endptr, int base);
#define _STRTOLL_DECLARED
#endif

#ifdef __cplusplus
}
#endif

#endif
EOF

RUN cat > ${SYSROOT}/usr/include/math_compat.h << 'EOF'
#ifndef MATH_COMPAT_H
#define MATH_COMPAT_H

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef logf
static inline float logf(float x) { return (float)log((double)x); }
#endif
#ifndef sqrtf
static inline float sqrtf(float x) { return (float)sqrt((double)x); }
#endif
#ifndef powf
static inline float powf(float x, float y) { return (float)pow((double)x, (double)y); }
#endif
#ifndef fabsf
static inline float fabsf(float x) { return (float)fabs((double)x); }
#endif
#ifndef sinf
static inline float sinf(float x) { return (float)sin((double)x); }
#endif
#ifndef cosf
static inline float cosf(float x) { return (float)cos((double)x); }
#endif
#ifndef tanf
static inline float tanf(float x) { return (float)tan((double)x); }
#endif
#ifndef asinf
static inline float asinf(float x) { return (float)asin((double)x); }
#endif
#ifndef acosf
static inline float acosf(float x) { return (float)acos((double)x); }
#endif
#ifndef atanf
static inline float atanf(float x) { return (float)atan((double)x); }
#endif
#ifndef atan2f
static inline float atan2f(float y, float x) { return (float)atan2((double)y, (double)x); }
#endif
#ifndef expf
static inline float expf(float x) { return (float)exp((double)x); }
#endif
#ifndef floorf
static inline float floorf(float x) { return (float)floor((double)x); }
#endif
#ifndef ceilf
static inline float ceilf(float x) { return (float)ceil((double)x); }
#endif
#ifndef fmodf
static inline float fmodf(float x, float y) { return (float)fmod((double)x, (double)y); }
#endif

// Add missing rounding functions expected by <cmath>
#ifndef roundf
static inline float roundf(float x) { return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f); }
#endif
#ifndef round
static inline double round(double x) { return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5); }
#endif

#ifdef __cplusplus
}
#endif

#endif
EOF

# Fix library symlinks for X11
RUN cd ${SYSROOT}/opt/csw/lib && \
    for lib in X11 Xext; do \
        if [ -f lib${lib}.so ] && [ ! -L lib${lib}.so ]; then \
            ln -sf $(readlink lib${lib}.so | sed 's/\.so\..*/.so/') lib${lib}.so; \
        fi; \
    done

# CMake toolchain file
RUN cat > /opt/cross/solaris9-toolchain.cmake << EOF
set(CMAKE_SYSTEM_NAME SunOS)
set(CMAKE_SYSTEM_PROCESSOR sparc)
set(CMAKE_SYSTEM_VERSION 9)

set(CMAKE_C_COMPILER sparc-sun-solaris2.9-gcc)
set(CMAKE_CXX_COMPILER sparc-sun-solaris2.9-g++)

set(CMAKE_SYSROOT ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Path prioritization
set(CMAKE_PREFIX_PATH "${SYSROOT}/opt/csw")
set(CMAKE_LIBRARY_PATH "${SYSROOT}/opt/csw/lib")
set(CMAKE_INCLUDE_PATH "${SYSROOT}/opt/csw/include")

# X11 configuration
set(X11_INC_SEARCH_PATH
    "${SYSROOT}/opt/csw/include"
    "${SYSROOT}/usr/openwin/include"
)

set(X11_LIB_SEARCH_PATH
    "${SYSROOT}/opt/csw/lib"
    "${SYSROOT}/usr/openwin/lib"
)

set(CMAKE_C_FLAGS "--sysroot=${SYSROOT} -g -O0 -mcpu=v8 -mno-v8plus -D_REENTRANT" CACHE STRING "")
set(CMAKE_CXX_FLAGS "\${CMAKE_C_FLAGS} -g -O0 -std=gnu++98" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "--sysroot=${SYSROOT} -L${SYSROOT}/opt/csw/lib -Wl,-rpath-link=${SYSROOT}/opt/csw/lib" CACHE STRING "")

set(SDL_X11 ON CACHE BOOL "")
set(SDL_STATIC ON CACHE BOOL "")
set(SDL_SHARED OFF CACHE BOOL "")
set(SDL_OPENGL OFF CACHE BOOL "")
set(SDL_ATOMIC_DISABLED ON CACHE BOOL "")
EOF

COPY SDL3-3.2.16 SDL3
RUN cd SDL3 && rm -rf build && mkdir build && cd build && \
    cmake .. \
      -DCMAKE_TOOLCHAIN_FILE=/opt/cross/solaris9-toolchain.cmake \
      -DCMAKE_INSTALL_PREFIX=${SYSROOT}/usr \
      -DCMAKE_BUILD_TYPE=Release \
      -DSDL_TEST=OFF \
      -DSDL_OPENGLES=OFF \
      -DSDL_X11_XSHAPE=OFF \
      -DSDL_EXAMPLES=OFF && \
    make -j$(nproc) && make install

ENV CC="sparc-sun-solaris2.9-gcc"
ENV CXX="sparc-sun-solaris2.9-g++"
ENV CFLAGS="--sysroot=${SYSROOT} -mcpu=v8 -mno-v8plus -I${SYSROOT}/opt/csw/include"
ENV CXXFLAGS="${CFLAGS} -std=gnu++11"
ENV LDFLAGS="--sysroot=${SYSROOT} -L${SYSROOT}/opt/csw/lib -Wl,-rpath-link=${SYSROOT}/opt/csw/lib -lposix4 -lm -lsocket -lnsl -lresolv -lrt"
ENV SDL_CFLAGS="-I${SYSROOT}/usr/include/SDL3"
ENV SDL_LIBS="-L${SYSROOT}/usr/lib -lSDL3"

# Verification tests
WORKDIR /tmp

# Test C compilation
RUN echo '#include <stdio.h>\nint main(){printf("C test OK\\n");return 0;}' > test.c && \
    sparc-sun-solaris2.9-gcc test.c -o test_c \
    --sysroot=${SYSROOT} \
    -mcpu=v8 \
    -mno-v8plus \
    -I${SYSROOT}/opt/csw/include \
    -L${SYSROOT}/opt/csw/lib \
    -Wl,-rpath-link=${SYSROOT}/opt/csw/lib \
    -lposix4 -lm -lsocket -lnsl -lresolv -lrt && \
    file test_c && \
    echo "C compilation successful" 

# Test C++ compilation
RUN echo '#include <iostream>\nint main(){std::cout << "C++ test OK" << std::endl;return 0;}' > test.cpp && \
    sparc-sun-solaris2.9-g++ test.cpp -o test_cpp \
    --sysroot=${SYSROOT} \
    -mcpu=v8 \
    -mno-v8plus \
    -I${SYSROOT}/opt/csw/include \
    -std=gnu++11 \  
    -L${SYSROOT}/opt/csw/lib \
    -Wl,-rpath-link=${SYSROOT}/opt/csw/lib \
    -lposix4 -lm -lsocket -lnsl -lresolv -lrt -lstdc++ && \
    file test_cpp && \
    echo "C++ compilation successful"

# Test X11 compilation
RUN echo '#include <X11/Xlib.h>\n#include <stdio.h>\nint main(){Display *d=XOpenDisplay(NULL);printf("X11 test: %s\\n",d?"SUCCESS":"FAILED");if(d)XCloseDisplay(d);return 0;}' > test_x11.c && \
    sparc-sun-solaris2.9-gcc test_x11.c -o test_x11 \
    --sysroot=${SYSROOT} \
    -mcpu=v8 \
    -mno-v8plus \
    -I${SYSROOT}/opt/csw/include \
    -I${SYSROOT}/usr/openwin/include \
    -L${SYSROOT}/opt/csw/lib \
    -L${SYSROOT}/usr/openwin/lib \
    -Wl,-rpath-link=${SYSROOT}/opt/csw/lib:${SYSROOT}/usr/openwin/lib \
    -lX11 && \
    file test_x11 && \
    echo "X11 compilation successful" 

WORKDIR /workspace
COPY isle-portable ./isle-portable
COPY build_isle.sh .

# Run the build script
RUN chmod +x build_isle.sh && \
    ./build_isle.sh && \
    file ./isle-portable/isle && \
    echo "ISLE build successful"

# Package the final exe and libs with proper paths and stuffs
RUN mkdir -p /package/isle-portable && \
    mkdir -p /package/isle-portable/lib && \
    # Copy exe
    cp /workspace/isle-portable/build/isle /package/isle-portable/isle-sparc && \
    cp /workspace/isle-portable/build/liblego1.so /package/isle-portable/lib && \
    # Copy GCC runtime libs
    cp /opt/cross/sparc-sun-solaris2.9/lib/libgcc_s.so.1 /package/isle-portable/lib/ && \
    cp /opt/cross/sparc-sun-solaris2.9/lib/libstdc++.so.6 /package/isle-portable/lib/ && \
    # Copy SDL3
    cp /opt/cross/solaris9/usr/lib/libSDL3.a /package/isle-portable/lib/ && \
    # Copy compat libs
    cp /workspace/isle-portable/combined_compat/libcombined_compat.a /package/isle-portable/lib/ && \
    # Copy some system libs
    find /opt/cross/solaris9 \( -name 'libm.so.2' -o -name 'libc.so.1' -o -name 'libdl.so.1' \) -exec cp -v {} /package/isle-portable/lib/ \; && \
    # Copy X11
    find /opt/cross/solaris9/usr/openwin/lib -maxdepth 1 \( -name 'libX11.so*' -o -name 'libXext.so*' \) -exec cp -v {} /package/isle-portable/lib/ \; && \
    # Copy additional nonsense
    find /opt/cross/solaris9/opt/csw/lib -maxdepth 1 \( -name 'libsocket.so*' -o -name 'libnsl.so*' -o -name 'libresolv.so*' \) -exec cp -v {} /package/isle-portable/lib/ \;

# Create run script (beware of shell syntax)
RUN /bin/echo '#!/bin/sh' > /package/isle-portable/run_isle.sh && \
    /bin/echo 'BASEDIR=`dirname "$0"`' >> /package/isle-portable/run_isle.sh && \
    /bin/echo 'LD_LIBRARY_PATH="$BASEDIR/lib"' >> /package/isle-portable/run_isle.sh && \
    /bin/echo 'export LD_LIBRARY_PATH' >> /package/isle-portable/run_isle.sh && \
    /bin/echo 'exec "$BASEDIR/isle-sparc" "$@"' >> /package/isle-portable/run_isle.sh && \
    chmod +x /package/isle-portable/run_isle.sh

# Create tarball
RUN tar -cvf /workspace/isle-portable-solaris9-sparc.tar -C /package/isle-portable .

VOLUME ["/workspace"]
CMD ["/bin/bash"]