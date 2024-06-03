## CrownFlame 2D Custom Game Engine

<div style="text-align:center">
	<img src="./resources/CrownFlame2D_Game_Engine.png" alt="CrownFlame 2D Custom Game Engine" />	
</div>

### About the Engine
The CrownFlame 2D Engine is being created to be a Cross Platform 2D Game Engine (Windows, Linux & MacOS) that is meant to be used for building 2D or 2.5D Role Playing Games (RPGs) with an easy to use API, and all the necessary modules included, such as Asset Creation & Importing, Vector Transformations, Animations, Audio,
Vertex & Fragment Shaders, Memory Management, Parallelization with CUDA and much more. This is a long term project. After this project is finished to a satisfactory level, my next step would be to build a 3D Game Engine version.

### Upcoming Release Features - Release 0
- Initializing OpenGL
- Creating a Debugging Wrapper for OpenGL Errors
- Displaying a Square on the screen
- Moving the displayed square using keyboard input

### Tech Stack
- C/C++
- OpenGL (Priority) (Cross Platform)
- DirectX12 (Windows Only)
- Vulkan (Cross Platform) (Overkill for 2D/2.5D, will come at the end)
- GLFW (Window Creation & Management)
- GLM (Mathematic Library for Graphics & Shaders)
- CUDA (Overkill for 2D/2.5D unless the game created needs optimization, will come at the end)
- CMake (Build Tool) (Cross Platform)

### 100% Completion Criteria - When the Game Engine is finalized
- Modules Present:
	- Asset Creation & Importing
	- Vector Transformations Wrapper
	- Animations Support
	- Audio (Background & On Action)
	- Vertex & Fragment Shaders Support
	- Memory Management Included
	- Parallelization with Nvidia CUDA Library (For Optimization)
	- Networking (For Multiplayer Support)
- Cross Platform Build & Gameplay (Priority: In that order)
	- Windows 10/11
	- Linux
	- MacOS
- Demo Game showcasing every feature listed above