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

#define SYNTH_C

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYNTHYM.H"
#include "DEMOSDK\SYNTHYMD.H"

#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\PC\EMUL.H"
#include <math.h>

static char* sndYMdelims = "\n\r";

#if SNDYM_REGS_MIRRORING
#define SND_YM_SET_REG(REGNUM,DATA,YMREGS) { HW_YM_SET_REG(REGNUM,DATA); YMREGS.regscopy[REGNUM] = DATA; if (REGNUM == HW_YM_SEL_ENVELOPESHAPE) YMREGS.envshapeReselected = true; }
#define SND_YM_REGS_NEWFRAME(YMREGS) { u8 t; YMREGS.envshapeReselected = false; for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++) YMREGS.squareRestarted[t] = false; }
#define SND_YM_REGS_SQUARE_RESTART(INDEX,YMREGS) YMREGS.squareRestarted[INDEX] = true
#else
#define SND_YM_SET_REG(REGNUM,DATA,YMREGS)  HW_YM_SET_REG(REGNUM,DATA)
#define SND_YM_REGS_NEWFRAME(YMREGS)
#define SND_YM_REGS_SQUARE_RESTART(INDEX,YMREGS)
#endif


#ifdef DEMOS_LOAD_FROMHD
static char sndYM_g_Error[256] = "";

char* SNDYMgetError(void) 
{
	return sndYM_g_Error;
}

static u32 getFileSize (FILE* _file)
{
    u32 filesize;

    fseek (_file, 0, SEEK_END);
    filesize = ftell(_file);
    fseek (_file, 0, SEEK_SET);

    return filesize;
}

static char* sndYMtoken (char* _init, char* _p)
{
    do
    {
        _p = strtok(_init, sndYMdelims);
        _init = NULL;
        if (_p == NULL)
        {
            return NULL;
        }

        if ( *_p != ';' )
        {
            while ((*_p == ' ') || (*_p == '\t'))
            {
                _p++;
            }

            if (*_p != 0)
            {
                char* p2 = strchr(_p, ' ');
                if ( p2 != NULL)
                    *p2 = 0;

                p2 = strchr(_p, '\t');
                if ( p2 != NULL)
                    *p2 = 0;

                return _p;
            }
        }
    }
    while (1);
}

static char* sndYMfirstToken (char* _init)
{
    return sndYMtoken(_init, _init);
}

static char* sndYMnextToken (char* _p)
{
    return sndYMtoken(NULL, _p);
}


/*
Most people know that writing to the YM buzzer waveform register retriggers  the
envelope from its  start (as stated  in the YM2149  datasheet). Whats not  often
known is  that its  possible to  retrigger the  squarewave in  this way as well.
Heres a bit of code that does just that for YM squarewave channel A:

        move.l      #$00000000,$ffff8800.w
        move.l      #$01000000,$ffff8800.w
        move.l      #$0000xx00,$ffff8800.w
        move.l      #$0100xx00,$ffff8800.w
*/

static char* sndYMsoundSet_load(MEMallocator* _allocator, SNDYMsound* _sound, char _name[24], char* p)
{
    s8  levels[256];
    bool end = false;


    p = sndYMnextToken(p);  
    strncpy(_name, p, sizeof(SNDYMsoundName));
    _name[sizeof(SNDYMsoundName)-1] = 0;

    _sound->nbcurves = 0;

    do
    {
        SNDYMcurve* curve = &_sound->curves[_sound->nbcurves];
        u16 phase = 0;
        u16 i = 0;
        s16 min = 0;
        s16 max = 0;


        p = sndYMnextToken(p);    
        if (strcmp(p, "END") == 0)
        {
            end = true;
        }
        else
        {
            if (_sound->nbcurves >= SND_YM_NBMAX_CURVES)
            {
                sprintf (sndYM_g_Error, "#ERROR: sound '%s' - too many curves declared (max 8)\n", _name);
                goto Error;
            }

            DEFAULT_CONSTRUCT(curve);

            if (strcmp(p, "AMP") == 0)
            {
                curve->curveType = SND_YM_AMPLITUDE;
                min = 0;
                max = 16;
            }
            else if (strcmp(p, "FREQ") == 0)
            {
                curve->curveType = SND_YM_FREQ;
                min = -64;
                max =  64;
            }
            else if (strcmp(p, "FREQFINE") == 0)
            {
                curve->curveType = SND_YM_FINE_FREQ;
                min = -128;
                max =  127;
            }
            else if (strcmp(p, "FREQFIX") == 0)
            {
                curve->curveType = SND_YM_FIX_FREQ;
                min = 0;
                max = 127;
            }
            else if (strcmp(p, "MIX") == 0)
            {
                curve->curveType = SND_YM_MIX;
                min = 0;
                max = 3;
            }
            else if (strcmp(p, "ENVFREQ") == 0)
            {
                curve->curveType = SND_YM_ENVFREQ;
                min = -64;
                max = 64;
            }
            else if (strcmp(p, "ENVFREQFINE") == 0)
            {
                curve->curveType = SND_YM_FINE_ENVFREQ;
                min = -128;
                max = 127;
            }
            else if (strcmp(p, "ENVSHAPE") == 0)
            {
                curve->curveType = SND_YM_ENVSHAPE;
                min = 0;
                max = 15;
            }
            else if (strcmp(p, "NOISEFREQ") == 0)
            {
                curve->curveType = SND_YM_NOISEFREQ;
                min = 0;
                max = 31;
            }
            else if (strcmp(p, "SQRSYNC") == 0)
            {
                curve->curveType = SND_YM_SQUARESYNC;
                min = 0;
                max = 1;
            }
            else
            {
                sprintf (sndYM_g_Error, "#ERROR: sound '%s' - unknown curve type : '%s'\n", _name, p);
                goto Error;
            }

            if ((min == 0) && (max == 0))
            {
                curve->tick = 0;
                curve->len = 0;
                curve->levels = NULL;
            }
            else
            {
                p = sndYMnextToken(p);
                curve->tick = atoi(p);

                if (curve->tick == 0)
                {
                    sprintf (sndYM_g_Error, "#ERROR: sound '%s' - curve tick should be > 0\n", _name);
                    goto Error;
                }

                do
                {
                    s8 len;

                    p = sndYMnextToken(p);
                    len = (s8) strlen(p);

                    switch (p[0])
                    {
                    case '|':
                        p++;
                        len--;
                        break;

                    case 'a':
                    case 's':
                    case 'd':
                    case 'r':
                        len = atoi(p+1);
                        break;

                    case 'A':
                    case 'S':
                    case 'D':
                    case 'R':
                        if (p[len-1] == '|')
                        {
                            len--;
                            len = -len;
                        }
                        break;

                    case '<':
                        break;

                    default:
                        ASSERT(0);
                    }

                    switch(p[0])
                    {
                    case 'A':
                    case 'a':
                        if (phase != 0)
                        {
                            sprintf (sndYM_g_Error, "#ERROR: sound '%s' - curve %d attack value detected after attack phasis\n", _name, _sound->nbcurves);
                            goto Error;
                        }
                        break;

                    case 'S':
                    case 's':
                        if (phase > 1)
                        {
                            sprintf (sndYM_g_Error, "#ERROR: sound '%s' - curve %d sustain value detected after sustain phasis\n", _name, _sound->nbcurves);
                            goto Error;
                        }
                        phase = 1;
                        break;

                    case 'D':
                    case 'd':
                    case 'R':
                    case 'r':
                        ASSERT(phase <= 2);
                        phase = 2;
                        break;

                    case '<':
                        phase = 3;
                        break;
                    }

                    if (i == 0)
                    {
                        curve->min = curve->max = len;
                    }
                    else
                    {
                        if (len < curve->min)
                        {
                            curve->min = len;
                        }
                        if (len > curve->max)
                        {
                            curve->max = len;
                        }
                    }

                    switch(phase)
                    {
                    case 0:
                        levels[i++] = len;
                        curve->attacklen++;
                        break;

                    case 1:
                        levels[i++] = len;
                        curve->sustainlen++;
                        break;

                    case 2:
                        levels[i++] = len;
                        curve->decaylen++;
                        break;
                    }

                    if ((len > max) || (len < min))
                    {
                        sprintf (sndYM_g_Error, "#ERROR: sound '%s' - curve %d value out of range %d [%d;%d]\n", _name, _sound->nbcurves, len, min, max); 
                        goto Error;
                    }
                }
                while (phase != 3);

                curve->len = i;
                curve->levels = (s8*) MEM_ALLOC(_allocator, i);
                STDmcpy (curve->levels, levels, i);
            }
            
            _sound->nbcurves++;
        }
    }
    while (end == false);

    return p;

Error:
    return NULL;
}

bool SNDYMloadSounds (MEMallocator* _allocator, char* _filename, SNDYMsoundSet* _soundSet)
{
    FILE* file = fopen(_filename, "rb");

	sndYM_g_Error[0] = 0;

    if (file != NULL)
    {
        u32     filesize;
        char*   buffer;
        char*   p;
        u16     t;


        fseek (file, 0, SEEK_END);
        filesize = ftell(file);
        fseek (file, 0, SEEK_SET);
        buffer = (char*) MEM_ALLOC (_allocator, filesize + 1);
        fread (buffer, 1, filesize, file);
        buffer[filesize] = 0;

        _soundSet->nbSounds = 0;
        p = sndYMfirstToken(buffer);

        while (p != NULL)
        {
            if (strcmp(p, "END") == 0)
            {
                _soundSet->nbSounds++;
            }

            p = sndYMnextToken (p);
        }

        fseek (file, 0, SEEK_SET);
        fread (buffer, 1, filesize, file);
        fclose (file);

        p = strtok(buffer, sndYMdelims);
        printf ("nb sounds: %d\n", _soundSet->nbSounds);

        _soundSet->sounds = (SNDYMsound*)     MEM_ALLOC (_allocator, sizeof(SNDYMsound)     * _soundSet->nbSounds);
        _soundSet->names  = (SNDYMsoundName*) MEM_ALLOC (_allocator, sizeof(SNDYMsoundName) * _soundSet->nbSounds);

        for (t = 0 ; t < _soundSet->nbSounds ; t++)
        {
            DEFAULT_CONSTRUCT (&_soundSet->sounds[t]);
            p = sndYMsoundSet_load (_allocator, &_soundSet->sounds[t], _soundSet->names[t], p);

            if (p == NULL)
            {
				_soundSet->nbSounds = t;
                return false;
            }
        }
    }

    return file != NULL;
}


void SNDYMinitPlayer (MEMallocator* _allocator, SNDYMplayer* _player, SNDYMsoundSet* _soundSet)
{
    u16 t, i;


    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
    {
        _player->commands[t].scorevolume = 15;
    }

    _player->soundSet = _soundSet;
    SND_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, 0xFF, _player->YMregs);

    _player->volumes = (u8*) MEM_ALLOC(_allocator, 16*16);

    for (t = 0 ; t < 16 ; t++)
    {
        for (i = 0 ; i < 16 ; i++)
        {
            _player->volumes[(t << 4) + i] = (u8)((i * t + 15) >> 4);
        }
    }   
}


void SNDYMwriteSounds(SNDYMsoundSet* _soundsSet, FILE* _file)
{
    u16 t;

    STDwriteL(_file, SND_YM_FORMAT);
    STDwriteW(_file, SND_YM_FORMAT_REV);

    STDwriteW(_file, _soundsSet->nbSounds);

    for (t = 0 ; t < _soundsSet->nbSounds ; t++)
    {
        SNDYMsound* sound = &_soundsSet->sounds[t];
        u16 i;

        STDwriteB(_file, sound->nbcurves);
        
        for (i = 0 ; i < sound->nbcurves ; i++)
        {
            SNDYMcurve* curve = &sound->curves[i];

            STDwriteB(_file, curve->curveType);            
            STDwriteB(_file, curve->attacklen);
            STDwriteB(_file, curve->sustainlen);
            STDwriteB(_file, curve->decaylen);
            STDwriteW(_file, curve->tick);
            STDwriteW(_file, curve->len);
            STDwriteB(_file, curve->min);
            STDwriteB(_file, curve->max);

            fwrite(curve->levels, curve->len, 1, _file);
        }
    }
}

#endif

void SNDYMstop(SNDYMplayer* _player)
{
    SND_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, 0xFF, _player->YMregs);
    SND_YM_SET_REG(HW_YM_SEL_LEVELCHA,     0,    _player->YMregs);
    SND_YM_SET_REG(HW_YM_SEL_LEVELCHB,     0,    _player->YMregs);
    SND_YM_SET_REG(HW_YM_SEL_LEVELCHC,     0,    _player->YMregs);

    STDmset (_player->channels, 0, sizeof(_player->channels));
}

u8* SNDYMreadSounds (MEMallocator* _allocator, void* _buffer, SNDYMsoundSet* _soundsSet)
{
    u8* p = (u8*) _buffer;
    u32 format;

    u16 t;


    STD_READ_L(p, format);

    if (format == SND_YM_FORMAT)
    {
        u16 formatrev;

        STD_READ_W(p, formatrev);
        ASSERT (formatrev == SND_YM_FORMAT_REV);

        STD_READ_W(p, _soundsSet->nbSounds);

        _soundsSet->sounds = (SNDYMsound*) MEM_ALLOC (_allocator, sizeof(SNDYMsound) * _soundsSet->nbSounds);

        for (t = 0 ; t < _soundsSet->nbSounds ; t++)
        {
            SNDYMsound* sound = &_soundsSet->sounds[t];
            u16 i;

            DEFAULT_CONSTRUCT (sound);
            STD_READ_B(p, sound->nbcurves);

            for (i = 0 ; i < sound->nbcurves ; i++)
            {
                SNDYMcurve* curve = &sound->curves[i];

                STD_READ_B(p, curve->curveType);            
                STD_READ_B(p, curve->attacklen);
                STD_READ_B(p, curve->sustainlen);
                STD_READ_B(p, curve->decaylen);
                STD_READ_W(p, curve->tick);
                STD_READ_W(p, curve->len);
                STD_READ_B(p, curve->min);
                STD_READ_B(p, curve->max);

                curve->levels = (s8*) MEM_ALLOC (_allocator, curve->len);

                STDmcpy(curve->levels, p, curve->len);
                p += curve->len;
            }
        }

        return p;
    }
    else
    {
        return NULL;
    }
}

void SNDYMfreePlayer(MEMallocator* _allocator, SNDYMplayer* _player)
{
    MEM_FREE(_allocator, _player->volumes);
    _player->volumes = NULL;
}

void SNDYMfreeSounds (MEMallocator* _allocator, SNDYMsoundSet* _soundSet)
{
    u16 t;

    for (t = 0 ; t < _soundSet->nbSounds ; t++)
    {
        u16 i;
        SNDYMsound* sound = &_soundSet->sounds[t];

        for (i = 0 ; i < ARRAYSIZE(sound->curves) ; i++)
        {
            SNDYMcurve* curve = &sound->curves[i];

            if (curve->levels != NULL)
            {
                MEM_FREE(_allocator, curve->levels);
            }
        }           
    }
    
    if (_soundSet->names != NULL)
    {
        MEM_FREE(_allocator, _soundSet->names);
    }

    if (_soundSet->sounds)
    {
        MEM_FREE(_allocator, _soundSet->sounds);
    }
}

#define sndym_PORTA_OFFSET_SHIFT    9
#define sndym_PORTA_OFFSET_LOWMASK  ((1 << sndym_PORTA_OFFSET_SHIFT) - 1)
#define sndym_PORTA_MAX_TRANSPOSE   (1 << (15 - sndym_PORTA_OFFSET_SHIFT))


static void sndYMpitchbend (SNDYMchannel* channel, s16 _keyoffset, u16 _portacount)
{
    ASSERT(_keyoffset >= -sndym_PORTA_MAX_TRANSPOSE);
    ASSERT(_keyoffset <   sndym_PORTA_MAX_TRANSPOSE);

    channel->portaoffset    = -(_keyoffset << sndym_PORTA_OFFSET_SHIFT);
    channel->portainc       = (s16) STDdivs(-channel->portaoffset, _portacount);
    channel->portfreqoffset = 0;
}

#define SNDYM_GETKEYFREQ(KEY,FREQ) {\
    s16 index = KEY;\
    if (index < 0)\
        index = 0;\
    else if (index >= SNDYM_NBKEYS)\
        index = SNDYM_NBKEYS - 1;\
    FREQ = PCENDIANSWAP16(SNDYM_g_keys.w[index]); }


bool SNDYMupdate (SNDYMplayer* _player)
{
    u8   curveindex;
    u8   channelindex;
    u8   mixer;
    u8*  volumestable = _player->volumes;
    CONST SNDYMcommand* command = _player->commands;
	SNDYMchannel* channel = _player->channels;

/*  char temp[256];
    static u32 framenum = 0;

    sprintf(temp, "framenum = %d", framenum++);
    TRAClog(temp, '\n');*/

    *HW_YM_REGSELECT = HW_YM_SEL_IO_AND_MIXER;
    mixer = HW_YM_GET_REG();

    SND_YM_REGS_NEWFRAME(_player->YMregs);

    for (channelindex = 0 ; channelindex < SND_YM_NB_CHANNELS ; channelindex++, channel++, command++)
    {
        u8 soundindex = command->soundindex;
    

		if (command->mute)
		{
			mixer |= 9 << channelindex;
			SND_YM_SET_REG(HW_YM_SEL_LEVELCHA + channelindex, 0, _player->YMregs);
		}
		else if (soundindex < _player->soundSet->nbSounds)
        {
            CONST SNDYMsound* sound = &_player->soundSet->sounds[soundindex];
            u16  nbcurves     = sound->nbcurves;
            bool freqset      = false;
            bool envfreqset   = false;
            bool amplitudeset = false;
            bool running      = false;
            u16  t;
			CONST SNDYMcurve*  curve        = sound->curves;
			SNDYMchannelCurve* channelcurve = channel->curvestate;


            if (command->justpressed)
			{
                EMULtraceClearValue(0);
                EMULtraceClearValue(1);
                EMULtraceClearValue(2);

                /*sprintf(temp, "channel  %d - just pressed", channelindex);
                TRAClog(temp, '\n');*/

				{
					SNDYMchannelCurve* c = channel->curvestate;

					for (t = 0; t < nbcurves; t++, c++)
					{
						c->running = true;
						c->current = 0;
						c->currenttick = 0;
					}
				}

                channel->portaoffset   = 0;
                channel->portainc      = 0;
                channel->portfreqoffset= 0;
                channel->pitchbend     = 0;

                if (channel->portkey != 0)
                {
                    if (command->portamientoticks != 0)
                    {
                        s16 keyoffset  = command->key;  

                        keyoffset -= channel->portkey;

                        sndYMpitchbend (channel, keyoffset, command->portamientoticks);

                        /*sprintf(temp, "channel  %d - portamiento", channelindex);
                        TRAClog(temp, '\n');*/
                    }
                }

                channel->portkey     = command->key;

                channel->freq        = 0;
                channel->finefreq    = 0;

                channel->fineenvfreq = 0;
                channel->fineenvfreq = 0;

                channel->freqsync    = false;
            }


            if ((command->pitchbendrange != 0) && (command->pitchbendticks > 0))
            {
                if (channel->portaoffset == 0) /* no slide currently running => new order */
                {
                    s8 pitchbendprev = channel->pitchbend;

                    channel->pitchbend += command->pitchbendrange;

                    if (channel->pitchbend < -sndym_PORTA_MAX_TRANSPOSE)
                    {
                        channel->pitchbend = pitchbendprev;
                    }
                    else if (channel->pitchbend >= sndym_PORTA_MAX_TRANSPOSE)
                    {
                        channel->pitchbend = pitchbendprev;
                    }
                    else
                    {
                        /*printf ("key = %d - pitchbend = %d - pitchbendprev = %d\n", command->key, channel->pitchbend, channel->pitchbendprev);*/
                        sndYMpitchbend(channel, command->pitchbendrange, command->pitchbendticks);
                    }

                    /*sprintf(temp, "channel  %d - pitchbend - range=%d, ticks=%d", channelindex, command->pitchbendrange, command->pitchbendticks);
                    TRAClog(temp, '\n');*/
                }
            }

            if (command->forcesqrsync)
            {
                channel->freqsync = true;
            }

            for (curveindex = 0 ; curveindex < nbcurves ; curveindex++, curve++, channelcurve++)
            {
                if (channelcurve->running)
                {
                    u8               current      = channelcurve->current;
                    s16              currenttick  = channelcurve->currenttick;
                    bool             isAmpCurve   = curve->curveType == SND_YM_AMPLITUDE; /* Amp curve controls sound lifecycle */


                    running = true;

                    if (--currenttick <= 0)
                    {
                        s8 level = 0;

                        if (curve->levels != NULL)
                            level = curve->levels[current];

                        switch (curve->curveType)
                        {
                        case SND_YM_SQUARESYNC:
                            if (channel->portaoffset == 0)
                                channel->freqsync |= level != 0;
                            break;

                        case SND_YM_AMPLITUDE:
                            channel->env = level == 16;
                            channel->amplitude = level;
                            amplitudeset = true;
                            break;

                        case SND_YM_FREQ:
                            SNDYM_GETKEYFREQ(command->key + level, channel->freq);
                            freqset = true;
                            break;

                        case SND_YM_FIX_FREQ:
                            SNDYM_GETKEYFREQ(level, channel->freq);
                            freqset = true;
                            break;

                        case SND_YM_FINE_FREQ:
                            channel->finefreq = level;
                            freqset = true;
                            break;

                        case SND_YM_MIX:
                            {
                                u8 m1 = 1 << channelindex;
                                u8 m2 = m1 << 3; 

                                channel->square = (level & 1) != 0;
                                channel->noise  = (level & 2) != 0;

                                mixer &= ~m1;
                                mixer &= ~m2;

                                if (channel->square == false)
                                    mixer |= m1;   

                                if (channel->noise == false)
                                    mixer |= m2;   
                            }
                            break;

                        case SND_YM_ENVFREQ:
                            {
                                u16 keyfreq;
                                SNDYM_GETKEYFREQ(command->key + level,keyfreq);

                                keyfreq += 4;
                                keyfreq >>= 3;

								channel->envfreq = keyfreq;
                                envfreqset = true;
                            }
                            break;

                        case SND_YM_FINE_ENVFREQ:
                            channel->fineenvfreq = level;
                            envfreqset = true;
                            break;

                        case SND_YM_ENVSHAPE:
                            SND_YM_SET_REG(HW_YM_SEL_ENVELOPESHAPE, level, _player->YMregs);
                            break;

                        case SND_YM_NOISEFREQ:
                            SND_YM_SET_REG(HW_YM_SEL_FREQNOISE, level, _player->YMregs);
                            break;
                        }

                        currenttick = curve->tick;
                        current++;

                        if (curve->sustainlen > 0)
                        {
                            if (command->pressed)
                            {
                                if ( current >= (curve->attacklen + curve->sustainlen) )
                                {
                                    current = curve->attacklen;
                                }
                            }
                            else if (isAmpCurve == false)
                            {
                                if ((curve->sustainlen > 0) && (curve->decaylen == 0))
                                {
                                    if ( current >= (curve->attacklen + curve->sustainlen) )
                                    {
                                        current = curve->attacklen;
                                    }
                                }
                            }
                        }

                        if ( current >= curve->len )
                        {
                            if (isAmpCurve)
                            {
                                u16 t;
								SNDYMchannelCurve* c = channel->curvestate;

                                current = 0;
                                currenttick = 0;
                                running = false;
                                amplitudeset = true;
                                channel->portkey = 0;

                                for (t = 0 ; t < sound->nbcurves ; t++, c++)
                                {                               
                                    c->running = false;
                                    c->current = 0;
                                    c->currenttick = 0;
                                }                        
                            }

                            channelcurve->running = false;
                            current = 0;
                        }
                    }

                    channelcurve->current     = current;
                    channelcurve->currenttick = currenttick;
                }
            }

            if (running)
            {
                if (channel->portaoffset != 0)
                {
                    u16 keyfreq1, keyfreq2, keyfreqinterpolated;
                    u16 destkeyfreq;
                    u16 portakeyindex;


                    channel->portaoffset += channel->portainc;

                    /*sprintf(temp, "channel  %d - portoffset - %d", channelindex, channel->portaoffset);
                    TRAClog(temp, '\n');*/

                    EMULtracePushValue(2, channel->portaoffset);

                    if ((channel->portainc < 0) && (channel->portaoffset < 0))
                    {
                        channel->portaoffset = 0;
                    }
                    else if ((channel->portainc > 0) && (channel->portaoffset > 0))
                    {
                        channel->portaoffset = 0;
                    }

                    SNDYM_GETKEYFREQ(command->key,destkeyfreq);

                    portakeyindex = command->key + channel->pitchbend + (channel->portaoffset >> sndym_PORTA_OFFSET_SHIFT);

                    /*printf("portakeyindex = %d\n", portakeyindex);*/

                    SNDYM_GETKEYFREQ(portakeyindex,   keyfreq1);
                    SNDYM_GETKEYFREQ(portakeyindex+1, keyfreq2);

                    keyfreqinterpolated = keyfreq1 + (s16)(STDmuls (keyfreq2 - keyfreq1, (channel->portaoffset & sndym_PORTA_OFFSET_LOWMASK )) >> sndym_PORTA_OFFSET_SHIFT);

                    channel->portfreqoffset = keyfreqinterpolated - destkeyfreq;

                    EMULtracePushValue(1, channel->portfreqoffset);

                    freqset    = channel->square;
                    envfreqset = channel->env;
                }

                if (command->scorevolumeset)
                {
                    amplitudeset = true;
                }
            }

            if (amplitudeset)
            {
                if (running == false)
                {
                    SND_YM_SET_REG(HW_YM_SEL_LEVELCHA + channelindex, 0, _player->YMregs);
                }
                else if (channel->env)
                {
                    SND_YM_SET_REG(HW_YM_SEL_LEVELCHA + channelindex, 16, _player->YMregs);
                }
                else
                {
                    SND_YM_SET_REG(HW_YM_SEL_LEVELCHA + channelindex, volumestable[(command->scorevolume << 4) + channel->amplitude], _player->YMregs);
                }
            }

            if (freqset)
            {
                u16 freq = channel->freq + channel->finefreq + channel->portfreqoffset + command->finetune;

                EMULtracePushValue(0, freq);

                /*sprintf(temp, "channel  %d - freqset - %d %d %d %d %d", channelindex, freq, channel->freq, channel->finefreq, channel->portfreqoffset, command->finetune);
                TRAClog(temp, '\n');*/

                if (channel->freqsync)
                {
                    SND_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (channelindex << 1), 0, _player->YMregs);
                    SND_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (channelindex << 1), 0, _player->YMregs);

                    SND_YM_REGS_SQUARE_RESTART(channelindex, _player->YMregs);

                    channel->freqsync = false;
                }

                SND_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (channelindex << 1), freq & 0xFF, _player->YMregs);
                SND_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (channelindex << 1), freq >> 8  , _player->YMregs);
            }

            if (envfreqset)
            {
                u16 envfreq = channel->envfreq + channel->fineenvfreq + (channel->portfreqoffset >> 3) + command->finetune;

                SND_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_L, (u8) envfreq      , _player->YMregs);
                SND_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_H, (u8)(envfreq >> 8), _player->YMregs);
            }
        }
    }

    SND_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, mixer, _player->YMregs);

    return true;
}


u8* SNDYMdrawSoundCurves (SNDYMplayer* _player, bool _active[SND_YM_NB_CHANNELS], u8* pinstr)
{
    static char text[100] = "                   ";
    u8 t;
    SNDYMcommand* command = _player->commands;


    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++, command++)
    {    
        u8 selectedsound = command->soundindex;

        if (_active[t] && (selectedsound < _player->soundSet->nbSounds))
        {
            SNDYMsound*  sound         = &_player->soundSet->sounds[selectedsound];
            u8              nbcurves      = sound->nbcurves;
            u16*            pcurve        = (u16*)(pinstr + 48);
            u8              c;


            STDuxtoa(&text[2],  selectedsound, 2);
            STDuxtoa(&text[5],  command->finetune, 2);
            STDuxtoa(&text[8],  command->portamientoticks, 2);
            STDuxtoa(&text[11], command->scorevolume, 2);
            STDuxtoa(&text[14],_player->channels[t].portaoffset, 4);

            if (_player->soundSet->names != NULL)
            {
                char* soundname = _player->soundSet->names[selectedsound];
                SYSfastPrint(soundname, pinstr + 160 * 9 + 5, 160, 4, (u32)&SYSfont);
            }

            for (c = 0 ; c < nbcurves ; c++)
            {
                u8 current = _player->channels[t].curvestate[c].current;
                SNDYMcurve* curve = &sound->curves[c];
                u8 i;
                u16* p;
                u16 len = curve->len;
                char* name = NULL;
                s8 min = curve->min;
                s8 max = curve->max;
                s8 w = max;
                s8 shift = 0;
                bool running = _player->channels[t].curvestate[c].running; 
                
                if ((len - current) > (SND_YM_PLAYER_NBLINES_INSTR - 12))
                {
                    len = current + (SND_YM_PLAYER_NBLINES_INSTR - 12);
                }

                switch (curve->curveType)
                {
                case SND_YM_AMPLITUDE:      name = "Ampl" ; break;
                case SND_YM_FREQ:           name = "Fq"   ; break;
                case SND_YM_FINE_FREQ:      name = "FFq"  ; break;
                case SND_YM_FIX_FREQ:       name = "FxFq" ; break;
                case SND_YM_MIX:            name = "Mix"  ; break;
                case SND_YM_ENVFREQ:        name = "EFq"  ; break;
                case SND_YM_FINE_ENVFREQ:   name = "EFFq" ; break;
                case SND_YM_ENVSHAPE:       name = "EShp" ; break;
                case SND_YM_NOISEFREQ:      name = "NFq"  ; break;
                case SND_YM_SQUARESYNC:     name = "SSyn" ; break;
                default:                    ASSERT(0);
                }

                if ((min < 0) && (max > 0))
                {
                    if (max < -min)
                    {
                        w = -min;
                    }
                }
                else if (min < 0)
                {
                    w = -min;
                }

                while ((w & 0xF0) != 0)
                {
                    w >>= 1;
                    shift++;
                }

                p = pcurve + 10*80;

                SYSfastPrint(name, pcurve + running, 160, 4, (u32)&SYSfont);

                for (i = current ; i < len ; i++)
                {
                    s8 value = curve->levels[i];


                    if (running)
                    {
                        p++;
                    }

                    if (value < 0)
                    {
                        *p = (1UL << (-value >> shift)) - 1;
                        *p = PCENDIANSWAP16(*p);
                    }

                    p += 2;

                    if (value > 0)
                    {
                        *p = ((s16)0x8000) >> (value >> shift);
                        *p = PCENDIANSWAP16(*p);
                    }

                    p += 77 + (running == false);
                }

                pcurve += 8;
            }
        }
        else
        {
            text[2] = 'X';
            text[3] = 'X';
        }

        SYSfastPrint(text, pinstr, 160, 4, (u32)&SYSfont);

        pinstr += 160*SND_YM_PLAYER_NBLINES_INSTR;
    }

    return pinstr;
}
