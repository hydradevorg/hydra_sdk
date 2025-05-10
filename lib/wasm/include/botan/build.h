#ifndef BOTAN_BUILD_CONFIG_H_
#define BOTAN_BUILD_CONFIG_H_

/**
* @file  build.h
* @brief Build configuration for Botan 3.0.0
*
* Automatically generated from
* 'configure.py --cpu=wasm --os=emscripten --cc=emcc --cc-bin=emcc --disable-shared --without-documentation --minimized-build --enable-modules=aes,sha2_32,sha2_64,hmac,pbkdf2,auto_rng,system_rng,hash,block,stream,modes,kdf,pubkey,rsa,ecdsa,ecdh,curve25519,ed25519,kyber --prefix=/volumes/bigcode/hydra_sdk/lib/wasm'
*
* Target
*  - Compiler: emcc  -s DISABLE_EXCEPTION_CATCHING=0 -std=c++20 -D_REENTRANT  -I/usr/local/opt/libomp/include
*  - Arch: wasm
*  - OS: emscripten
*/

/**
 * @defgroup buildinfo Build Information
 */

/**
 * @ingroup buildinfo
 * @defgroup buildinfo_version Build version information
 * @{
 */

#define BOTAN_VERSION_MAJOR 3
#define BOTAN_VERSION_MINOR 0
#define BOTAN_VERSION_PATCH 0
#define BOTAN_VERSION_DATESTAMP 20230411


#define BOTAN_VERSION_RELEASE_TYPE "release"

#define BOTAN_VERSION_VC_REVISION "git:51b06ca93d1998d19927699f78b8d67539148dde"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

/**
 * @}
 */

/**
 * @ingroup buildinfo
 * @defgroup buildinfo_configuration Build configurations
 * @{
 */

/** How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 32


#define BOTAN_INSTALL_PREFIX R"(/volumes/bigcode/hydra_sdk/lib/wasm)"
#define BOTAN_INSTALL_HEADER_DIR R"(include/botan-3)"
#define BOTAN_INSTALL_LIB_DIR R"(/volumes/bigcode/hydra_sdk/lib/wasm/lib)"
#define BOTAN_LIB_LINK ""
#define BOTAN_LINK_FLAGS ""

#define BOTAN_SYSTEM_CERT_BUNDLE "/etc/ssl/cert.pem"

#ifndef BOTAN_DLL
  #define BOTAN_DLL 
#endif

/* Target identification and feature test macros */

#define BOTAN_TARGET_OS_IS_EMSCRIPTEN

#define BOTAN_TARGET_OS_HAS_ATOMICS
#define BOTAN_TARGET_OS_HAS_DEV_RANDOM
#define BOTAN_TARGET_OS_HAS_FILESYSTEM
#define BOTAN_TARGET_OS_HAS_POSIX1


#define BOTAN_BUILD_COMPILER_IS_CLANG



#define BOTAN_TARGET_ARCH_IS_WASM
#define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN







/**
 * @}
 */

/**
 * @ingroup buildinfo
 * @defgroup buildinfo_modules Enabled modules and API versions
 * @{
 */

/*
* Module availability definitions
*/
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_ASN1 20201106
#define BOTAN_HAS_AUTO_RNG 20161126
#define BOTAN_HAS_AUTO_SEEDING_RNG 20160821
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BIGINT 20210423
#define BOTAN_HAS_BIGINT_MP 20151225
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_CIPHER_MODES 20180124
#define BOTAN_HAS_CPUID 20170917
#define BOTAN_HAS_CURVE_25519 20170621
#define BOTAN_HAS_ECC_GROUP 20170225
#define BOTAN_HAS_ECC_KEY 20190801
#define BOTAN_HAS_ECC_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_ECDH 20131128
#define BOTAN_HAS_ECDSA 20131128
#define BOTAN_HAS_EC_CURVE_GFP 20131128
#define BOTAN_HAS_ED25519 20170607
#define BOTAN_HAS_EMSA_PSSR 20131128
#define BOTAN_HAS_ENTROPY_SOURCE 20151120
#define BOTAN_HAS_HASH 20180112
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_DRBG 20140319
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KYBER 20220107
#define BOTAN_HAS_KYBER_COMMON 20220107
#define BOTAN_HAS_MAC 20150626
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MGF1 20140118
#define BOTAN_HAS_MODES 20150626
#define BOTAN_HAS_NUMBERTHEORY 20201108
#define BOTAN_HAS_PASSWORD_HASHING 20210419
#define BOTAN_HAS_PBKDF 20180902
#define BOTAN_HAS_PBKDF2 20180902
#define BOTAN_HAS_PEM_CODEC 20131128
#define BOTAN_HAS_PK_PADDING 20131128
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_RSA 20160730
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SHA3 20161018
#define BOTAN_HAS_SHAKE 20161009
#define BOTAN_HAS_SHAKE_CIPHER 20161018
#define BOTAN_HAS_STATEFUL_RNG 20160819
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_SYSTEM_RNG 20141202
#define BOTAN_HAS_UTIL_FUNCTIONS 20180903
#define BOTAN_HAS_X25519 20180910


/**
 * @}
 */

/**
 * @addtogroup buildinfo_configuration
 * @{
 */

/** Local/misc configuration options (if any) follow */


/*
* Things you can edit (but probably shouldn't)
*/

/** How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 4096

#if defined(BOTAN_HAS_VALGRIND) || defined(BOTAN_ENABLE_DEBUG_ASSERTS)
   /**
    * @brief Prohibits access to unused memory pages in Botan's memory pool
    *
    * If BOTAN_MEM_POOL_USE_MMU_PROTECTIONS is defined, the Memory_Pool
    * class used for mlock'ed memory will use OS calls to set page
    * permissions so as to prohibit access to pages on the free list, then
    * enable read/write access when the page is set to be used. This will
    * turn (some) use after free bugs into a crash.
    *
    * The additional syscalls have a substantial performance impact, which
    * is why this option is not enabled by default. It is used when built for
    * running in valgrind or debug assertions are enabled.
    */
   #define BOTAN_MEM_POOL_USE_MMU_PROTECTIONS
#endif

/**
* If enabled uses memset via volatile function pointer to zero memory,
* otherwise does a byte at a time write via a volatile pointer.
*/
#define BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO 1

/**
* Normally blinding is performed by choosing a random starting point (plus
* its inverse, of a form appropriate to the algorithm being blinded), and
* then choosing new blinding operands by successive squaring of both
* values. This is much faster than computing a new starting point but
* introduces some possible corelation
*
* To avoid possible leakage problems in long-running processes, the blinder
* periodically reinitializes the sequence. This value specifies how often
* a new sequence should be started.
*/
#define BOTAN_BLINDING_REINIT_INTERVAL 64

/**
* Userspace RNGs like HMAC_DRBG will reseed after a specified number
* of outputs are generated. Set to zero to disable automatic reseeding.
*/
#define BOTAN_RNG_DEFAULT_RESEED_INTERVAL 1024

/** Number of entropy bits polled for reseeding userspace RNGs like HMAC_DRBG */
#define BOTAN_RNG_RESEED_POLL_BITS 256

#define BOTAN_RNG_RESEED_DEFAULT_TIMEOUT std::chrono::milliseconds(50)

/**
* Specifies (in order) the list of entropy sources that will be used
* to seed an in-memory RNG.
*/
#define BOTAN_ENTROPY_DEFAULT_SOURCES \
   { "rdseed", "hwrng", "getentropy", "system_rng", "system_stats" }

/** Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* Check for a common build problem */

#if defined(BOTAN_TARGET_ARCH_IS_X86_64) && ((defined(_MSC_VER) && !defined(_WIN64)) || \
                                             (defined(__clang__) && !defined(__x86_64__)) || \
                                             (defined(__GNUG__) && !defined(__x86_64__)))
    #error "Trying to compile Botan configured as x86_64 with non-x86_64 compiler."
#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_32) && ((defined(_MSC_VER) && defined(_WIN64)) || \
                                             (defined(__clang__) && !defined(__i386__)) || \
                                             (defined(__GNUG__) && !defined(__i386__)))

    #error "Trying to compile Botan configured as x86_32 with non-x86_32 compiler."
#endif

#include <botan/compiler.h>

#endif

