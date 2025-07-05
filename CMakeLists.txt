cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE vcpkg/scripts/buildsystems/vcpkg.cmake)

project(awscurl VERSION 1.0)

add_executable(${PROJECT_NAME} awscurl.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE CLI11::CLI11)

find_package(AWSSDK CONFIG COMPONENTS core sts REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${AWSSDK_LIBRARIES})
