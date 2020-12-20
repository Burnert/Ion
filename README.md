# Ion
A simple C++ game engine.

## Build Instructions

### 1. **Project files**

Project files are generated using Premake5.  
To generate project files for the Ion project run the appropriate script (ex. GenerateVS2019.bat).

### 2. **Build**

Use your IDE to build project binaries.

Two relevant files are generated when building:
- Ion.dll
- { *ApplicationName* }.exe

**Ion.dll** is the engine binary. Each library used by the engine is linked statically into this file.  
**{ _ApplicationName_ }.exe** is the application executable.

Binaries are located inside the *Build* folder in the root directory of the project.

### 3. **Run**

Use your IDE to run your application.  

## Getting Started

Initial project setup is shown in **IonExample** project. The main file is [Example.cpp](IonExample/Source/Example.cpp).

For now that's all there is. I'll write documentation as I go. 🙃
