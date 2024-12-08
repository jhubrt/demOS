echo %1 %2

echo D:\CODE\DEVPACK.3\BIN\GEN.TTP > TOOLS\EMUEXEC\EMUEXEC.CFG
echo %1 %2 -Q >> TOOLS\EMUEXEC\EMUEXEC.CFG

start /B /WAIT %HATARI_PATH%\hatari.exe -W --fast-boot 1 --fast-forward 0 --auto D:\PROJECTS\DEMOS\TOOLS\EMUEXEC\EMUEXEC.TOS --configfile PC\_hatari\hataridevpack.cfg --confirm-quit 0

