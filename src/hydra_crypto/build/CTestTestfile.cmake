# CMake generated Testfile for 
# Source directory: /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto
# Build directory: /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(FalconSignatureTest "/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/build/test_falcon_signature")
set_tests_properties(FalconSignatureTest PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/CMakeLists.txt;79;add_test;/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/CMakeLists.txt;0;")
add_test(KyberKEMTest "/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/build/test_kyber_kem")
set_tests_properties(KyberKEMTest PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/CMakeLists.txt;83;add_test;/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/CMakeLists.txt;0;")
add_test(DilithiumSignatureTest "/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/build/test_dilithium_signature")
set_tests_properties(DilithiumSignatureTest PROPERTIES  _BACKTRACE_TRIPLES "/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/CMakeLists.txt;87;add_test;/Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/CMakeLists.txt;0;")
subdirs("_deps/httplib-build")
subdirs("_deps/json-build")
