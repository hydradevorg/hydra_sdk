add_test([=[TVC.EncodeDecodeRoundtrip]=]  /Volumes/BIGCODE/hydra_sdk/build/src/hydra_compression/tvc_test [==[--gtest_filter=TVC.EncodeDecodeRoundtrip]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TVC.EncodeDecodeRoundtrip]=]  PROPERTIES WORKING_DIRECTORY /Volumes/BIGCODE/hydra_sdk/build/src/hydra_compression SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  tvc_test_TESTS TVC.EncodeDecodeRoundtrip)
