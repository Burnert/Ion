# Ion

A C++ game engine, created with simplicity in mind.  

The main focus of this engine is to create retro style games quickly.  

It is also our, *not yet existing*, studio's in-house engine, that we will use to create our games.

## Clone

If you want to clone the repo, use the following Git command:  

`git clone --recurse-submodules "https://github.com/Burnert/Ion/"`

You can also use this batch script:  

```bat
@echo off
set /p branchname="Enter branch name to clone (press ENTER to clone master branch): "

if "%branchname%" equ "" (
	set branchname="master"
)

if exist Ion\ (
	rmdir /s /q Ion
	echo Removed the old Ion folder.
)

echo Cloning branch %branchname%...
git clone --recurse-submodules -b %branchname% "https://github.com/Burnert/Ion/"
if errorlevel 1 goto lCloneError
echo Repository has been cloned successfully.
cd Ion
goto lGenerateSln

:lCloneError
echo Could not clone branch %branchname%.
pause
goto lEnd

:lGenerateSln
set /p vsversion="Enter Visual Studio version (2019 or 2022): "
if "%vsversion%" neq "2019" if "%vsversion%" neq "2022" goto lGenerateSln
echo Generating Visual Studio %vsversion% solution...
call GenerateVS%vsversion%.bat < nul
echo Visual Studio %vsversion% solution has been generated successfully.

:lChoice1
set /p choice="Would you like to open the solution? [Y/N]: "
if /i "%choice%" equ "Y" goto lOpenSln
if /i "%choice%" equ "N" goto lEnd
goto lChoice1

:lOpenSln
echo Launching solution...
start Ion.sln

:lEnd
```

To use it, save it in a directory as a .bat file. It will create an **Ion** folder next to the file and clone the repo into it.

**The script will DELETE the Ion folder, if it already exists. Don't use it, if you've made any changes in your local repo!**

## Build Instructions

### 1. Project files

Project files are generated using Premake5.  
To generate project files run the appropriate script (ex. GenerateVS2019.bat).

### 2. Build

Use your IDE to build project binaries.

Two relevant files are generated when building:
- Ion.dll
- { *ApplicationName* }.exe

**Ion.dll** is the engine binary. Each library used by the engine is linked statically into this file.  
**{ _ApplicationName_ }.exe** is the application executable.

*This will most likely change in the future, because it's painful to work with sometimes.  
Either static linking will be used for applications, or the engine will be an .exe file and the application will be a .dll.  
Or it might even support both ways, but not the one which is currently used.*

Binaries are located inside the *Build* folder in the root directory of the project.

### 3. Run

Use your IDE to run your application.  

## Getting Started

Initial project setup is shown in **IonExample** project. The main file is [Example.cpp](IonExample/Source/Example.cpp).

For now that's all there is. I'll write documentation as I go. 🙃

---

There is no license at the moment, because the thing is not even close to being ready.

That being said, you cannot copy the source code, use it in your projects, nor redistribute it.

You can play around with it locally, and that's about it.

**It WILL change in the future.**
