
#[[

cmake_minimum_required(VERSION 3.10)

project(Engine)

add_executable(${PROJECT_NAME} /src/Source.cpp)

add_library(Core STATIC 
	src/Backend.cpp
	src/Component.cpp
	src/Entity.cpp
)

add_library(Graphics STATIC
	src/Mesh.cpp
	src/Model.cpp
	src/stb.cpp
)

add_library(Editor STATIC
	src/Editor.cpp
	src/ImGuizmo.cpp
)

target_include_directories(Core PRIVATE ${CMAKE_SOURCE_DIR}/include/Core)
target_include_directories(Graphics PRIVATE ${CMAKE_SOURCE_DIR}/include/Graphics)
target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/include/Editor)


target_link_libraries(Core PRIVATE stb)
target_link_libraries(Core PRIVATE )
target_link_libraries(${PROJECT_NAME} PRIVATE Core Graphics Editor)


]]










cmake_minimum_required(VERSION 3.10)

project(Engine)
# Example Dependencies CMake Setup

# Add third-party library paths
include_directories(${CMAKE_SOURCE_DIR}/dependencies/include/)

# Add the executable
add_executable(${PROJECT_NAME} src/Source.cpp)

# Core Library
add_library(Core STATIC
    src/Backend.cpp
    src/Component.cpp
    src/Entity.cpp
    src/Util.cpp
)
target_include_directories(Core PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies/include/nlohmann
    ${CMAKE_SOURCE_DIR}/dependencies/include/spdlog
    ${CMAKE_SOURCE_DIR}/dependencies/include/fmt
)
target_link_libraries(Core PRIVATE spdlog fmt)

# Graphics Library
add_library(Graphics STATIC
    src/Mesh.cpp
    src/Model.cpp
    src/stb.cpp
)
target_include_directories(Graphics PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies/include/glad
    ${CMAKE_SOURCE_DIR}/dependencies/include/glm
    ${CMAKE_SOURCE_DIR}/dependencies/include/khr
    ${CMAKE_SOURCE_DIR}/dependencies/include/freetype
)
target_link_libraries(Graphics PRIVATE glad glm freetype stb_load)

# Editor Library
add_library(Editor STATIC
    src/Editor.cpp
    src/ImGuizmo.cpp
    src/SystemIcons.cpp
)
target_include_directories(Editor PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies/include/glfw
    ${CMAKE_SOURCE_DIR}/dependencies/include/assimp
)
target_link_libraries(Editor PRIVATE glfw assimp)


target_include_directories(Core PRIVATE                 # Set Core to Physics once classes are defined
    ${CMAKE_SOURCE_DIR}/dependencies/include/physx
)
target_link_libraries(Core PRIVATE physx)

# Link Everything to the Main Executable
target_link_libraries(${PROJECT_NAME} PRIVATE Core Graphics Editor Physics)