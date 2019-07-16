@echo off
cd %APPVEYOR_BUILD_FOLDER%

echo Compiler: %COMPILER%
echo Arch: %MSYS2_ARCH%
echo Platform: %PLATFORM%
echo MSYS2 directory: %MSYS2_DIR%
echo MSYS2 system: %MSYSTEM%

if %COMPILER%==msys2 (
    @echo on
    SET "PATH=C:\%MSYS2_DIR%\usr\bin;%PATH%"
    pacman -S --noconfirm gcc pcre pcre-devel mingw-w64-i686-toolchain
    SET "CC=gcc.exe"
    make all
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
    dumpbin /dependents libcurl-4.dll
    dumpbin /dependents ctr.exe > deps
    dumpbin /exports ctr.exe > exports
    appveyor PushArtifact deps
    appveyor PushArtifact exports
    make distribute
)
