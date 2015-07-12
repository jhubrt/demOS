/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of rebirth demo

  rebirth demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  rebirth demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with rebirth demo.  
  If not, see <http://www.gnu.org/licenses/>.
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
