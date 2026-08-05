# BLSplayer

## Run

Those are TTP program : enter BLZ or BLS filenames to run in. On most recent TOS (like EmuTOS) just drag & drop BLZ/BLS file on TTP icon.

## BLZplay

BLZplay is the optimized player (ASM replay) version that only supports BLZ format. 

Available commands :
- **[ESC]** Quit
- **[X NUMPAD-]** Change display panel
- **[UP]** Restart current pattern
- **[Z]** Toggle xor pass in curve display
- **[RETURN]** Reset raster max
- **[BACKSPACE]** Sync routine to display start or VBL start
- **[LEFT]** Decrement song index
- **[RIGHT]** Increment song index
- **[F1-F10]** Goto song index x for key Fx 
- **[HOME]** Restart song
- **[NUMPAD 0]** Reinit settings (masks...)
- **[1-4]** Select current PCM channel from 1 to 4
- **[Q-P/A-H]** Change bit decimation mask to current channel

## BLSplay

BLSplay is the development / debugging non-optimized player version (C replay) that supports both BLZ and BLS formats. BLS is an intermediate format of the process producing the final BLZ (for development/test/debug).

Available commands :
- All commands from BLZplay plus...
- **[5-8]** Mute PCM channel from 1 to 4
- **[9-0-MINUS]** Mute YM channel from 1 to 3
- **[SPACE]** Toggle mute for current channel
- **[NUMPAD 1 2 4 5 6 7 8 9]** Offset volume on current PCM channel (only for sounds using volume control by shifting)
