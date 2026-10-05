echo Build PureC project %1

echo C:\PC\PCMAKE.TTP> %~dp0\..\..\TOOLS\EMUEXEC\EMUEXEC.CFG
echo -B -C %ATARI_DEMOS_PATH% -v -F -w %1 >> %~dp0\..\..\TOOLS\EMUEXEC\EMUEXEC.CFG

start /B /WAIT %HATARI_PATH%\hatari.exe -W --machine st --fast-boot 1 --fast-forward 1 --fastfdc 1 --drive-a 0 --drive-b 0 --auto %ATARI_DEMOS_PATH%TOOLS\EMUEXEC\EMUEXEC.TOS --confirm-quit 0 --harddrive %HATARI_PATH%\..\zHatariHD --memsize 4 --tos %HATARI_PATH%\..\TOS\etos256fr.img --mono --cpuclock 32 --sound off --compatible 0 --cpu-exact 0 --blitter 0 --natfeats 1

