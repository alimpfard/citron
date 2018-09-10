@echo off
setlocal
    set CITRON_EXT_PATH=%~dp0&&ctr.exe eval -p "> " --pc "-> " --clear-errors --terminal-width 80 %*
endlocal
