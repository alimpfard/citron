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
    pacman -S --needed --noconfirm pacman-mirrors
    pacman -S --needed --noconfirm git
    pacman -Syu --noconfirm
    pacman -S --needed --noconfirm mingw-w64-i686-toolchain make patch perl
    make distribute
)