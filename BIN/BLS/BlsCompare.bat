cd "%~d0%~p0\"
%~d0

SET TEXTDIFF="C:\Program Files (x86)\WinMerge\WinMergeU.exe"

BlsConvert.exe "%1" > ~1.txt.tmp
BlsConvert.exe "%2" > ~2.txt.tmp

%TEXTDIFF% ~1.txt.tmp ~2.txt.tmp
