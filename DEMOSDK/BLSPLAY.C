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

#if BLS_SCOREMODE_ENABLE

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\PC\EMUL.H"
#include "DEMOSDK\BLSZDUMP.H"

#include "DEMOSDK\BLSZPLAY.H" /* define wrapping to C or ASM version for each routine */


static void blsUpdateYM (BLSplayer* _player)
{
    SNDYMcommand* command = _player->ymcommands;
    u8 t;


    blsRASTER(0x440);

    SNDYMupdate (&_player->ymplayer);

    STDmcpy(_player->ymplayer.commands, command, sizeof(_player->ymplayer.commands));

    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++, command++)
    {
        command->justpressed    = false;
        command->forcesqrsync   = false;
        command->scorevolumeset = false;
        command->pitchbendrange = 0;
    }

    blsRASTER(0x50);
}


void BLSplayerInit(MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, BLZdmaMode _dmamode)
{
    u8 t;


    BLZplayerInit(_allocator, _player, _sndtrack, _dmamode);
    _player->speed      = 6;    /* default speed    */
    _player->speedcount = 1;    /* play at next vbl */

    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
    {
        _player->ymcommands[t].scorevolume = 15;
    }

    SNDYMinitPlayer (_allocator, & _player->ymplayer, &_sndtrack->YMsoundSet);
}



static void blsPlayCommonFx (BLSplayer* _player, BLSvoice* _voice, BLScell* cell, u8 currentrow, u8* delay)
{
    IGNORE_PARAM(_voice);

	switch (cell->fx & BLS_FX_MASK)
	{
	case BLSfx_SPEED:
		_player->speed = cell->value;
		break;

	case BLSfx_JUMP:
		_player->trackindex = cell->value;
		_player->row = 0;
		break;

	case BLSfx_PATTERN_BREAK:
		_player->trackindex++;
		if (_player->trackindex >= _player->sndtrack->trackLen)
		{
			_player->trackindex = 0;
		}
		_player->row = cell->value;
		break;

	case BLSfx_LOOP_START_SET:
		_player->loopstart = currentrow;
		break;

	case BLSfx_LOOP:
		if (_player->loopcount == 0)
		{
			_player->loopcount = cell->value;
			_player->row = _player->loopstart;
		}
		else if (_player->loopcount > 0)
		{
			_player->loopcount--;
			if (_player->loopcount > 0)
			{
				_player->row = _player->loopstart;
			}
		}
		break;

	case BLSfx_DELAY_PATTERN:
		*delay = _player->speed * cell->value;
		break;

	default:
		ASSERT(0);
	}
}


static u16 blsPlayPcmVoices(BLSplayer* _player, BLScell** pcells, u16 nb, u8 currentrow, u8* delay)
{
	BLSsoundTrack* sndtrack = (BLSsoundTrack*) _player->sndtrack;
	u16 i;


	for (i = 0; i < nb ; i++)
	{
		BLScell* cell = pcells[i];
        u8 fx = cell->fx;
        s16 voicenum = fx >> BLS_FX_VOICENUM_SHIFT;
        BLSvoice* voice = &_player->voices[voicenum];
        
        fx &= BLS_FX_MASK;


        if (voicenum >= BLS_NBVOICES)
        {
            break;
        }

		if (cell->precomputedKey != 0)
		{
			if (cell->precomputedKey == BLS_PRECOMPUTED_KEY_OFF)
			{
				voice->keys[0] = NULL;
				voice->arpeggioState = ArpeggioState_STOPPED;
				voice->current = 0;           
            }
			else
			{
                ASSERT(cell->precomputedKey <= sndtrack->nbKeys);
				voice->keys[0] = &sndtrack->keys[cell->precomputedKey - 1];
				voice->current = 0;
				voice->currentsource = 0;
				voice->volume = 0;
				voice->arpeggioState = ArpeggioState_STOPPED;
				voice->retrigger = 0;
				voice->sampledelay = 0;
			}       

            blsOptDump_startkey (_player->blitzDumper, voice);
        }

		switch (fx)
		{
		case BLSfx_NONE:
			if (voice->arpeggioState != ArpeggioState_STOPPED)
			{
				voice->arpeggioState = ArpeggioState_STOPPING;
			}
			voice->retrigger = 0;
			break;

		case BLSfx_VOLUME:
			voice->volume = cell->value;
			break;

		case BLSfx_ARPEGGIO:

			switch (voice->arpeggioState)
			{
			case ArpeggioState_STOPPED:
				voice->arpeggioState = ArpeggioState_STARTING;
				break;
			}

			{
				BLSprecomputedKey* key1 = &sndtrack->keys[cell->value];
				BLSprecomputedKey* key2 = &sndtrack->keys[cell->value2];

				if ((key1 != voice->keys[1]) || (key2 != voice->keys[2]))
				{
					voice->arpeggioState = ArpeggioState_STARTING;
					voice->current = voice->currentsource = 0;
                    voice->keys[1] = key1;
                    voice->keys[2] = key2;
                
                    blsOptDump_startkey (_player->blitzDumper, voice);
                }
			}
			break;

		case BLSfx_SETBALANCE:
			if ((i == 0) || (i == 3))
			{
				_player->volumeLeft2 = *(u16*)&cell->value;
				_player->volumeLeft2 = PCENDIANSWAP16(_player->volumeLeft2);
			}
			else
			{
				_player->volumeRight2 = *(u16*)&cell->value;
				_player->volumeRight2 = PCENDIANSWAP16(_player->volumeRight2);
			}
			break;

		case BLSfx_BITMASK:
			voice->mask = *(u16*)&cell->value;
			voice->mask = PCENDIANSWAP16(voice->mask);
			break;

		case BLSfx_SETSAMPLEOFFSET:
			if (voice->keys[0] != NULL)
			{
				u16 offset = *(u16*)(&cell->value);
				offset = PCENDIANSWAP16(offset);
                voice->current = STDmulu(voice->keys[0]->freqmul, offset) >> voice->keys[0]->freqmulshift;
                blsOptDump_setsampleoffset(_player->blitzDumper, voice, voice->current);
			}
			break;

		case BLSfx_RETRIGGER_SAMPLE:
			voice->retrigger = voice->retriggercount = cell->value;
			break;

		case BLSfx_DELAY_SAMPLE:
			voice->sampledelay = cell->value;
			break;

		default:
			blsPlayCommonFx(_player, voice, cell, currentrow, delay);
		}
	}

    return i;
}


static void blsPlayYmVoices (BLSplayer* _player, BLScell** pcells, u16 nb, u8 currentrow, u8* delay)
{
	u16 i;


	for (i = 0 ; i < nb ; i++)
	{
		BLScell* cell = pcells[i];
        u8 fx = cell->fx;
        SNDYMcommand* command = &_player->ymcommands[(fx >> BLS_FX_VOICENUM_SHIFT) - BLS_NBVOICES];

        fx &=  BLS_FX_MASK;

		if (cell->precomputedKey != 0)
		{
			if (cell->precomputedKey == BLS_PRECOMPUTED_KEY_OFF)
			{
				command->pressed = command->justpressed = false;
			}
			else
			{
				command->justpressed = true;
				command->pressed = true;
				command->key = cell->precomputedKey; /* C-1 is at index 1 */
				command->soundindex = cell->value2;
                command->finetune = 0;
			}
		}

		switch (fx)
		{
		case BLSfx_NONE:
            ASSERT(cell->precomputedKey != 0);
			break;

		case BLSfx_VOLUME:
			command->scorevolume = cell->value;
            command->scorevolumeset = true;
			break;

		case BLSfx_PORTAMENTO_TICKS:
			command->portamientoticks = cell->value;
			break;

        case BLSfx_PITCHBEND_TICKS:
            command->pitchbendticks = cell->value;
            break;

        case BLSfx_PITCHBEND_RANGE:
            command->pitchbendrange = cell->value;
            break;

        case BLSfx_SET_FINETUNE:
            command->finetune = (s8) cell->value;
            break;

		default:
			blsPlayCommonFx(_player, NULL, cell, currentrow, delay);
		}
	}
}


static void blsUpdateScore(BLSplayer* _player)
{
    BLSsoundTrack* sndtrack = (BLSsoundTrack*) _player->sndtrack;
    u8             patternindex = sndtrack->track[_player->trackindex];
    BLSpattern*	   pattern      = sndtrack->patterns + patternindex;


    _player->speedcount--;
    if ( _player->speedcount == 0 )
    {
        u8             currentrow   = _player->row;
		u16			   rowcellsindex= pattern->rowtocellindex[currentrow];
		u16			   nbrowcells   = pattern->rowtocellindex[currentrow + 1] - rowcellsindex;
        u8             delay        = 0;

        _player->pattern = patternindex; /* not used by replay */

                                         /* interpret pattern */
        {
			static BLScell defCell[BLS_NBVOICES] = 
            {
                {0, 0 << BLS_FX_VOICENUM_SHIFT, 0, 0},
                {0, 1 << BLS_FX_VOICENUM_SHIFT, 0, 0},
                {0, 2 << BLS_FX_VOICENUM_SHIFT, 0, 0},
                {0, 3 << BLS_FX_VOICENUM_SHIFT, 0, 0}
            };

			BLScell*  pcells [32];
			BLScell*  cell = rowcellsindex + pattern->cells;
            u16 lastloop = 0;
            s16 index = 0;
            u16 i;


            /* printf ("p=%d r=%d - nbcells %d ", _player->pattern, _player->row-1, nbrowcells); */

            for (i = 0; i < nbrowcells; i++, cell++)
            {
                s16 voicenum = cell->fx >> BLS_FX_VOICENUM_SHIFT;
                s16 min = voicenum < BLS_NBVOICES ? voicenum : BLS_NBVOICES;
                u16 t;

                for (t = lastloop ; t < min ; t++)
                {
                    pcells[index] = &defCell[t];
                    index++;
                }

                pcells[index] = cell;
                index++;
                lastloop = voicenum + 1;

                /* printf ("(v=%d, fx=%d, key=%d)", voicenum, cell->fx & BLS_FX_MASK, cell->precomputedKey); */
            }

            for (i = lastloop; i < BLS_NBVOICES ; i++)
            {
                pcells[index] = &defCell[i];
                index++;
            }

            /* printf("\n"); */

            ASSERT(index <= ARRAYSIZE(pcells));

			i = blsPlayPcmVoices (_player, pcells, index,     currentrow, &delay);
			blsPlayYmVoices (_player, pcells + i, index - i, currentrow, &delay);
		}

		/* reset speed for next interpretation */
        _player->speedcount = _player->speed + delay;
    }

    if (_player->speedcount == 1)
    {
        _player->row++;

        if (_player->row >= pattern->nbrows)
        {
            _player->row = 0;
            _player->loopstart = 0;
            _player->loopcount = 0;
            _player->trackindex++;
            if (_player->trackindex >= sndtrack->trackLen)
            {
                _player->maxframenum = _player->framenum;
                _player->framenum    = 0;
                _player->tracklooped = true;
                _player->trackindex  = 0;
            }
            else
            {
                blsOptDump_newpattern (_player->blitzDumper, sndtrack->track[_player->trackindex]);
            }
        }
    }
}


static void blsUpdateRunningEffects (BLSplayer* _player)
{
    u16 t;
    BLSvoice*   voice = _player->voices;        

    for (t = 0 ; t < BLS_NBVOICES ; t++, voice++)
    {
        if (voice->retrigger > 0)
        {
            voice->retriggercount--;
            if (voice->retriggercount == 0)
            {
                voice->current = 0;
                voice->retriggercount = voice->retrigger;
                blsOptDump_retriggersample(_player->blitzDumper, voice);
            }
        }

        switch (voice->arpeggioState)
        {
        case ArpeggioState_STARTING:
            voice->arpeggioState = ArpeggioState_RUNNING;
            blsOptDump_startarpegio(_player->blitzDumper, voice, voice->keys[1], voice->keys[2]);
            /* no break here !!! */

        case ArpeggioState_STOPPED:
            voice->arpeggioOffset = 0;
            break;

        case ArpeggioState_STOPPING:
            voice->arpeggioState  = ArpeggioState_STOPPED;
            voice->arpeggioOffset = 8;
            blsOptDump_stoparpegio(_player->blitzDumper, voice);
            /* no break here */

        case ArpeggioState_RUNNING:
            {
                u32 v;
                u16 freqmul;
                u8  freqmulshift;
                u16 currentSource = (u16) voice->currentsource;
                BLSprecomputedKey* key;

                voice->arpeggioOffset += 4;
                if (voice->arpeggioOffset == 12)
                {
                    voice->arpeggioOffset = 0;
                }

                key = voice->keys[voice->arpeggioOffset >> 2];
                freqmul       = key->freqmul;
                freqmulshift  = key->freqmulshift;

                v = STDmulu(currentSource, freqmul);
                v >>= freqmulshift;
                voice->current = v;               
            }
            break;
        }
    }
}


void BLSupdate (BLSplayer* _player)
{
#   ifndef __TOS__
    u8 playBuffer = EMULgetPlayOffset () >= BLS_NBBYTES_PERFRAME;
    {
        static u8 oldBuffer = 0;

        if (oldBuffer == playBuffer)
        {
            return;
        }

        oldBuffer = playBuffer;
    }
#   endif

    ASSERT( BLSisBlitzSndtrack(_player->sndtrack) == false );

    _player->framenum++;

    blsRASTER(0x50);

    blzUpdateSoundBuffers(_player);

    blsUpdateYM(_player);

    blsUpdateScore(_player);

    blsUpdateRunningEffects(_player);

    blzUpdateAllVoices(_player);

    blzSetDMABuffer (_player);

#   ifndef __TOS__
    blzUpdateMicrowireEmul(_player);

    {
        static void* PCdelayedbuffer = NULL;

        if (PCdelayedbuffer != NULL)
            EMULplaysound (PCdelayedbuffer, BLS_NBBYTES_PERFRAME, playBuffer != 0 ? 0 : BLS_NBBYTES_PERFRAME);

        PCdelayedbuffer = (void*)_player->dmabufstart;
    }
#   endif
}

/* -------------------------------------------------------------------------------
ASYNC PLAY ROUTINE
------------------------------------------------------------------------------- */
void BLSupdAsync(BLSplayer* _player)
{
    u8* cleared = (u8*) _player->buffertoupdate;


    /* as we do not re-use buffer, enforce update by disabling clear channel optimization */
    cleared += BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    *(u32*) cleared = 0;

    _player->framenum++;

    blsOptDump_newframe(_player->blitzDumper, &(_player->ymplayer.YMregs), _player->framenum);

    blsUpdateScore (_player);

    blsUpdateRunningEffects (_player);

    blsUpdateYM(_player);

    blzUpdateAllVoices (_player);

    blsOptDump_endframe(_player->blitzDumper);
}

u32 BLSasyncPlay (BLSplayer* _player, u8 _trackindex, void* _buffer, u32 _buffersize)
{
    u32 size = 0;


    _player->buffertoupdate = (s8*) _buffer;

    do
    {
        if (_trackindex == _player->trackindex)
        {
            if (_player->speedcount == 1)
            {
                break;
            }
        }

        ASSERT ( (size + BLS_NBBYTES_PERFRAME) < _buffersize );

        BLSupdAsync(_player);

        _player->buffertoupdate += BLS_NBBYTES_PERFRAME;
        size += BLS_NBBYTES_PERFRAME;
    }
    while (1);

    return size;
}


void BLSgoto (BLSplayer* _player, u8 _trackIndex)
{
    u16 i;

    _player->trackindex = _trackIndex;
    _player->row = 0;
    _player->loopstart = 0;
    _player->loopcount = 0;
    _player->speedcount = 1;

    for (i = 0 ; i < BLS_NBVOICES ; i++)
    {
        _player->voices[i].keys[0]       = NULL;
        _player->voices[i].arpeggioState = ArpeggioState_STOPPED;
    }
}

#endif /* BLS_SCOREMODE_ENABLE */
