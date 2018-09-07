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
    bash -lc "pacman -S --needed --noconfirm pacman-mirrors"
    bash -lc "pacman -S --needed --noconfirm git"
    bash -lc "pacman -Syu --noconfirm"
    bash -lc "pacman -S --needed --noconfirm mingw-w64-i686-toolchain make patch perl"
    REM let's see where the heck we exactly are
    bash -lc "ls" 
    bash -lc "make distribute"
)