md _logs

SET BLSFOLDER=U:\HDisk\STE_D\PROJECTS\DEMOS\BLSPLAY
SET BLSCONVERT=PC\Debug\BLSconvert_debug.exe
SET TEXTVIEWER="C:\Program Files (x86)\Notepad++\notepad++.exe"

%BLSCONVERT% %BLSFOLDER%\DATA\QUICKIE.MOD           > _logs\blsconvert_quickie.txt
%BLSCONVERT% %BLSFOLDER%\DATA\_QUICKIE.MOD          > _logs\blsconvert__quickie.txt
%BLSCONVERT% %BLSFOLDER%\DATA\QUICKIEX.XM           > _logs\blsconvert_quickiex.txt
%BLSCONVERT% %BLSFOLDER%\DATA\LOADER.MOD            > _logs\blsconvert_loader.txt
%BLSCONVERT% %BLSFOLDER%\DATA\DEMO_B4.MOD           > _logs\blsconvert_demo_b4.txt
%BLSCONVERT% %BLSFOLDER%\DATA\NUTEK3.MOD            > _logs\blsconvert_nutek3.txt

%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\ARPEGGIO.MOD > _logs\blsconvert_arpeggio.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\BALANCE.MOD  > _logs\blsconvert_balance.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\CLIENT.MOD   > _logs\blsconvert_client.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\DELAY.MOD    > _logs\blsconvert_delay.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\EXTENDED.MOD > _logs\blsconvert_extended.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\FINETUNE.MOD > _logs\blsconvert_finetune.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\JUMPS.MOD    > _logs\blsconvert_jumps.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\LONGLOOP.MOD > _logs\blsconvert_longloop.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\MASK.MOD     > _logs\blsconvert_mask.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\PATRLOOP.MOD > _logs\blsconvert_patrloop.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\SAMPLE.MOD   > _logs\blsconvert_sample.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\SPEED.MOD    > _logs\blsconvert_speed.txt
%BLSCONVERT% %BLSFOLDER%\DATA\UNITTEST\VOLUME.MOD   > _logs\blsconvert_volume.txt

start %TEXTVIEWER% _logs\blsconvert_loader.txt
start %TEXTVIEWER% _logs\blsconvert_quickie.txt
start %TEXTVIEWER% _logs\blsconvert__quickie.txt
start %TEXTVIEWER% _logs\blsconvert_quickiex.txt
start %TEXTVIEWER% _logs\blsconvert_demo_b4.txt

start %TEXTVIEWER% _logs\blsconvert_arpeggio.txt
start %TEXTVIEWER% _logs\blsconvert_balance.txt
start %TEXTVIEWER% _logs\blsconvert_client.txt
start %TEXTVIEWER% _logs\blsconvert_delay.txt
start %TEXTVIEWER% _logs\blsconvert_extended.txt
start %TEXTVIEWER% _logs\blsconvert_finetune.txt
start %TEXTVIEWER% _logs\blsconvert_jumps.txt
start %TEXTVIEWER% _logs\blsconvert_longloop.txt
start %TEXTVIEWER% _logs\blsconvert_mask.txt
start %TEXTVIEWER% _logs\blsconvert_patrloop.txt
start %TEXTVIEWER% _logs\blsconvert_sample.txt
start %TEXTVIEWER% _logs\blsconvert_speed.txt
start %TEXTVIEWER% _logs\blsconvert_volume.txt
