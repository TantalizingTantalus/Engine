cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source and header files
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS Headers/*.h)

# Include directories
include_directories(Headers)
include_directories(dependencies)

# Specify dependencies
include_directories(
    dependencies/glfw/include
    dependencies/glad/include
    dependencies/glm
    dependencies/imgui
    dependencies/assimp/include
    dependencies/freetype/include
    dependencies/spdlog/include
    # Add others as needed
)

# Link libraries
link_directories(
    dependencies/glfw/lib
    dependencies/assimp/lib
    dependencies/freetype/lib
    # Add paths for other libraries as needed
)

# Add executable
add_executable(MyProject ${SOURCES} ${HEADERS})

# Link necessary libraries
target_link_libraries(MyProject
    glfw
    glad
    assimp
    freetype
    spdlog
    # Add others as needed
)

# If OpenGL is required
find_package(OpenGL REQUIRED)
target_link_libraries(MyProject OpenGL::GL)
