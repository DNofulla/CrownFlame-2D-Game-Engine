#!/usr/bin/env pwsh

# Build and Run Script for OpenGL Game Project
# This script configures, builds, and runs the game automatically

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  OpenGL Game - Build and Run Script   " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Function to check if last command was successful
function Check-LastCommand {
    param($message)
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: $message" -ForegroundColor Red
        exit 1
    }
}

# Get the script directory (project root)
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

Write-Host "Project directory: $scriptDir" -ForegroundColor Green
Write-Host ""

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Name "build" | Out-Null
}
else {
    Write-Host "Build directory already exists." -ForegroundColor Green
}

# Navigate to build directory
Set-Location "build"

# Configure the project with CMake
Write-Host "Configuring project with CMake..." -ForegroundColor Yellow
cmake -G "Visual Studio 17 2022" -A x64 ..
Check-LastCommand "CMake configuration failed"

Write-Host "Configuration successful!" -ForegroundColor Green
Write-Host ""

# Build the project
Write-Host "Building project (Release configuration)..." -ForegroundColor Yellow
cmake --build . --config Release
Check-LastCommand "Build failed"

Write-Host "Build successful!" -ForegroundColor Green
Write-Host ""

# Check if executable exists
$exePath = "Release\mygame.exe"
if (-not (Test-Path $exePath)) {
    Write-Host "ERROR: Executable not found at $exePath" -ForegroundColor Red
    exit 1
}

# Run the game
Write-Host "Starting game..." -ForegroundColor Cyan
Write-Host "Game executable: $(Resolve-Path $exePath)" -ForegroundColor Green
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "           RUNNING GAME                 " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Start the game
& ".\$exePath"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "         GAME FINISHED                  " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan 