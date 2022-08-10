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
#include "DEMOSDK\BLSSND.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"

#ifdef DEMOS_LOAD_FROMHD

static void blsDumpSndtrackState (BLSsoundTrack* _sndtrack, FILE* _file)
{
    u16 w = 24;
    u16 t, i;
    BLSsample*         sample = _sndtrack->samples;
    BLSprecomputedKey* pkey   = _sndtrack->keys;


    for (t = 0 ; t < _sndtrack->nbKeys; t++)
    {
        static char tracep[] = 
        {
            "PKEY ----------        \n"
            "sampleindex  =         \n"
            "blittertrans =         \n"
            "freqmul      =         \n"
            "freqdiv      =         \n"
            "freqmulshift =         \n"
            "freqdivshift =         \n\n"
        };

        i = 15;
        STDuxtoa(&tracep[i], t                       , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->sampleIndex       , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->blitterTranspose  , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->freqmul           , 4);  i += w;
        STDuxtoa(&tracep[i], pkey->freqdiv           , 4);  i += w;
        STDuxtoa(&tracep[i], pkey->freqmulshift      , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->freqdivshift      , 2);  i += w;

        pkey++;

        fwrite (tracep, sizeof(tracep) - 1, 1, _file);
    }

    for (t = 0 ; t < _sndtrack->nbSamples ; t++)
    {
        static char tracep[] = 
        {
            "SAMPLE --------        \n"
            "length       =         \n"
            "loopstart    =         \n"
            "looplength   =         \n"
            "flags        =         \n\n"
        };

        i = 15;
        STDuxtoa(&tracep[i], t                       , 2);  i += w;
        STDuxtoa(&tracep[i], sample->sampleLen       , 6);  i += w;
        STDuxtoa(&tracep[i], sample->sampleLoopStart , 6);  i += w;
        STDuxtoa(&tracep[i], sample->sampleLoopLength, 4);  i += w;
        STDuxtoa(&tracep[i], sample->flags           , 4);  i += w;

        sample++;

        fwrite (tracep, sizeof(tracep) - 1, 1, _file);
    }
}

/* -------------------------------------------------------------------------------
    TEST MODE
------------------------------------------------------------------------------- */
static void blsDumpPlayerState (BLSplayer* _player, u32 _offset, FILE* _file)
{
    BLSvoice* voice = _player->voices;
    u16 i = 15;
    u16 w = 24;
	u16 w2 = w * BLS_NBVOICES;
    u16 t, j;

    static char tracep[] = 
    {
        "PLAYER --------        \n"
        "offset       =         \n"
        "speed        =         \n"
        "speedcount   =         \n"
        "trackindex   =         \n"
        "pattern      =         \n"
        "row          =         \n"
        "loopstart    =         \n"
        "loopcount    =         \n"
        "volumeLeft   =         \n"
        "volumeRight  =         \n"
    };

    static char trace[] = 
    {
        "key0         =          " "key0         =          " "key0         =          " "key0         =         \n"
        "key1         =          " "key1         =          " "key1         =          " "key1         =         \n"
        "key2         =          " "key2         =          " "key2         =          " "key2         =         \n"
        "current      =          " "current      =          " "current      =          " "current      =         \n"
        "currentsourc =          " "currentsourc =          " "currentsourc =          " "currentsourc =         \n"
        "mask         =          " "mask         =          " "mask         =          " "mask         =         \n"
        "volume       =          " "volume       =          " "volume       =          " "volume       =         \n"
        "sampledelay  =          " "sampledelay  =          " "sampledelay  =          " "sampledelay  =         \n"
        "retrigr      =          " "retrigr      =          " "retrigr      =          " "retrigr      =         \n"
        "retrigrcount =          " "retrigrcount =          " "retrigrcount =          " "retrigrcount =         \n"
        "arpegioState =          " "arpegioState =          " "arpegioState =          " "arpegioState =         \n"
        "arpegioOfset =          " "arpegioOfset =          " "arpegioOfset =          " "arpegioOfset =         \n"
        "\n\n"
    };

    STDutoa (&tracep[i], _player->framenum, 6);     i += w;
    STDuxtoa(&tracep[i], _offset            , 6);   i += w;
#   if BLS_SCOREMODE_ENABLE
    STDuxtoa(&tracep[i], _player->speed, 2);        i += w;
    STDuxtoa(&tracep[i], _player->speedcount, 2);   i += w;
#   endif
    STDuxtoa(&tracep[i], _player->trackindex, 2);   i += w;
#   if BLS_SCOREMODE_ENABLE
    STDuxtoa(&tracep[i], _player->pattern, 2);      i += w;
    STDuxtoa(&tracep[i], _player->row, 2);          i += w;
    STDuxtoa(&tracep[i], _player->loopstart, 2);    i += w;
    STDuxtoa(&tracep[i], _player->loopcount, 2);    i += w;
#   endif
    STDuxtoa(&tracep[i], _player->volumeLeft2, 4);  i += w;
    STDuxtoa(&tracep[i], _player->volumeRight2, 4); i += w;

    fwrite (tracep, sizeof(tracep) - 1, 1, _file);

    for (t = 0 ; t < BLS_NBVOICES ; t++)
    {
        i = 15 + t * w;

        for (j = 0 ; j < 3 ; j++)
        {
            if (voice->keys[j] != NULL)
                STDuxtoa(&trace[i], voice->keys[j]->sampleIndex, 2);
            else
                STDuxtoa(&trace[i], -1, 2);
            i += w2;
        }

        STDuxtoa(&trace[i], voice->current        , 8); i += w2;
        STDuxtoa(&trace[i], voice->currentsource  , 8); i += w2;
        STDuxtoa(&trace[i], voice->mask           , 4); i += w2;
        STDuxtoa(&trace[i], voice->volume         , 2); i += w2;
        STDuxtoa(&trace[i], voice->sampledelay    , 2); i += w2;
        STDuxtoa(&trace[i], voice->retrigger      , 2); i += w2;
        STDuxtoa(&trace[i], voice->retriggercount , 2); i += w2;
        STDuxtoa(&trace[i], voice->arpeggioState  , 2); i += w2;
        STDuxtoa(&trace[i], voice->arpeggioOffset , 2); i += w2;
        i += w2;

        voice++;
    }   

	fwrite (trace, sizeof(trace) - 1, 1, _file);
}


static void blsDumpPlayerLowLevelState (BLSplayer* _player, FILE* _file)
{
    BLSvoice* voice = _player->voices;
    u16 i = 15;
    u16 w = 24;
    u16 t, j;


    {
        static char tracep[] =
        {
            "PLAYER --------        \n"
            "trackindex   =         \n"
            "volumeLeft   =         \n"
            "volumeRight  =         \n"
        };

        STDutoa (&tracep[i], _player->framenum, 6);     i += w;
        STDuxtoa(&tracep[i], _player->trackindex, 2);   i += w;
        STDuxtoa(&tracep[i], _player->volumeLeft2, 4);  i += w;
        STDuxtoa(&tracep[i], _player->volumeRight2, 4); i += w;

        fwrite(tracep, sizeof(tracep) - 1, 1, _file);
    }

    {
        u16 w2 = w * BLS_NBVOICES;

        static char trace[] = 
        {
            "active       =          " "active       =          " "active       =          " "active       =         \n"
            "key0         =          " "key0         =          " "key0         =          " "key0         =         \n"
            "key1         =          " "key1         =          " "key1         =          " "key1         =         \n"
            "key2         =          " "key2         =          " "key2         =          " "key2         =         \n"
            "current      =          " "current      =          " "current      =          " "current      =         \n"
            "currentsourc =          " "currentsourc =          " "currentsourc =          " "currentsourc =         \n"
            "mask         =          " "mask         =          " "mask         =          " "mask         =         \n"
            "volume       =          " "volume       =          " "volume       =          " "volume       =         \n"
            "arpegioState =          " "arpegioState =          " "arpegioState =          " "arpegioState =         \n"
            "arpegioOfset =          " "arpegioOfset =          " "arpegioOfset =          " "arpegioOfset =         \n"
            "\n\n"
        };

        for (t = 0; t < BLS_NBVOICES; t++)
        {
#           if BLS_SCOREMODE_ENABLE
            bool active = voice->mirroractivation;
#           else
            bool active = (voice->sampledelay == 0) && (voice->keys[0] != NULL);
#           endif

            u8   volume = voice->volume;
            u32  current = voice->current;

            i = 15 + t * w;

            STDuxtoa(&trace[i], active, 1); i += w2;

            for (j = 0; j < 3; j++)
            {
                if (voice->keys[j] != NULL)
                    STDuxtoa(&trace[i], voice->keys[j]->sampleIndex, 2);
                else
                    STDuxtoa(&trace[i], -1, 2);

                i += w2;
            }

            if (active == false)
            {
                volume = 0; /* if key is null, ignore volume state */
                current = 0;
            }

            STDuxtoa(&trace[i], current, 8); i += w2;
            STDuxtoa(&trace[i], voice->currentsource, 8); i += w2;
            STDuxtoa(&trace[i], voice->mask, 4); i += w2;
            STDuxtoa(&trace[i], volume, 2); i += w2;
            STDuxtoa(&trace[i], voice->arpeggioState, 2); i += w2;
            STDuxtoa(&trace[i], voice->arpeggioOffset, 2); i += w2;
            i += w2;

            voice++;
        }

        fwrite(trace, sizeof(trace) - 1, 1, _file);
    }

    {
        static char ymtrace[] =
        {
            "freqA        =          " "freqB        =          " "freqC        =          " "\n"
            "freqnoise    =          " "             =          " "             =          " "\n"
            "mixer        =          " "mixer        =          " "mixer        =          " "\n"
            "level        =          " "level        =          " "level        =          " "\n"
            "freqenv_l    =          " "freqenv_h    =          " "envshape     =          " "\n"
            "\n\n"
        };

        u16 v;

        i = 15;

        for (t = 0 ; t < 3 ; t++, i += w)
        {
            *HW_YM_REGDATA = HW_YM_SEL_FREQCHA_H + t;
            v = HW_YM_GET_REG();
            v <<= 8;
            *HW_YM_REGDATA = HW_YM_SEL_FREQCHA_L + t;
            v |= HW_YM_GET_REG();

            STDuxtoa(&ymtrace[i], v, 4);
        }

        *HW_YM_REGDATA = HW_YM_SEL_FREQNOISE;
        STDuxtoa(&ymtrace[i], HW_YM_GET_REG(), 2); i += w;

        for (t = 0 ; t < 3 ; t++, i += w)
        {
            *HW_YM_REGDATA = HW_YM_SEL_LEVELCHA + t;
            STDuxtoa(&ymtrace[i], HW_YM_GET_REG(), 4);
        }
    }
}


#if BLS_SCOREMODE_ENABLE

#if BLS_ENABLE_OPTIMIZED_DUMP
static void blsExtSequenceAssignFrameNum(BLSplayer* player_)
{
    BLSextSequenceRow* seq = player_->sequence;
    u16 seqsize = player_->sequencesize;
    u16 t;

    if (player_->sequenceframenummap[player_->trackindex] == BLS_SEQ_FRAMENUM_UNDEFINED)
    {
        player_->sequenceframenummap[player_->trackindex] = player_->framenum;
    }

    for (t = 0 ; t < seqsize ; t++, seq++)
    {
        if (seq->framenum == BLS_SEQ_FRAMENUM_UNDEFINED)
        {
            if ((player_->trackindex == seq->trackindex) && (player_->row == seq->row))
            {
                seq->framenum = player_->framenum;
            }
        }
    }
}
#else 
#   define blsExtSequenceAssignFrameNum(player_)
#endif /* BLS_ENABLE_OPTIMIZED_DUMP */

void BLStestPlay (BLSplayer* _player, char* _filesamplename, char* _filetracename, u8 _mode)
{
    FILE* filetrace  = NULL;
    FILE* filesample = NULL;
    u32   offset     = 0;


    ASSERT( BLSisBlitzSndtrack(_player->sndtrack) == false );

    if (_filesamplename != NULL)
        filesample = fopen(_filesamplename, "wb");

    if (_filetracename != NULL)
    {
        filetrace = fopen(_filetracename , "wb");
        ASSERT(filetrace != NULL);

        if (filetrace != NULL)
        {
            blsDumpSndtrackState(_player->sndtrack, filetrace);
        }
    }

    _player->buffertoupdate = _player->buffer;

    do
    {
        if (filetrace != NULL)
        {
            if (_mode == 2)
                blsDumpPlayerState(_player, offset, filetrace);
            else
                blsDumpPlayerLowLevelState(_player, filetrace);
        }

/*        if (_player->framenum == 3595)
            printf("");*/

        blsExtSequenceAssignFrameNum(_player);

        BLSupdAsync(_player);

        if (filesample != NULL)
        {
		    fwrite (_player->buffer, BLS_NBBYTES_PERFRAME, 1, filesample);
        }

        offset += BLS_NBBYTES_PERFRAME;
    }
    while (_player->tracklooped == false);

    if (filesample != NULL)
    {
        fclose(filesample);
    }

    if (filetrace != NULL)
    {
        fclose(filetrace);
    }
}

#endif /* BLS_SCOREMODE_ENABLE */

void BLZtestPlay (BLSplayer* _player, char* _filesamplename, char* _filetracename, u8 _mode)
{
    FILE* filetrace  = NULL;
    FILE* filesample = NULL;
    u32   offset = 0;


    IGNORE_PARAM(_mode);

    ASSERT( BLSisBlitzSndtrack(_player->sndtrack) );

    if (_filesamplename != NULL)
        filesample = fopen(_filesamplename, "wb");

    if (_filetracename != NULL)
    {
        filetrace = fopen(_filetracename , "wb");
        ASSERT(filetrace != NULL);

        if (filetrace != NULL)
        {
            blsDumpSndtrackState(_player->sndtrack, filetrace);
        }
    }

    _player->buffertoupdate = _player->buffer;

    do
    {
        if (filetrace != NULL)
        {
            blsDumpPlayerLowLevelState(_player, filetrace);
        }

/*        if (_player->framenum == 3594)
            printf("");*/

        BLZupdAsync(_player);

        if (filesample != NULL)
        {
            fwrite (_player->buffer, BLS_NBBYTES_PERFRAME, 1, filesample);
        }
        offset += BLS_NBBYTES_PERFRAME;
    }
    while (_player->tracklooped == false);

    if (filesample != NULL)
    {
        fclose(filesample);
    }

    if (filetrace != NULL)
    {
        fclose(filetrace);
    }
}

#endif /* DEMOS_LOAD_FROMHD */


