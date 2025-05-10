#ifndef BOTAN_TARGET_INFO_H_
#define BOTAN_TARGET_INFO_H_

#include <botan/build.h>

/**
* @file  target_info.h
*
* Automatically generated from
* 'configure.py --cpu=wasm --os=emscripten --cc=emcc --cc-bin=emcc --disable-shared --without-documentation --minimized-build --enable-modules=aes,sha2_32,sha2_64,hmac,pbkdf2,auto_rng,system_rng,hash,block,stream,modes,kdf,pubkey,rsa,ecdsa,ecdh --prefix=/volumes/bigcode/hydra_sdk/lib/wasm'
*
* Target
*  - Compiler: emcc  -s DISABLE_EXCEPTION_CATCHING=0 -std=c++20 -D_REENTRANT -I/usr/local/opt/libomp/include
*  - Arch: wasm
*  - OS: emscripten
*/

/*
* Configuration
*/
#define BOTAN_CT_VALUE_BARRIER_USE_ASM

[[maybe_unused]] static constexpr bool OptimizeForSize = false;



/*
* Compiler Information
*/
#define BOTAN_BUILD_COMPILER_IS_CLANG

#define BOTAN_COMPILER_INVOCATION_STRING "emcc  -I/usr/local/opt/libomp/include"


/*
* External tool settings
*/




/*
* CPU feature information
*/
#define BOTAN_TARGET_ARCH "wasm"

#define BOTAN_TARGET_ARCH_IS_WASM




/*
* Operating system information
*/
#define BOTAN_TARGET_OS_IS_EMSCRIPTEN

#define BOTAN_TARGET_OS_HAS_ATOMICS
#define BOTAN_TARGET_OS_HAS_DEV_RANDOM
#define BOTAN_TARGET_OS_HAS_POSIX1
#define BOTAN_TARGET_OS_HAS_SYSTEM_CLOCK


/*
* System paths
*/
#define BOTAN_INSTALL_PREFIX R"(/volumes/bigcode/hydra_sdk/lib/wasm)"
#define BOTAN_INSTALL_HEADER_DIR R"(include/botan-3)"
#define BOTAN_INSTALL_LIB_DIR R"(/volumes/bigcode/hydra_sdk/lib/wasm/lib)"
#define BOTAN_LIB_LINK ""
#define BOTAN_LINK_FLAGS ""

#define BOTAN_SYSTEM_CERT_BUNDLE "/etc/ssl/cert.pem"

#endif
