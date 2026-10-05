echo %1 %2

echo %ATARI_DEVPACK_PATH%GEN.TTP> %~dp0\..\..\TOOLS\EMUEXEC\EMUEXEC.CFG
echo %1 %2 -Q >> %~dp0\..\..\TOOLS\EMUEXEC\EMUEXEC.CFG

start /B /WAIT %HATARI_PATH%\hatari.exe -W --fast-boot 1 --fast-forward 1 --fastfdc 1 --drive-a 0 --drive-b 0 --auto %ATARI_DEMOS_PATH%TOOLS\EMUEXEC\EMUEXEC.TOS --confirm-quit 0 --harddrive %HATARI_PATH%\..\zHatariHD --memsize 4 --tos %HATARI_PATH%\..\TOS\etos256fr.img --mono --cpulevel 2 --cpuclock 16 --natfeats 1

REM  I currently move output path in .S file to relative path. 
REM  They are relative to the current execution path which is EMUEXEC.TOS current path when run from Visual Studio through Hatari
REM  launching EMUEXEC.TOS with --auto option
