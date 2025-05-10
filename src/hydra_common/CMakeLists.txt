cmake_minimum_required(VERSION 3.16)
project(hydra_common VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library headers
set(HYDRA_COMMON_HEADERS
    ${CMAKE_SOURCE_DIR}/include/hydra_common/error_handling.hpp
    ${CMAKE_SOURCE_DIR}/include/hydra_common/secure_memory.hpp
    ${CMAKE_SOURCE_DIR}/include/hydra_common/resource_monitor.hpp
    ${CMAKE_SOURCE_DIR}/include/hydra_common/logging.hpp
)

# Create an interface library (header-only)
add_library(hydra_common INTERFACE)

# Include directories
target_include_directories(hydra_common INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# Compile features
target_compile_features(hydra_common INTERFACE cxx_std_20)

# Install headers
install(FILES ${HYDRA_COMMON_HEADERS}
    DESTINATION include/hydra_common
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    # Create test directory if it doesn't exist
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

    # Create test file if it doesn't exist
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_common_utils.cpp)
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_common_utils.cpp
"#include <gtest/gtest.h>
#include <hydra_common/error_handling.hpp>
#include <hydra_common/secure_memory.hpp>
#include <hydra_common/resource_monitor.hpp>
#include <hydra_common/logging.hpp>

// Test error handling
TEST(ErrorHandlingTest, BasicErrorHandling) {
    // Set up a test error handler
    bool error_handled = false;
    class TestErrorHandler : public hydra::common::IErrorHandler {
    public:
        TestErrorHandler(bool& handled_flag) : m_handled_flag(handled_flag) {}

        void handleError(
            const std::string& message,
            hydra::common::ErrorSeverity severity,
            hydra::common::ErrorCategory category
        ) override {
            m_handled_flag = true;
        }

        void handleException(const std::exception& e) override {
            m_handled_flag = true;
        }

    private:
        bool& m_handled_flag;
    };

    // Set the test error handler
    auto handler = std::make_shared<TestErrorHandler>(error_handled);
    hydra::common::ErrorHandling::setErrorHandler(handler);

    // Report an error
    hydra::common::ErrorHandling::reportError(
        \"Test error\",
        hydra::common::ErrorSeverity::WARNING,
        hydra::common::ErrorCategory::MEMORY
    );

    // Check that the error was handled
    EXPECT_TRUE(error_handled);
}

// Test secure memory
TEST(SecureMemoryTest, SecureWipe) {
    // Create a vector with test data
    std::vector<uint8_t> data(100, 0xFF);

    // Wipe the data
    hydra::common::secureWipeContainer(data);

    // Check that the data was wiped
    EXPECT_TRUE(data.empty());
}

// Test resource monitor
TEST(ResourceMonitorTest, ResourceLimits) {
    // Register a resource limit
    hydra::common::ResourceMonitor::getInstance().registerLimit(
        hydra::common::ResourceLimit(
            hydra::common::ResourceType::MEMORY,
            \"test_memory\",
            100,  // Soft limit
            200   // Hard limit
        )
    );

    // Update resource usage below soft limit
    bool result = hydra::common::ResourceMonitor::getInstance().updateUsage(\"test_memory\", 50);
    EXPECT_TRUE(result);

    // Update resource usage above soft limit but below hard limit
    result = hydra::common::ResourceMonitor::getInstance().updateUsage(\"test_memory\", 150);
    EXPECT_TRUE(result);

    // Update resource usage above hard limit
    result = hydra::common::ResourceMonitor::getInstance().updateUsage(\"test_memory\", 250);
    EXPECT_FALSE(result);
}

// Test logging
TEST(LoggingTest, BasicLogging) {
    // Set up a test log sink
    class TestLogSink : public hydra::common::ILogSink {
    public:
        void write(const hydra::common::LogMessage& message) override {
            m_last_message = message.message;
            m_message_count++;
        }

        void flush() override {}

        std::string getLastMessage() const { return m_last_message; }
        int getMessageCount() const { return m_message_count; }

    private:
        std::string m_last_message;
        int m_message_count = 0;
    };

    // Create and add the test sink
    auto sink = std::make_shared<TestLogSink>();
    hydra::common::Logger::getInstance().clearSinks();
    hydra::common::Logger::getInstance().addSink(sink);

    // Set global log level
    hydra::common::Logger::getInstance().setLevel(hydra::common::LogLevel::DEBUG);

    // Log a message
    hydra::common::Logger::getInstance().log(
        \"test_module\",
        hydra::common::LogLevel::INFO,
        \"Test message\"
    );

    // Check that the message was logged
    EXPECT_EQ(sink->getMessageCount(), 1);
    EXPECT_EQ(sink->getLastMessage(), \"Test message\");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}"
        )
    endif()

    add_executable(test_common_utils ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_common_utils.cpp)
    target_link_libraries(test_common_utils
        PRIVATE
            hydra_common
            GTest::GTest
            GTest::Main
    )
    add_test(NAME CommonUtilsTest COMMAND test_common_utils)
endif()
