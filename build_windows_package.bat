@echo off
cd %APPVEYOR_BUILD_FOLDER%

echo Compiler: %COMPILER%
echo Arch: %MSYS2_ARCH%
echo Platform: %PLATFORM%
echo MSYS2 directory: %MSYS2_DIR%
echo MSYS2 system: %MSYSTEM%
 
if %COMPILER%==msys2 (
    @echo on
    SET "PATH=C:\%MSYS2_DIR%\%MSYSTEM%\bin;C:\%MSYS2_DIR%\usr\bin;%PATH%"
    pacman -S --noconfirm gcc
    SET "CC=gcc.exe"
    gcc -v
    make distribute
)