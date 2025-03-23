# Read Me!

This is a simple engine written in C++ using OpenGL and GLFW.

# The Goal
My Goal is to learn the broader aspects of graphics engine programming. 

![demo gif of the engine in action!](/Engine/EngineDemo.gif?raw=true "Demo!")

# ECS (Entity Component System)
This engine uses an ECS design for ease of applying behavior to objects. Create a new entity, then add relevant components!

![view of the Entity hierarchy](/Engine/Hierarchy.png?raw=true "Hierarchy")

The newly created Entities are added to the scene hierarchy.

![view of the Component panel](/Engine/Components.png?raw=true "Components")

Modify added components directly in the component properties panel.

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
- PhysX
- FreeType
- ImGUI
- Imguizmo
