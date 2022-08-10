set DEMOS_PATH=%~dp0..\..\..

set TARGET=_debug
set ALL=/a ziks

%DEMOS_PATH%\BIN\EXTERN\logfilter.exe %DEMOS_PATH%\BLITZIK\TOOLS\BATCH\LogFilter.cfg %DEMOS_PATH%\BLITZIK\TOOLS\BATCH\blitzwav.bat
