# Find required packages
find_package(OpenSSL REQUIRED)

# Library headers
set(HYDRA_ADDRESS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/address_generator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/geohash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/layered_matrix.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/vector_compression.hpp
)

# Library sources
set(HYDRA_ADDRESS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/address_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/geohash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layered_matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_compression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lmvs_address_generator.cpp
)

# Build static library
add_library(hydra_address STATIC ${HYDRA_ADDRESS_SOURCES})

# Include directories
target_include_directories(hydra_address
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(hydra_address
    PUBLIC
        hydra_crypto
        hydra_math
        hydra_qzkp
        hydra_compression_tvc
        hydra_compression_ost
        hydra_lmvs
        ${OPENSSL_LIBRARIES}
        GMP::GMP
)

# Install targets
install(TARGETS hydra_address
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HYDRA_ADDRESS_HEADERS}
    DESTINATION include/hydra_address
)
