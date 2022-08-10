# BLITSnd - Optimizations

Obviously, BLITSnd is not a standard module replay routine. It is more
about playing with the constraints like you do when composing YM
chiptunes (but here you play with PCM samples + YM ;))

Keep in mind :

*   pre-transposing samples is costly. Pre-transpose is related to the
    size of samples but also to the number of different keys used for
    each samples
*   octave transposed melodies are cool for memory
*   blitter manage efficiently short looping waves. It is better to use
    this kind of samples for melodic lines as they will limit the impact
    of pre-transposing
*   drums are cool because they do not need to be transposed (be
    careful to use always the same key for a specific drum)
*   do not use volume FX (bit shift) when you can it do another way
*   do not use volume effect to tune volume of your sample (and then
    always use the same value). Amplify your sample directly with the
    "volume" value into the sample section instead.
*   try to use STe balance instead when possible for volume control
    because it is more precise and it is free, but balance affects voice
    0&3 (L) or 1&2 (R) in the same time. Else remember bit shifted
    samples use x2 more memory.
*   not looping sounds considerations :
    -   do not use linear interpolation for pre-transpose when not needed
        because it disable memory optimization by duplicating bytes
        dynamically
*   looping sounds considerations
    -   think at your sample in terms of duration when played at 25khz (to
        have an idea of the memory space used)
    -   you cannot exceed 16KB loop (16KB when pre-transposed at 25khz)
*   try FX by bit masking to bring diversity to your samples dynamically
    (will not use more memory nor CPU time). From my experience it is not bad
    with basslines. Can also do some overdrive effect on voices.
*   YM voices are very cheap to use. Can be great to use them as much as possible for melodies

