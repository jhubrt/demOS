/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2016 J.Hubert

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
------------------------------------------------------------------------------------------------- */

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\SOUND.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\INTERLUD.H"
#include "REBIRTH\SRC\MORPH.H"


#define INTERLUDE_WIDTH			    320
#define INTERLUDE_HEIGHT		    200
#define INTERLUD_MAX_H		    	36
#define INTERLUDE_FB_SIZE		    32000
#define INTERLUDE_MORPHPOINTS	    0x390
#define INTERLUDE_GRADIENT_SPEED    4

#define INTERLUDE_TEXT_ENTRY(NAME)    NAME, ARRAYSIZE(NAME)


static void InterludeInitText1 (void)
{                                         /*|                                        |*/      
    static InterludeText text0[] = {1,  0 , "r e-b i r t h"};
    static InterludeText text1[] = {10, 19,                    "powered by demOS",
                                    19, 19,                    "cyber.savina.net"};

    static InterludeText text2[] = {40, 0 , "Code + Graphix ... Metal Ages"};
    static InterludeText text3[] = {52, 0 , "Photographs ...... Chiappi"               };
    static InterludeText text4[] = {61, 19,                    "chiappi-photo",
                                    70, 25,                          ".jimdo.com"};

    static InterludeText text5[] = {90, 0 , "Musik ............ Anthony Rother"};

    static InterludeText text6[] = {100,19,                    "Dualis from",
                                    109,19,                    "Hacker excellent LP"};
    static InterludeText text7 [] = {130, 0, "Extern code ...... Mr Ni!, TOS-Crew"      };
    static InterludeText text8 [] = {139,12,             "...... Zerkman, Sector One"   };
    static InterludeText text9 [] = {160, 0, "Great advices .... NPomaredes, Hatari"    };
    static InterludeText text10[] = {169,14,               ".... Evil, DHS"             };
    static InterludeText text11[] = {191,24, "cybernetics 2015"                         };


    static InterludeMessage messages[] =
    {
        INTERLUDE_TEXT_ENTRY(text0),
        INTERLUDE_TEXT_ENTRY(text1),
        INTERLUDE_TEXT_ENTRY(text2),
        INTERLUDE_TEXT_ENTRY(text3),
        INTERLUDE_TEXT_ENTRY(text4),
        INTERLUDE_TEXT_ENTRY(text5),
        INTERLUDE_TEXT_ENTRY(text6),
        INTERLUDE_TEXT_ENTRY(text7),
        INTERLUDE_TEXT_ENTRY(text8),
        INTERLUDE_TEXT_ENTRY(text9),
        INTERLUDE_TEXT_ENTRY(text10),
        INTERLUDE_TEXT_ENTRY(text11)
    };

    g_screens.interlude->sequence    = messages;
    g_screens.interlude->sequenceLen = ARRAYSIZE(messages);
}


static void InterludeInitText2 (void)
{                                         /*|                                        |*/      
    static InterludeText text0[] = {1,  0 , "r e-b i r t h"};

    static InterludeText text1[] = {25, 0 , "atari ste 1 mb+ 2 drives",
                                    34, 0 , "tested on ste, Steem 3.2, Hatari"};

    static InterludeText text2[] = {52, 0 , "greetings fly to:"};

    static InterludeText text3[] = {63 , 4, "family",
                                    72, 15, "friends",
                                    81, 25, "colleagues",
                                    90, 4 , "Cybernetics members"};

    static InterludeText text4[] = {104, 0, "Cyclone Delos Jester Niko Nucleus ODC",
                                    113, 0, "Sink Smith Tobe"};

    static InterludeText text5[] = {127, 0, "people from the demoscene"};

    static InterludeText text6[] = {141, 0, "emulators developers",
                                    150, 1, "[in particular Steem Hatari]"};

    static InterludeText text7[] = {164, 0, "the open source community"};

    static InterludeText text8[] = {178, 0, "people from the 8 bits musik scene"};

    static InterludeText text9[] = {192, 0, "everybody creating cool stuffs for fun"};

    static InterludeMessage messages[] = 
    {
        INTERLUDE_TEXT_ENTRY(text0),
        INTERLUDE_TEXT_ENTRY(text1),
        INTERLUDE_TEXT_ENTRY(text2),
        INTERLUDE_TEXT_ENTRY(text3),
        INTERLUDE_TEXT_ENTRY(text4),
        INTERLUDE_TEXT_ENTRY(text5),
        INTERLUDE_TEXT_ENTRY(text6),
        INTERLUDE_TEXT_ENTRY(text7),
        INTERLUDE_TEXT_ENTRY(text8),
        INTERLUDE_TEXT_ENTRY(text9)
    };

    g_screens.interlude->sequence    = messages;
    g_screens.interlude->sequenceLen = ARRAYSIZE(messages);
}


static void InterludeInitRasters (void)
{
    u8* p = (u8*) g_screens.interlude->rasters;
    u16 t;


    *((RASinterupt*) p) = RASvbl2;
    p += sizeof(RASinterupt);

    {
        RASopVbl2* opVbl = (RASopVbl2*) p;

        opVbl->color                  = 0;
        opVbl->backgroundColor        = 0;

        if (g_screens.interlude->sequenceLen > 1)
        {
            opVbl->scanLinesTo1stInterupt = g_screens.interlude->sequence[1].texts[0].y - 1;
        }
        else
        {
            opVbl->scanLinesTo1stInterupt = 255;
        }

        opVbl->nextRasterRoutine = RASmid1;

        g_screens.interlude->rastersColors[0] = &opVbl->color;

        p += sizeof(RASopVbl2);
    }

    {
        u16 seqLen = g_screens.interlude->sequenceLen;

        for (t = 1 ; t < seqLen ; t++)
        {
            RASopMid1* opMid1 = (RASopMid1*) p;

            opMid1->color = RASstopMask;

            if ((t + 1) < seqLen)
            {
                opMid1->scanLineToNextInterupt = g_screens.interlude->sequence[t+1].texts[0].y - g_screens.interlude->sequence[t].texts[0].y;
            }
            else
            {
                opMid1->scanLineToNextInterupt = 255;
            }

            opMid1->nextRasterRoutine = RASmid1;

            g_screens.interlude->rastersColors[t] = &opMid1->color;

            p += sizeof(RASopMid1);
        }
    }
}


static void InterludeInitGradientIndexes (void)
{
    u16 t;

    for (t = 0 ; t < INTERLUDE_MAX_MESSAGES ; t++)
    {
        g_screens.interlude->gradientIndex[t] = (INTERLUDE_GRADIENTLEN << INTERLUDE_GRADIENT_SPEED) - 1;
    }
}


void InterludeEntry (FSM* _fsm)
{
    u16 t;


    IGNORE_PARAM(_fsm);

    g_screens.interlude = MEM_ALLOC_STRUCT( &sys.allocatorMem, Interlude );
    DEFAULT_CONSTRUCT(g_screens.interlude);

    if (g_screens.interludeTextIndex == 0)
    {
        InterludeInitText1();
    }
    else
    {
        InterludeInitText2();
    }

    g_screens.interludeTextIndex++;

    STDmset(HW_COLOR_LUT, 0, 32);

    g_screens.interlude->framebuffers[0] = (u8*) RINGallocatorAlloc ( &sys.mem, 64000UL);
    g_screens.interlude->framebuffers[1] = g_screens.interlude->framebuffers[0] + 32000;    
    g_screens.interlude->printbuffer     = (u16*)  RINGallocatorAlloc ( &sys.mem, INTERLUD_MAX_H * sizeof(u16) * 20);
    g_screens.interlude->morphcode       = (u8*)   RINGallocatorAlloc ( &sys.mem, SLIgetMorphCodeLen(INTERLUDE_MORPHPOINTS));
    g_screens.interlude->horitable       = (u32*)  RINGallocatorAlloc ( &sys.mem, INTERLUDE_WIDTH  * sizeof(u32));
    g_screens.interlude->verttable       = (u16*)  RINGallocatorAlloc ( &sys.mem, INTERLUDE_HEIGHT * sizeof(u16));
    g_screens.interlude->startpos        = (u16*)  RINGallocatorAlloc ( &sys.mem, INTERLUDE_MORPHPOINTS * sizeof(u16) * 2);
    g_screens.interlude->endpos          = (u16*)  RINGallocatorAlloc ( &sys.mem, INTERLUDE_MORPHPOINTS * sizeof(u16) * 2);
    g_screens.interlude->pos             = (u16*)  RINGallocatorAlloc ( &sys.mem, INTERLUDE_MORPHPOINTS * sizeof(u16) * 3);
    g_screens.interlude->rasters         = (u8*)   RINGallocatorAlloc ( &sys.mem, INTERLUDE_MAX_MESSAGES * sizeof(RASopMid1) + sizeof(RASopVbl2) + sizeof(RASinterupt) );

    g_screens.interlude->index = -1;

    /* init random background */
    {
        u16* p = (u16*) g_screens.interlude->framebuffers[0];
        u16 p0, p1, p2, v7;

        for (t = 0 ; t < 2000 ; t += 8)
        {
            *p++ = 0;

            p0 = STDifrnd();
            p1 = STDifrnd();
            p2 = STDifrnd();

            v7 = p0 & p1 & p2;   /* detect pixels with color = 14 */

            p0 &= ~v7;           /* masks low bit plane with this mask to transform pixels with color = 14 into pixels with color = 12 */

            *p++ = p0;
            *p++ = p1;
            *p++ = p2;
        }

        for (t = 2000 ; t < 32000 ; t += 2000)
        {
            STDmcpy (g_screens.interlude->framebuffers[0] + t, g_screens.interlude->framebuffers[0], 2000);
        }
    }

    InterludeInitGradientIndexes();

    InterludeInitRasters ();

    SLIinitMorph (g_screens.interlude->verttable, g_screens.interlude->horitable, INTERLUDE_HEIGHT, INTERLUDE_WIDTH >> 1, INTERLUDE_WIDTH);

    SLIgenerateMorphCode(g_screens.interlude->morphcode, INTERLUDE_MORPHPOINTS);

    RASsetColReg (0x825C);

    RASnextOpList = g_screens.interlude->rasters;

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);
}


void InterludeActivity (FSM* _fsm)
{
    register s16*  i = g_screens.interlude->gradientIndex;
    register u16** p = g_screens.interlude->rastersColors;
    register s16   t;

  
    s16 nb = g_screens.interlude->index;

    IGNORE_PARAM(_fsm);

    for (t = 0 ; t <= nb ; t++)
    {
        (*i)--;

        if ((*i) < 0)
        {
            (*i) = 0;
        }

        **p = g_screens.interlude->gradient[(*i) >> INTERLUDE_GRADIENT_SPEED];

        p++;
        i++;
    }

    if (g_screens.interlude->animationStep > 0)
    {
        u8* framebuffer = g_screens.interlude->framebuffers[g_screens.interlude->animationStep & 1];

        SYSwriteVideoBase((u32) framebuffer);

        SLImorphFunc (
            framebuffer, 
            g_screens.interlude->pos, 
            (u32) g_screens.interlude->verttable, 
            (u32) g_screens.interlude->horitable, 
            (u32) g_screens.interlude->morphcode );

        g_screens.interlude->animationStep--;
    }
}


static void InterludeInitMorphPointsFromText(u16* _points, void* _printbuffer, InterludeMessage* _message)
{
    u16 t;
    u16 ybase = _message->texts[0].y;

    STDmset (_printbuffer, 0, INTERLUD_MAX_H * sizeof(u16) * 20);

    for (t = 0 ; t < _message->nbTexts ; t++)
    {
        u16 dy = _message->texts[t].y - ybase;

        ASSERT( dy < (INTERLUD_MAX_H - 8) );
        SYSfastPrint (_message->texts[t].text, ((u8*)_printbuffer) + (dy * 40) + _message->texts[t].col, 40, 2);
    }

    {
        u16* ps = (u16*) _printbuffer;
        u16* pd = _points;
        u16 x, y, nb = 0;

        for (y = 0 ; y < INTERLUD_MAX_H ; y++)
        {
            for (x = 0 ; x < 320 ; )
            {
                u16 val = *ps++;
                u16 i;

                for (i = 16 ; i > 0 ; i--)
                {
                    if ( val & 0x8000 )
                    {
                        if (nb >= INTERLUDE_MORPHPOINTS)
                        {
                            break;
                        }

                        *pd++ = x;
                        *pd++ = y + ybase;
                        nb++;
                    }

                    val <<= 1;
                    x++;
                }
            }
        }

        y  = 0;
        ps = _points;

        for (x = nb ; x < INTERLUDE_MORPHPOINTS ; x++)
        {
            *pd++ = *ps++;
            *pd++ = *ps++;

            y++;

            if ( y >= nb )
            {
                y = 0;
                ps = _points;
            }
        }

        /* debug display of number of points in current sentence */
        /*   { 
            static char line[] = "    ";

            STDuxtoa(line, nb, 4);

            for (x = 0 ; x < 8 ; x += 2)
            {
                SYSdebugPrint (g_screens.interludeFramebuffers[0] + x, 160, SYS_4P_BITSHIFT, 36, 190, line);
                SYSdebugPrint (g_screens.interludeFramebuffers[1] + x, 160, SYS_4P_BITSHIFT, 36, 190, line);
            }
        }*/
    }
}

static void InterludeInitMorphPointsRandom (u16* _dest, u16 _offx, u16 _mask, u16 _y)
{
    u16 t;

    for (t = 0 ; t < INTERLUDE_MORPHPOINTS ; t++)
    {
        *_dest++ = (STDifrnd() & _mask) + _offx;
        *_dest++ = _y;
    }
}


static void InterludeUpdate4Planes (InterludeMessage* _msg)
{
    u16* p  = (u16*) g_screens.interlude->printbuffer;

    u16 ymin =  _msg->texts[0].y;
    u16 ymax =  8 + _msg->texts[_msg->nbTexts - 1].y;
    u16 y;

    u16* d1 = (u16*) (g_screens.interlude->framebuffers[0] + (ymin * 160));


    STDmcpy(g_screens.interlude->framebuffers[0], g_screens.interlude->framebuffers[1], 32000);

    ymax -= ymin;
    ymax *= 10;

    SYSvsync;

    for (y = ymax ; y > 0 ; y--)
    {
        u16 v = *p++;

        d1++;
        *d1++ |= v;
        *d1++ |= v;
        *d1++ |= v;

        v = *p++;

        d1++;
        *d1++ |= v;
        *d1++ |= v;
        *d1++ |= v;
    }

    SYSwriteVideoBase((u32) g_screens.interlude->framebuffers[0]);
    SYSvsync;
    STDmcpy(g_screens.interlude->framebuffers[1], g_screens.interlude->framebuffers[0], 32000);
}

static u16 interludeGrad[] = 
{
    /*0 */  0,
    /*1 */  0x080,
    /*2 */  0x010,
    /*3 */  0x018,
    /*4 */  0x098,
    /*5 */  0x898,
    /*6 */  0x828,
    /*7 */  0x821,
    /*8 */  0x8A1,
    /*9 */  0x831,
    /*10*/  0x131,
    /*11*/  0x1B1,
    /*12*/  0x1B9,
    /*13*/  0x149,
    /*14*/  0x1C9,
    /*15*/  0x1C2,
    /*16*/  0x152,
    /*17*/  0x952,
    /*18*/  0x9D2,
    /*19*/  0x962,
    /*20*/  0x96A,
    /*21*/  0x9EA,
    /*22*/  0x97A,
    /*23*/  0x973,
    /*24*/  0x9F3,
    /*25*/  0x2FB 
};


void InterludeBacktask (FSM* _fsm)
{
    u16 t;

    SYSwriteVideoBase((u32) g_screens.interlude->framebuffers[1]);
    SYSvsync;

    STDmcpy(g_screens.interlude->framebuffers[1], g_screens.interlude->framebuffers[0], INTERLUDE_FB_SIZE);

    {
            ASSERT (sizeof(g_screens.interlude->gradient) == sizeof(interludeGrad));

            STDmcpy (g_screens.interlude->gradient, interludeGrad, sizeof(interludeGrad));

            {
                u16 temp[20];

                COLcomputeGradient (&interludeGrad[0], &interludeGrad[INTERLUDE_GRADIENTLEN - 1], 1, ARRAYSIZE(temp), temp);

                for (t = 1 ; t < 14 ; t += 2)
                {
                    HW_COLOR_LUT[t] = temp[t];
                }
            }

            HW_COLOR_LUT[7] = HW_COLOR_LUT[15] = interludeGrad[INTERLUDE_GRADIENTLEN - 1];
    }

    for (t = 0 ; t < INTERLUDE_GRADIENTLEN ; t++)
    {
        g_screens.interlude->gradient[t] |= RASstopMask;
    }

    for (t = 0 ; t < g_screens.interlude->sequenceLen ; t++)
    {
        *g_screens.interlude->rastersColors[t] = g_screens.interlude->gradient[INTERLUDE_GRADIENTLEN - 1];
    }

    do
    {
        {
            InterludeMessage* msgend = NULL;

            if ( g_screens.interlude->index < 0 )
            {
                InterludeInitMorphPointsRandom (g_screens.interlude->startpos, 0, 255, 0);
            }
            else
            {
                InterludeMessage* msgstart = &g_screens.interlude->sequence[g_screens.interlude->index];
                InterludeInitMorphPointsFromText (g_screens.interlude->startpos, g_screens.interlude->printbuffer, msgstart);
            }            

            if ( (g_screens.interlude->index + 1) >= g_screens.interlude->sequenceLen )
            {
                InterludeInitMorphPointsRandom (g_screens.interlude->endpos, 0, 127, 199);
            }
            else
            {
                msgend = &g_screens.interlude->sequence[g_screens.interlude->index + 1];
                InterludeInitMorphPointsFromText (g_screens.interlude->endpos, g_screens.interlude->printbuffer, msgend);
            }

            SLIstartMorph (g_screens.interlude->startpos, g_screens.interlude->endpos, (u32) g_screens.interlude->morphcode, (u32) g_screens.interlude->pos, INTERLUDE_MORPHPOINTS);

            SLIdisplayMorph(
                g_screens.interlude->framebuffers[1], 
                g_screens.interlude->pos, 
                (u32) g_screens.interlude->verttable, 
                (u32) g_screens.interlude->horitable, 
                INTERLUDE_MORPHPOINTS );

            g_screens.interlude->animationStep = SLI_MORPHFRAMES;

            while (g_screens.interlude->animationStep != 0);

            if (msgend != NULL)
            {
                InterludeUpdate4Planes(msgend);
            }
            else
            {
                u16* p = (u16*)(g_screens.interlude->framebuffers[0] + 32000 - 160);

                for (t = 0 ; t < 20 ; t++)
                {
                    p[16000] = 0;
                    *p = 0;
                    p += 4;
                }
            }

            for (t = 0 ; t < 30 ; t++)
            {
                SYSvsync;
            }

            g_screens.interlude->index++;

            if (g_screens.interlude->index >= g_screens.interlude->sequenceLen)
            {
                if ( FSMisLastState(_fsm) == false )
                {
                    g_screens.interlude->index--;
                    break;
                }

                InterludeInitGradientIndexes ();

                g_screens.interlude->index = -1;
            }
        }
    }
    while (1);
    
    for (t = 0 ; t < 75 ; t++)
    {
        u16 i;

        SYSvsync;
     
        for (i = 0 ; i < 3 ; i++)
        {
            InterludeActivity (NULL);
        }
    }

    RASnextOpList = NULL;

    snd.playerClientStep++;

    FSMgotoNextState (&g_stateMachine);
    ScreenWaitMainDonothing();

    IGNORE_PARAM(_fsm);
}


void InterludeExit (FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    RINGallocatorFree ( &sys.mem, g_screens.interlude->framebuffers[0]);
    RINGallocatorFree ( &sys.mem, g_screens.interlude->rasters);
    RINGallocatorFree ( &sys.mem, g_screens.interlude->printbuffer);
    RINGallocatorFree ( &sys.mem, g_screens.interlude->morphcode );
    RINGallocatorFree ( &sys.mem, g_screens.interlude->horitable );
    RINGallocatorFree ( &sys.mem, g_screens.interlude->verttable );
    RINGallocatorFree ( &sys.mem, g_screens.interlude->startpos );
    RINGallocatorFree ( &sys.mem, g_screens.interlude->endpos );
    RINGallocatorFree ( &sys.mem, g_screens.interlude->pos );

    MEM_FREE( &sys.allocatorMem, g_screens.interlude );
    g_screens.interlude = NULL;

    FSMgotoNextState (&g_stateMachineIdle);
}

