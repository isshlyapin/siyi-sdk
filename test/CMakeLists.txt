enable_testing()

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(crc16_test test_crc16.cxx)
target_compile_features(crc16_test PRIVATE cxx_std_23)
target_link_libraries(crc16_test PRIVATE siyi-sdk GTest::GTest GTest::Main)

add_executable(protocol_test test_protocol.cxx)
target_compile_features(protocol_test PRIVATE cxx_std_23)
target_link_libraries(protocol_test PRIVATE siyi-sdk GTest::GTest GTest::Main)

add_executable(sender_test test_sender.cxx)
target_compile_features(sender_test PRIVATE cxx_std_23)
target_link_libraries(sender_test PRIVATE siyi-sdk GTest::GTest GTest::Main)

add_executable(receiver_test test_receiver.cxx)
target_compile_features(receiver_test PRIVATE cxx_std_23)
target_link_libraries(receiver_test PRIVATE siyi-sdk GTest::GTest GTest::Main)

gtest_discover_tests(crc16_test)
gtest_discover_tests(protocol_test)
gtest_discover_tests(sender_test)
gtest_discover_tests(receiver_test)