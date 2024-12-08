set DEMOS_PATH=%~dp0..\..

echo %~nx1
echo D:\PROJECTS\DEMOS\WIZPLAY.TTP > %DEMOS_PATH%\TOOLS\EMUEXEC\EMUEXEC.CFG
echo D:\PROJECTS\DEMOS\RELAPSE\DATA\ZIKS\%~nx1 >> %DEMOS_PATH%\TOOLS\EMUEXEC\EMUEXEC.CFG

start /WAIT %HATARI_PATH%\hatari.exe -W --fast-boot 1 --fast-forward 0 --auto D:\PROJECTS\DEMOS\TOOLS\EMUEXEC\EMUEXEC.TOS --configfile %~dp0hatariwizplay.cfg --confirm-quit 0

pause

exit 0
