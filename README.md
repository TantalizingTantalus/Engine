# Read Me!

This is a simple engine written in C++ using OpenGL and GLFW.

# The Goal
My Goal is to learn the broader aspects of graphics engine programming. This is my first attempt at a larger system using Object Oriented Programming principles.

![alt text](/Engine/EngineDemo.gif?raw=true "title")

# File Browser
![alt text](/Engine/FileViewerScreenshot.png?raw=true "file_viewer")
Each folder is represented as "ImGui::TreeNodeEx()".\
Each file is represented as "ImGui::Selectable()".

## To do: 
- Sort viewer by folders first, then files
- UI Cleanup
- Update Scene Graph handling
- glDrawInstanced implementation so that models can be easily re-used
- Asset System
- Physics Engine Implementation

# Installation
Currently, I am not using valid CMake to build the project. The .sln is included in the root "Engine" directory to help with building the project.

# Libraries Used
- OpenGL
- GLFW
- glad
- glm
- spdlog
- Assimp
- stb_image
