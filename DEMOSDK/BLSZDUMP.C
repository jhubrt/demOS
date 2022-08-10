/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
  and associated documentation files (the "Software"), 
  to deal in the Software without restriction, including without limitation the rights to use, 
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies 
  or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------------------------*/

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\BLSZDUMP.H"
#include "DEMOSDK\HARDWARE.H"

#if BLS_ENABLE_OPTIMIZED_DUMP


struct BLSoptDumpPCMVoiceState_
{
    u8   volume;
    u16  keyindex;
    u8   mask;
};
typedef struct BLSoptDumpPCMVoiceState_ BLSoptDumpPCMVoiceState;


struct BLSoptDumpPCMVoicePostInfo_
{
    bool nullkey;
};
typedef struct BLSoptDumpPCMVoicePostInfo_ BLSoptDumpPCMVoicePostInfo;


struct BLSoptDumpPCMVoiceSignal_
{
    bool voiceactivated;
    bool keystart;
    bool arpegiostart;
    bool arpegiostop;
    bool retriggersample;
    u32  sampleoffset;

    u8   arpegiokey1;
    u8   arpegiokey2;
};
typedef struct BLSoptDumpPCMVoiceSignal_ BLSoptDumpPCMVoiceSignal;


enum BLSpatternCaptureState_
{
    BLS_PCS_STARTING,
    BLS_PCS_CAPTURE,
    BLS_PCS_DONOTCAPTURE
};
typedef enum BLSpatternCaptureState_ BLSpatternCaptureState;

struct BLSoptDump_
{
    BLSsoundTrack* sndtrack;

    BLSoptDumpPCMVoiceState     pcmstates[2][BLS_NBVOICES];
    BLSoptDumpPCMVoiceState*    pcmcurrent;

    BLSoptDumpPCMVoiceSignal    pcmsignals[BLS_NBVOICES];
    BLSoptDumpPCMVoicePostInfo  postinfo[BLS_NBVOICES];

    YMregs                   lastYMregs;
    bool                     forceYMkeyframe;
    
    bool                     endpattern;
    u8                       endpatternindex;

    u8      back;
    u32     startoffset;

    FILE*   file;

    u8      patternssequence[256];
    YMregs  patternsstartstate[256];
    u8      sequenceindex;
    u8      lastpatternindex;
    bool    patternsplayed[256];
    u32     patternsoffset[256];
    u16     patternslength[256];

    u32     currentdata;
    BLSpatternCaptureState captureState;

    u16     frame;
};
typedef struct BLSoptDump_ BLSoptDump;

static void blsOptDump_storepattern(BLSoptDump* _dump, u8 _patternindex);


BLSoptDump* BLSoptDumpInit(FILE* _output, BLSsoundTrack* _sndtrack)
{
    u8 t;

    ASSERT(_output != NULL);

    BLSoptDump* dumper = (BLSoptDump*) malloc(sizeof(BLSoptDump));

    memset(dumper, 0, sizeof(BLSoptDump));

    dumper->file     = _output;
    dumper->sndtrack = _sndtrack;

    dumper->pcmcurrent    = dumper->pcmstates[dumper->back];
    dumper->startoffset   = ftell(_output);
    
    for (t = 0 ; t < BLS_NBVOICES ; t++)
    {
        dumper->pcmstates[0][t].mask = 0xFF;    // Force dump of mask of each voice at sndtrack start
        dumper->pcmstates[1][t].mask = 0;       
    }

    STDwriteL(_output, 0UL);

    blsOptDump_storepattern(dumper, _sndtrack->track[0]);

    return dumper;
}


bool blsOptCheckContext(BLSoptDump* _dumper, u8 t, u8 i)
{
    YMregs* p1    = &_dumper->patternsstartstate[t];
    YMregs* p2    = &_dumper->patternsstartstate[i];
    u8      mixer = p1->regscopy[HW_YM_SEL_IO_AND_MIXER] & 0x3F;
    bool    diff  = false;


    if (mixer != (p2->regscopy[HW_YM_SEL_IO_AND_MIXER] & 0x3F))
    {
        diff = true;
    }
    else
    {
        u8 t, t2;

        for (t = 0, t2 = 0; t < 3; t++, t += 2)
        {
            bool square = (mixer & 1) == 0;
            bool noise  = (mixer & 8) == 0;
            bool env    = false;            


            if (square)
                if ((p1->regscopy[HW_YM_SEL_FREQCHA_L + t2] != p2->regscopy[HW_YM_SEL_FREQCHA_L + t2]) || 
                    (p1->regscopy[HW_YM_SEL_FREQCHA_H + t2] != p2->regscopy[HW_YM_SEL_FREQCHA_H + t2]))
                    diff = true;

            if (noise)
                if (p1->regscopy[HW_YM_SEL_FREQNOISE] != p2->regscopy[HW_YM_SEL_FREQNOISE])
                    diff = true;

            if (square || noise)
                if (p1->regscopy[HW_YM_SEL_LEVELCHA + t] != p2->regscopy[HW_YM_SEL_LEVELCHA + t])
                    diff = true;

            if (p1->regscopy[HW_YM_SEL_LEVELCHA + t] == 16)
                env = true;

            if (env)
            {
                if ((p1->regscopy[HW_YM_SEL_FREQENVELOPE_L] != p2->regscopy[HW_YM_SEL_FREQENVELOPE_L]) || 
                    (p1->regscopy[HW_YM_SEL_FREQENVELOPE_H] != p2->regscopy[HW_YM_SEL_FREQENVELOPE_H]))
                    diff = true;

                if (p1->regscopy[HW_YM_SEL_ENVELOPESHAPE] != p2->regscopy[HW_YM_SEL_ENVELOPESHAPE])
                    diff = true;
            }

            mixer >>= 1;
        }
    }

    return diff;
}

void blsOptCheckPatternsJunction(BLSoptDump* _dumper)
{
    bool tested[256];
    u8 t;


    memset(tested, 0, sizeof(tested));

    for (t = 0; t < _dumper->sequenceindex; t++)
    {
        u16 patternindex = _dumper->patternssequence[t];
        u8 i;

        if (tested[patternindex] == false)
        {
            bool diff = false;            

            for (i = t + 1 ; i < _dumper->sequenceindex; i++)
            {
                if (patternindex == _dumper->patternssequence[i])
                {
                    if (blsOptCheckContext(_dumper, t, i))
                    {
                        if (diff == false)
                        {
                            printf ( "#WARNING: pattern $%x starts differently between track indexes $%x and ", _dumper->patternssequence[i], t);
                            diff = true;
                        }

                        printf ("$%x ", i);
                    }
                }
            }

            if (diff)
            {
                printf (". Check there is no audible glitch into BLZ version (look at doc to understand how to fix)\n" );
            }

            tested[patternindex] = true;
        }
    }
}


u8 blsOptCompactPatternsIndex(BLSoptDump* _dumper)
{
    u8 lastindex = 0;
    u16 t;

    for (t = 0; t < ARRAYSIZE(_dumper->patternsplayed); t++)
    {
        u16 i;

        _dumper->patternsplayed [lastindex] = _dumper->patternsplayed[t];
        _dumper->patternsoffset [lastindex] = _dumper->patternsoffset[t];
        _dumper->patternslength [lastindex] = _dumper->patternslength[t];

        for (i = 0; i < _dumper->sequenceindex; i++)
        {
            if (_dumper->patternssequence[i] == t)
            {
                _dumper->patternssequence[i] = lastindex;
            }
        }

        if (_dumper->patternsplayed[t])
        {
            lastindex++;
        }
    }

    return lastindex; 
}

void BLSoptDumpShutdown(BLSoptDump* _dumper)
{
    u8  lastindex;
    u32 pos = ftell(_dumper->file);
    u16 t;

    if (_dumper->captureState == BLS_PCS_CAPTURE)
    {
        u32 datalen = _dumper->currentdata - _dumper->patternsoffset[_dumper->lastpatternindex];
        ASSERT(datalen <= 0xFFFF);
        _dumper->patternslength[_dumper->lastpatternindex] = (u16) datalen;
        _dumper->patternsplayed[_dumper->lastpatternindex] = true;
    }

    blsOptCheckPatternsJunction(_dumper);

    lastindex = blsOptCompactPatternsIndex(_dumper);

    fseek(_dumper->file, _dumper->startoffset, SEEK_SET);
    STDwriteL (_dumper->file, _dumper->currentdata);
    fseek(_dumper->file, pos, SEEK_SET);
        
    STDwriteB(_dumper->file, _dumper->sequenceindex);
    fwrite(_dumper->patternssequence, sizeof(*_dumper->patternssequence), _dumper->sequenceindex, _dumper->file);

    STDwriteB(_dumper->file, lastindex);
    for (t = 0 ; t < lastindex ; t++)
    {
        STDwriteL(_dumper->file, _dumper->patternsoffset[t]);
    }
    for (t = 0 ; t < lastindex ; t++)
    {
        STDwriteW(_dumper->file, _dumper->patternslength[t]);
    }

    _dumper->file = NULL;

    free (_dumper);
}


void blsOptDump_newpattern(BLSoptDump* _dump, u8 _patternindex)
{
    _dump->endpattern = true;
    _dump->endpatternindex = _patternindex;
}

static void blsOptDump_storepattern(BLSoptDump* _dump, u8 _patternindex)
{
    _dump->patternssequence   [_dump->sequenceindex] = _patternindex;
    _dump->patternsstartstate [_dump->sequenceindex] = _dump->lastYMregs;
    _dump->sequenceindex++;

    switch(_dump->captureState)
    {
    case BLS_PCS_STARTING:
        _dump->captureState = BLS_PCS_CAPTURE;
        _dump->patternsplayed[_patternindex] = true;
        break;

    case BLS_PCS_CAPTURE:
        {
            u32 datalen = _dump->currentdata - _dump->patternsoffset[_dump->lastpatternindex];
            ASSERT(datalen <= 0xFFFF );
            _dump->patternslength[_dump->lastpatternindex]  = (u16) datalen;
            _dump->patternsplayed[ _dump->lastpatternindex] = true;
        }
        /* no break here */

    case BLS_PCS_DONOTCAPTURE:
        if (_dump->patternsplayed[_patternindex])
        {
            _dump->captureState = BLS_PCS_DONOTCAPTURE;
        }
        else
        {
            _dump->captureState = BLS_PCS_CAPTURE;
            _dump->patternsoffset[_patternindex] = _dump->currentdata;
        }
        break;
    }

    _dump->lastpatternindex = _patternindex;

    /* Make patterns independent => reset last value */
    if (_dump->pcmcurrent != NULL)
    {
        u8 t;

        for (t = 0; t < BLS_NBVOICES; t++)
        {
            _dump->pcmcurrent->keyindex = 0;
            _dump->pcmcurrent->volume   = 0;
        }

        _dump->forceYMkeyframe = true;
    }    
}

static void blsOptDump_outputPCMdata(BLSoptDump* _dump)
{
    u8  v;
    u8  compressed[100];
    u8  index = 0;
    u8* voices = &compressed[index++];

    memset(compressed, 0, sizeof(compressed));

    /*
    1 byte          V0desc V1desc V2desc V3desc V0active V1active V2active V3active

    foreach V<x>desc

    1 byte          Keystart ArpegioStart ArpegioStop MaskChanged RetriggerSample Volume 

    if ArpegOn
    2 bytes     keys 1 & 2 index

    if SampleIndexChanged
    1 byte SampleIndex

    if TranposeChanged
    1 byte Transpose (4 bits used)

    if MaskChanged
    1 byte Mask
    */

    for (v = 0; v < BLS_NBVOICES; v++)
    {
        bool activated = _dump->pcmsignals[v].voiceactivated;

        *voices |= activated << (3 - v);

        /*if (activated)*/
        {
            BLSoptDumpPCMVoiceState* laststate = _dump->pcmstates[_dump->back == 0];

            u8*  voicemask      = &compressed[index++];
            u16  keyindex       = _dump->pcmcurrent[v].keyindex;
            u8   volume         = _dump->pcmcurrent[v].volume;
            u8   mask           = _dump->pcmcurrent[v].mask;
            bool arpegiostart   = _dump->pcmsignals[v].arpegiostart;
            bool arpegiostop    = _dump->pcmsignals[v].arpegiostop;
            bool retriggersample= _dump->pcmsignals[v].retriggersample;
            bool writemask      = false;
            u8   currentmask    = 0x80;


            *voicemask = 0;

            if (_dump->pcmsignals[v].keystart)
            {
                *voicemask |= currentmask;
                compressed[index++] = (u8)keyindex;
                writemask = true;
            }

            currentmask >>= 1;
            if (arpegiostart)
            {
                *voicemask |= currentmask;
                compressed[index++] = _dump->pcmsignals[v].arpegiokey1;
                compressed[index++] = _dump->pcmsignals[v].arpegiokey2;
                writemask = true;
            }
            currentmask >>= 1;

            if (arpegiostop)
            {
                *voicemask |= currentmask;
                writemask = true;
            }
            currentmask >>= 1;

            if (laststate[v].mask != mask)
            {
                *voicemask |= currentmask;
                compressed[index++] = mask;
                writemask = true;
            }
            currentmask >>= 1;

            if (retriggersample)
            {
                *voicemask |= currentmask;
                writemask = true;
            }
            currentmask >>= 1;

            if ((laststate[v].volume != volume) ||
                (_dump->pcmsignals[v].keystart && (volume != 0)))
            {
                *voicemask |= currentmask;
                compressed[index++] = volume;
                writemask = true;
            }
            currentmask >>= 1;

            if (_dump->pcmsignals[v].sampleoffset != 0)
            {
                u32 offset = _dump->pcmsignals[v].sampleoffset;

                *voicemask |= currentmask;
                compressed[index++] = (u8)(offset >> 16);
                compressed[index++] = (u8)(offset >> 8);
                compressed[index++] = (u8) offset;
                writemask = true;
            }
            currentmask >>= 1;

            ASSERT(currentmask == 1);

            if (writemask)
                *voices |= 1 << (7 - v);
            else
                index--;
        }
    }

    if (_dump->captureState == BLS_PCS_CAPTURE)
    {
        fwrite(compressed, 1, index, _dump->file);
        _dump->currentdata += index;
    }

    /*
    arpeggiostatechanged     1 bit
    sampleindexchanged       1 bit
    transposechanged         1 bit
    maskchanged              1 bit
    volume                   4 bit

    sampleindex              8 bits
    transpose                4 bits signed
    volume                   3 bits unsigned
    currentsource            16 bits ? 24 bits
    */

    _dump->back ^= 1;
    _dump->pcmcurrent = _dump->pcmstates[_dump->back];
}


void blsOptDump_nullkeypost(struct BLSoptDump_* _dump, BLSvoice* _voice)
{
    _dump->postinfo[_voice->voicenum].nullkey = true;
}


static void blsOptDump_outputYMdata(BLSoptDump* _dump, YMregs* _ymregs)
{
    u8  compressed[100];
    u8  index = 0;
    u8* regmask = &compressed[index++];
    u8  i;
    u8  currentmask = 0x80;

    memset(compressed, 0, sizeof(compressed));

    { /* MIXER */
        u8 mixer = _ymregs->regscopy[HW_YM_SEL_IO_AND_MIXER] & 0x3F;
        if ((mixer != (_dump->lastYMregs.regscopy[HW_YM_SEL_IO_AND_MIXER] & 0x3F)) || _dump->forceYMkeyframe)
        {
            *regmask |= currentmask;
            compressed[index++] = mixer | 0xC0; /* port A & B out */
        }
        currentmask >>= 1;
    }

    /* FREQ SQUARES */
    for (i = 0 ; i < SND_YM_NB_CHANNELS ; i++)
    {
        u8   freqL           = _ymregs->regscopy[HW_YM_SEL_FREQCHA_L + (i << 1)];
        u8   freqH           = _ymregs->regscopy[HW_YM_SEL_FREQCHA_H + (i << 1)];
        bool squareRestarted = _ymregs->squareRestarted[i];


        if ((freqL != _dump->lastYMregs.regscopy[HW_YM_SEL_FREQCHA_L + (i << 1)]) ||
            (freqH != _dump->lastYMregs.regscopy[HW_YM_SEL_FREQCHA_H + (i << 1)]) ||
            squareRestarted || 
            _dump->forceYMkeyframe)
        {
            *regmask |= currentmask;

            compressed[index++] = freqL;
            compressed[index++] = freqH | (squareRestarted ? 0x80 : 0);
        }
        currentmask >>= 1;
    }

    {   /* ENV FREQ */
        u8   envfreqL           = _ymregs->regscopy[HW_YM_SEL_FREQENVELOPE_L];
        u8   envfreqH           = _ymregs->regscopy[HW_YM_SEL_FREQENVELOPE_H];


        if ((envfreqL != _dump->lastYMregs.regscopy[HW_YM_SEL_FREQENVELOPE_L]) ||
            (envfreqH != _dump->lastYMregs.regscopy[HW_YM_SEL_FREQENVELOPE_H]) ||
            _dump->forceYMkeyframe)
        {
            *regmask |= currentmask;

            compressed[index++] = envfreqL;
            compressed[index++] = envfreqH;
        }
        currentmask >>= 1;
    }

    {
        u8 level;
        {  /* LEVEL CHANNEL A + NOISE FREQ */
            u8   noisefreq    = _ymregs->regscopy[HW_YM_SEL_FREQNOISE];
            bool noisefreqset = noisefreq != _dump->lastYMregs.regscopy[HW_YM_SEL_FREQNOISE];


            level = _ymregs->regscopy[HW_YM_SEL_LEVELCHA];
            if (level != _dump->lastYMregs.regscopy[HW_YM_SEL_LEVELCHA] || noisefreqset || _dump->forceYMkeyframe)
            {
                *regmask |= currentmask;

                if (noisefreqset)
                {
                    compressed[index++] = level | 0x80;
                    compressed[index++] = noisefreq;
                }
                else
                {
                    compressed[index++] = level;
                }
            }
            currentmask >>= 1;
        }

        {  /* LEVEL CHANNEL B + ENV SHAPE */
            u8   envshape     = _ymregs->regscopy[HW_YM_SEL_ENVELOPESHAPE];
            bool envshapeset = envshape != _dump->lastYMregs.regscopy[HW_YM_SEL_ENVELOPESHAPE] || _ymregs->envshapeReselected;


            level = _ymregs->regscopy[HW_YM_SEL_LEVELCHB];
            if (level != _dump->lastYMregs.regscopy[HW_YM_SEL_LEVELCHB] || envshapeset || _dump->forceYMkeyframe)
            {
                *regmask |= currentmask;

                if (envshapeset)
                {
                    compressed[index++] = level | 0x80;
                    compressed[index++] = envshape;
                }
                else
                {
                    compressed[index++] = level;
                }
            }
            currentmask >>= 1;
        }

        /* LEVEL CHANNEL C */
        level = _ymregs->regscopy[HW_YM_SEL_LEVELCHC];
        if ((level != _dump->lastYMregs.regscopy[HW_YM_SEL_LEVELCHC]) || _dump->forceYMkeyframe)
        {
            *regmask |= currentmask;
            compressed[index++] = level;
        }

        ASSERT(currentmask == 1);
    }

    if (_dump->captureState == BLS_PCS_CAPTURE)
    {
        fwrite(compressed, 1, index, _dump->file);
        _dump->currentdata += index;
    }

    memcpy (_dump->lastYMregs.regscopy, _ymregs->regscopy, sizeof( _ymregs->regscopy));
    _dump->forceYMkeyframe = false;
}

void blsOptDump_newframe(BLSoptDump* _dump, YMregs* _ymregs, u16 _frame)
{
    u8 t;


    memset (_dump->pcmsignals, 0, sizeof(_dump->pcmsignals));

    for (t = 0 ; t < BLS_NBVOICES ; t++)
    {
        if (_dump->postinfo[t].nullkey)
        {
            _dump->postinfo[t].nullkey = false;
            _dump->pcmsignals[t].keystart = true;
            _dump->pcmcurrent[t].keyindex = 0;
        }
    }

    if (_dump->file != NULL)
    {
        ASSERT(_dump->captureState != BLS_PCS_STARTING);

        blsOptDump_outputYMdata(_dump, _ymregs);
    }

    _dump->frame = _frame;
}

void blsOptDump_endframe(BLSoptDump* _dump)
{
    if (_dump->file != NULL)
    {
        ASSERT(_dump->captureState != BLS_PCS_STARTING);

        blsOptDump_outputPCMdata(_dump);

        if (_dump->endpattern)
        {
            blsOptDump_storepattern(_dump, _dump->endpatternindex);
            _dump->endpattern = false;
        }
    }
}

void blsOptDump_voiceactivation(BLSoptDump* _dump, BLSvoice* _voice, bool _active)
{
    _dump->pcmsignals[_voice->voicenum].voiceactivated = _active;
}

void blsOptDump_startkey(BLSoptDump* _dump, BLSvoice* _voice)
{
    _dump->pcmsignals[_voice->voicenum].keystart = true;

    if (_dump->pcmcurrent != NULL)
    {
        BLSprecomputedKey* key = _voice->keys[0];
            
        if (key != NULL)
            _dump->pcmcurrent[_voice->voicenum].keyindex = (key - _dump->sndtrack->keys) + 1;
        else
            _dump->pcmcurrent[_voice->voicenum].keyindex = 0;
    }
}

void blsOptDump_startarpegio(struct BLSoptDump_* _dump, BLSvoice* _voice, BLSprecomputedKey* _key1, BLSprecomputedKey* _key2)
{
    _dump->pcmsignals[_voice->voicenum].arpegiostart = true;
    _dump->pcmsignals[_voice->voicenum].arpegiokey1  = _key1 - _dump->sndtrack->keys;
    _dump->pcmsignals[_voice->voicenum].arpegiokey2  = _key2 - _dump->sndtrack->keys;
}

void blsOptDump_stoparpegio(struct BLSoptDump_* _dump, BLSvoice* _voice)
{
    _dump->pcmsignals[_voice->voicenum].arpegiostop = true;
}

void blsOptDump_setsampleoffset(struct BLSoptDump_* _dump, BLSvoice* _voice, u32 _offset)
{
    _dump->pcmsignals[_voice->voicenum].sampleoffset = _offset;
}

void blsOptDump_retriggersample(struct BLSoptDump_* _dump, BLSvoice* _voice)
{
    _dump->pcmsignals[_voice->voicenum].retriggersample = true;
}

void blsOptDump_mask(BLSoptDump* _dump, BLSvoice* _voice, u16 _mask)
{
    if (_dump->pcmcurrent != NULL)
        _dump->pcmcurrent[_voice->voicenum].mask = (u8) _mask;
}

void blsOptDump_volume(BLSoptDump* _dump, BLSvoice* _voice, u8 _volume)
{
    if (_dump->pcmcurrent != NULL)
        _dump->pcmcurrent[_voice->voicenum].volume = _volume;
}

#endif
