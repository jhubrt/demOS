cd %~d0
cd %DEMOS_PATH%

md %DEMOS_PATH%\BLITZIK\DATABIN\ZIKS

%DEMOS_PATH%\BIN\EXTERN\wmake %ALL% /f %DEMOS_PATH%\BLITZIK\TOOLS\BATCH\blitzwav.mak 
  
%DEMOS_PATH%\BIN\TOOLS\Imager.exe %DEMOS_PATH%\BLITZIK\BLITZWAV.INI %DEMOS_PATH%\

type %DEMOS_PATH%\BLITZIK\BLITZWAV.TXT
