/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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

#ifndef POLYZOOM_H
#define POLYZOOM_H

#include "DEMOSDK\FSM.H"

void CybervectorEntry		(FSM* _fsm);
void CybervectorActivity	(FSM* _fsm);
void CybervectorExit		(FSM* _fsm);
void CybervectorBacktask	(FSM* _fsm);

STRUCT(Cybervector)
{
	u16*	polygonsList;
	u8*		polygonsData;
	void*	framebuffers[3];
	u16		flipbuffer;

	u16*	rastersop[2];
	u16		fliprasterop;

	u16*	displist;
	u16*    displistp;

	u16*	sin;
	u16*	cos;
	u16		nbFrames;
	u16	    currentFrame;
	u16*	pal;
};

#endif
