cmake_minimum_required(VERSION 2.8)

project(Create3DVTKText)

# Find VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Set source files
aux_source_directory(. Srcs)

add_executable(create ${Srcs})
target_link_libraries(create ${VTK_LIBRARIES})
