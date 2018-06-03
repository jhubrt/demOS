# BLITSnd

## Initial ideas

In 2015 I have made some experiments using the blitter to mix sounds on
the STe. My idea was to try a new (fast) way to manage synthetic PCM
voices.  
Blitter is fast, can loop for free and has not been used for
sound yet (from what I was aware of...). So I have developed a small
prototype to demonstrate this approach (2 voices).  
I think it would have been cool to use this stuff in parallel with YM voices. 
But it would have been necessary to integrate it into an existing YM tracker
which I was not able to do...
* [Synthetic sound prototype](http://cyber.savina.net/sound/sound.htm)
* [Discussion thread on Atari forum](http://www.atari-forum.com/viewtopic.php?f=16&t=29097)

In november 2017 at the Alchemy 12 party, I have discussed with the
coder who made the 2nd entry at demo contest
[Eerie Forest - GX4000](https://www.youtube.com/watch?v=CPCyF71098o).
For the soundtrack he has made a routine using pre-transposed samples
stored on a cartridge.  
It remembered me a small 68k soundtrack routine I have coded in 1993 playing modules with
pre-transposed samples + .MOD converter tool in GFA basic. This
version was simplistic and I have never used it in the end...

What about trying this approach once again combined with my ideas to use blitter ?
