@echo off
REM Build and Run Script for OpenGL Game Project
REM This script configures, builds, and runs the game automatically

echo ========================================
echo   OpenGL Game - Build and Run Script   
echo ========================================
echo.

REM Get the script directory (project root)
cd /d "%~dp0"

echo Project directory: %CD%
echo.

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
) else (
    echo Build directory already exists.
)

REM Navigate to build directory
cd build

REM Configure the project with CMake
echo Configuring project with CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo Configuration successful!
echo.

REM Build the project
echo Building project (Release configuration)...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo Build successful!
echo.

REM Check if executable exists
if not exist "Release\mygame.exe" (
    echo ERROR: Executable not found at Release\mygame.exe
    pause
    exit /b 1
)

REM Run the game
echo Starting game...
echo Game executable: %CD%\Release\mygame.exe
echo.
echo ========================================
echo            RUNNING GAME                
echo ========================================
echo.

REM Start the game
Release\mygame.exe

echo.
echo ========================================
echo          GAME FINISHED                 
echo ========================================
pause 