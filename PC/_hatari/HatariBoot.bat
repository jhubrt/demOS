set DEMOS_PATH=%~dp0..\..
echo %~n1
%HATARI_PATH%\hatari.exe -W --avi-vcodec BMP --configfile %DEMOS_PATH%\PC\_hatari\%~n1o.cfg

