
setlocal enabledelayedexpansion enableextensions

call :execute_citron package\prepare_install.ctr, "%SYSTEMDRIVE%\Program Files", Citron
robocopy package/prepared "%SYSTEMDRIVE%\Program Files\Citron" /s /e
set "a=%SYSTEMDRIVE%\Program Files\Citron\"
set "b=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Citron\Citron REPL.lnk"
call :path_from_file linkf "!b!"
mkdir "%linkf%"
echo Set oWS = WScript.CreateObject("WScript.Shell") > CreateShortcut.vbs
echo sLinkFile = "%b%" >> CreateShortcut.vbs
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> CreateShortcut.vbs
echo oLink.TargetPath = "%a%citron.bat" >> CreateShortcut.vbs
echo oLink.WorkingDirectory = "%a%" >> CreateShortcut.vbs
echo oLink.Description = "Project Citron 0.0.8 REPL" >> CreateShortcut.vbs
echo oLink.IconLocation = "%SYSTEMDRIVE%\Program Files\Citron\image\citron.ico" >> CreateShortcut.vbs
echo oLink.Save >> CreateShortcut.vbs
type CreateShortcut.vbs
cscript CreateShortcut.vbs
del CreateShortcut.vbs
assoc .ctr=citronfile
Ftype citronfile="%a%ctr.bat" %%1
goto eof

:execute_citron 
@rem exec citron script %1 with the rest of the  [8 because fuck batch]
call package\ctr.bat "%2" "%3" "%4" "%5" "%6" "%7" "%8" "%9"
EXIT /B 0

:path_from_file <resultVar> <pathVar>
(
    set "%~1=%~dp2"
    exit /b
)

:eof
endlocal