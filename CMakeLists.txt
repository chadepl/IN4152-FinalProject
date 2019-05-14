# Specifies the CMake version we require
cmake_minimum_required (VERSION 3.8)

# Store the name of our project in a variable called PROJECT
set(PROJECT "FinalProject")
# Sets the name of the project for our build
project (${PROJECT} LANGUAGES CXX C)

set (CMAKE_CXX_STANDARD 11)

# Set our Includes folder as the place to look for library includes
include_directories(${CMAKE_SOURCE_DIR}/3rdParty/Includes/)

# Set the source directory as an include folder for relative includes
include_directories(${CMAKE_SOURCE_DIR}/Source)


IF (APPLE)
#link_directories(/usr/local/lib)
ENDIF()

# Add source subdirectory which contains the source files
add_subdirectory(Source)

# Specify the name of the project executable and which sources should be used in the project
add_executable(${PROJECT}
    ${SOURCE_FILES}
)


# Specify the libraries to use when linking the executable
IF (WIN32)
target_link_libraries (${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Libraries/glfw3.lib)
target_link_libraries (${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Libraries/GDT/$<CONFIG>/GDT.lib)
ENDIF()
IF (APPLE)
target_link_libraries (${PROJECT} /usr/local/lib/libglfw.3.3.dylib)
target_link_libraries (${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Libraries/GDT/$<CONFIG>/libGDT.a)
ENDIF()


set(EXTERN_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/glm
    CACHE INTERNAL "" FORCE)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/Resources/"
        ${CMAKE_SOURCE_DIR}/Build/Resources)

IF (WIN32)
add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/3rdParty/Binaries/" ${CMAKE_BINARY_DIR}/$<CONFIG>)
ENDIF()
