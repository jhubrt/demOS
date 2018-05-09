# BLITSnd

## Possible future improvements

### FX

Samples in ping pong mode

### Storage

*   Allows storage optimizations 1/3, 1/5, 1/6...
*   Allow transposing by x3 (19 semitones), by x6 (31 semitones), by x9
    (38 semitones)
*   =\> These two stuffs can have colliding cases (currently BLITsnd
    always uses powers of 2 and avoids these collisions) =\> would need
    to compute different combinations and find the optimum
*   =\> Allowing these values would need additional mul / div 
    (I currently use bit shifts instead) which may slow down the replay routine a bit

### Perform replay at conversion step

Perform the real score replay at conversion step in order to detect some
more optimizations:

*   Detect keys were volume is not used even if it is used for other
    keys of the same instrument
*   Try to use STe balance automatically instead of bit shift when
    possible

### Edition Tool

*   Instead of a .MOD converter would be great to have a dedicated edition tool...
*   Having a richer concept for instruments (like in YM trackers)
*   Having YM available in parallel :)

## TODO

*   Debug rendering when using STe microwire
* 	Port *BLSinitSample* in ASM
