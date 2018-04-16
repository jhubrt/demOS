/*-----------------------------------------------------------------------------------------------
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
-------------------------------------------------------------------------------------------------*/

#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\STANDARD.H"

#ifdef DEMOS_DEBUG
#	define RINGALLOCATOR_DEBUG
#endif

#ifdef RINGALLOCATOR_DEBUG
#	define RINGALLOCATOR_GUARD1		0x600D8EADUL
#	define RINGALLOCATOR_GUARD2		0x600DF007UL
#endif

STRUCT(AllocCell)
{
#	ifdef RINGALLOCATOR_DEBUG
	u32 		guard1;
#	endif

	struct AllocCell_* next;
	struct AllocCell_* prev;

#	ifdef RINGALLOCATOR_DEBUG
	u32 		guard2;
#	endif
};

/* use bit 1 of prev pointer as a boolean to indicate if cell is free */
#define ALLOCCELL_isFree(_cell)		(((u32)_cell->prev) & 1)
#define ALLOCCELL_deallocate(_cell)	((*(u32*)&_cell->prev) |= 1)
#define ALLOCCELL_getPrev(_cell)	((AllocCell*)(((u32)_cell->prev) & 0xFFFFFFFEUL))


void RINGallocatorReset (RINGallocator* _m)
{
    /* relocate all ring pointers on buffer initial state and set a free cell at the begining of the buffer */
	_m->head = _m->tail = _m->buffer;
    _m->last = NULL;
}


void RINGallocatorInit (RINGallocator* _m, void* _buffer, u32 _size)
{
	_m->buffer	  = (u8*) _buffer;
	_m->size		  = (_size + 3) & 0xFFFFFFFCUL;
	_m->bufferEnd = _m->buffer + _m->size;
	
	RINGallocatorReset (_m);
}


void* RINGallocatorAlloc (RINGallocator* _m, u32 _size)
{
	u32 size = sizeof(AllocCell) + ((_size + 3UL) & 0xFFFFFFFCUL);
    AllocCell* cell;


	if ( _m->last == NULL )
	{
		if ( size <= _m->size )
		{
	        cell = (AllocCell*) _m->head;
		    
            cell->prev = cell->next = NULL;
			
            _m->tail += size;
            _m->last = _m->head;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		u8* p = _m->tail;

        cell = (AllocCell*) _m->last;

#		ifdef RINGALLOCATOR_DEBUG
		ASSERT(cell->guard1 == RINGALLOCATOR_GUARD1);
		ASSERT(cell->guard2 == RINGALLOCATOR_GUARD2);
#		endif

		if ( p >= _m->head )
		{
			if ( (p + size) <= _m->bufferEnd )
			{
				p += size;
			}
			else if (( _m->buffer + size ) <= _m->head )
			{
				_m->tail = _m->buffer;
				p = _m->buffer + size;
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			if (( p + size ) <= _m->head )
			{
				p += size;
			}
			else
			{
				return NULL;
			}
		}

		{
			AllocCell* cellNew = (AllocCell*) _m->tail;

			cell->next = cellNew;
			cellNew->prev = cell;
			cellNew->next = NULL;

			_m->last = (u8*) cellNew;
			_m->tail = p;
			cell = cellNew;
		}
	}

#	ifdef RINGALLOCATOR_DEBUG
	cell->guard1 = RINGALLOCATOR_GUARD1;
	cell->guard2 = RINGALLOCATOR_GUARD2;
#	endif

	return cell + 1;
}


void* RINGallocatorAllocTemp (RINGallocator* _m, u32 _size)
{
	u32 size = sizeof(AllocCell) + ((_size + 3UL) & 0xFFFFFFFCUL);
    AllocCell* cell;


	if ( _m->last == NULL )
	{
		if ( size <= _m->size )
		{
	        cell = (AllocCell*) _m->head;
		    
            cell->prev = cell->next = NULL;
			
            _m->tail += size;
            _m->last = _m->head;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		u8* p = _m->head;

        cell = (AllocCell*) _m->head;

#		ifdef RINGALLOCATOR_DEBUG
		ASSERT(cell->guard1 == RINGALLOCATOR_GUARD1);
		ASSERT(cell->guard2 == RINGALLOCATOR_GUARD2);
#		endif

        if ( p <= _m->tail )
		{
            if ( (_m->buffer + size) <= p )
            {
                p -= size;
            }
            else if ( (_m->tail + size) <= _m->bufferEnd )
			{
				p = _m->bufferEnd - size;
			}
			else
			{
				return NULL;
			}
		}
		else
		{
            if (( _m->tail + size ) <= _m->head )
			{
				p -= size;
			}
			else
			{
				return NULL;
			}
		}

		{
			AllocCell* cellNew = (AllocCell*) p;
            
            ASSERT(ALLOCCELL_isFree(cell) == false);
			cell->prev = cellNew;
			cellNew->prev = NULL;
			cellNew->next = cell;

			_m->head = p;
			cell = cellNew;
		}
	}

#	ifdef RINGALLOCATOR_DEBUG
	cell->guard1 = RINGALLOCATOR_GUARD1;
	cell->guard2 = RINGALLOCATOR_GUARD2;
#	endif

	return cell + 1;
}


void RINGallocatorFree (RINGallocator* _m, void* _address)
{
	AllocCell* cell = ((AllocCell*) _address) - 1;


#	ifdef RINGALLOCATOR_DEBUG
	ASSERT(cell->guard1 == RINGALLOCATOR_GUARD1);
	ASSERT(cell->guard2 == RINGALLOCATOR_GUARD2);
#	endif

	ASSERT( ALLOCCELL_isFree(cell) == false );

	ALLOCCELL_deallocate (cell);

	if ( ALLOCCELL_getPrev(cell) == NULL ) /* If no cell before, wipe all the free cells after */
	{
		AllocCell* c = cell;

		do
		{
			c = c->next;

			if ( c != NULL )
			{
				_m->head = (u8*) c;

				if ( ALLOCCELL_isFree(c) == false )
				{
					c->prev = NULL;
					break;
				}
			}
			else
			{
				RINGallocatorReset (_m);
				break;
			}
		}
		while (1);
	}
	else if ( cell->next == NULL ) /* If no cell after, wipe all the free cells before */
	{
		AllocCell* c = cell;

		do
		{
     		_m->tail = (u8*) c;
			c = ALLOCCELL_getPrev(c);

			if ( c != NULL )
			{
				if ( ALLOCCELL_isFree(c) == false )
				{
    				c->next = NULL;
                    _m->last = (u8*) c;
					break;
				}
			}
			else
			{
				RINGallocatorReset (_m);
				break;
			}
		}
		while (1);
	}
}

bool RINGallocatorIsEmpty (RINGallocator* _m)
{
    return _m->last == NULL;
}

void RINGallocatorFreeSize(RINGallocator* _m, RINGallocatorFreeArea* _info)
{
    _info->nbareas = 0;
    _info->size    = 0;

    if (_m->tail > _m->head)
    {
        if (_m->head > _m->buffer )
        {
            _info->areasizes[_info->nbareas++] = _m->head - _m->buffer;
        }

        if (_m->tail < _m->bufferEnd )
        {
            _info->areasizes[_info->nbareas++] = _m->bufferEnd - _m->tail;
        }
    }
    else
    {
        if (_m->tail > _m->buffer )
        {
            _info->areasizes[_info->nbareas++] = _m->tail - _m->buffer;
        }

        if (_m->head < _m->bufferEnd )
        {
            _info->areasizes[_info->nbareas++] = _m->bufferEnd - _m->head;
        }
    }
    
    {
        u16 i;
    
        for (i = 0 ; i < _info->nbareas ; i++)
        {
            _info->size += _info->areasizes[i];

        }
    }
}

#ifdef DEMOS_DEBUG

void RINGallocatorDump (RINGallocator* _m, FILE* _output)
{
	AllocCell* cell = (AllocCell*) _m->head;
    u32 nbcells = 0;
    u32 nbfreecells = 0;
    char line[78];
    s32  dsize = ARRAYSIZE(line) - 1;
    char occupied = 'X';
    u32 totalallocated = 0;


    STDmset(line, 0x20202020UL, dsize);
    line[dsize] = 0;

	fprintf (_output, "Allocator %p\n", _m);
	fprintf (_output, "buf: %-8p size: %-8ld head: %-8p tail: %p last: %p\n\n", _m->buffer, _m->size, _m->head, _m->tail, _m->last);

    if (_m->last != NULL)
    {
        do
        {
            s32  size;
            bool endofbuffer = false;
            u8*  cellend;


            if ( cell->next != NULL )
            {
                cellend = (u8*) cell->next;
            }
            else 
            {
                ASSERT( ALLOCCELL_isFree(cell) == false);
                cellend = _m->tail;
            }

            if ( cellend < (u8*) cell )
            {
                endofbuffer = true;
                size = ((u32)_m->bufferEnd) - ((u32)cell);
            }
            else
            {
                size = ((u32)cellend) - ((u32)cell);
            }

            if (ALLOCCELL_isFree(cell))
            {
                nbfreecells ++;
            }
            else
            {
                totalallocated += size;
            }

            size -= sizeof(AllocCell);

            nbcells++;

            fprintf (_output, "%-8p %s prev: %-8p next: %-8p user: %p rsiz: %ld %c\n", 
                cell, 
                ALLOCCELL_isFree(cell) ? "[ ]" : "[X]", 
                cell->prev, 
                cell->next, 
                cell + 1,
                size,
                endofbuffer ? '+' : ' ');

            {
                s16 start = (s16)(((u8*)cell - _m->buffer) * dsize / _m->size);
                s16 end   = (s16)(start + size * dsize / _m->size);
                s16 j;
                char cr = ALLOCCELL_isFree(cell) ? '_' : occupied;
            
                ASSERT(end < dsize);
                for (j = start ; j <= end ; j++)
                {
                    line[j] = cr;
                }

                occupied ^= 'X' ^ 'x';
            }

            cell = cell->next;
        }
        while (cell != NULL);
    }

    line [(_m->head - _m->buffer) * dsize / _m->size] = '[';
    line [(_m->tail - _m->buffer) * dsize / _m->size] = ']';
    fprintf (_output, "\n%s\n", line);

    if (_m->last != NULL)
    {
        STDmset(line, 0x20202020UL, dsize);
        line [(_m->last - _m->buffer) * dsize / _m->size] = '^';
        fprintf (_output, "%s\n", line);
    }

	fprintf (_output, "cells: %lu - allocs: %lu - freecells: %lu - total allocated bytes: %lu\n\n", nbcells, nbcells - nbfreecells, nbfreecells, totalallocated);

    RINGallocatorCheck(_m);    
}


u32 RINGallocatorList (RINGallocator* _m, void** _list, u32 _maxSize)
{
	u32 nb = 0;

    if (_m->last != NULL)
    {
        AllocCell* cell = (AllocCell*) _m->head;

        do
        {
            if ( ALLOCCELL_isFree (cell) == false )
            {
                if ( nb < _maxSize )
                {
                    *_list++ = cell + 1;
                }

                nb++;
            }

            cell = cell->next;
        }
        while (cell != NULL);
    }

	return nb;
}


u32 RINGallocatorCheck (RINGallocator* _m)
{
    u32 nbchecks = 0;

    if (_m->last != NULL)
    {
        AllocCell* cell = (AllocCell*) _m->head;

        do
        {
            ASSERT(cell->guard1 == RINGALLOCATOR_GUARD1);
            ASSERT(cell->guard2 == RINGALLOCATOR_GUARD2);
            nbchecks++;

            cell = cell->next;
        }
        while (cell != NULL);
    }

    return nbchecks;
}


#endif


#ifdef DEMOS_UNITTEST

void RINGallocatorUnitTest (void)
{
	RINGallocator allocator;
	void* adr[16];
    void* adrtemp[16];
	void* alloc[16];

    u32 size = 128UL*1024UL;

	void* buffer = malloc(size);

	RINGallocatorInit (&allocator, buffer, size);

	adr[0] = RINGallocatorAlloc (&allocator, 65536UL);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 1 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[0] == alloc[0] );

    adrtemp[0] = RINGallocatorAllocTemp (&allocator, 8192);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	RINGallocatorDump (&allocator, stdout);

    RINGallocatorFree (&allocator, adrtemp[0]);
	ASSERT( 1 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	RINGallocatorDump (&allocator, stdout);

    adrtemp[0] = RINGallocatorAllocTemp (&allocator, 8192);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	RINGallocatorDump (&allocator, stdout);

    adrtemp[1] = RINGallocatorAllocTemp (&allocator, 8192);
	ASSERT( 3 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	RINGallocatorDump (&allocator, stdout);

    RINGallocatorFree (&allocator, adrtemp[0]);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	RINGallocatorDump (&allocator, stdout);

    RINGallocatorFree (&allocator, adrtemp[1]);
	ASSERT( 1 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	RINGallocatorDump (&allocator, stdout);

	adr[1] = RINGallocatorAlloc (&allocator, 16384UL);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[0] == alloc[0] );
	ASSERT( adr[1] == alloc[1] );

	adr[2] = RINGallocatorAlloc (&allocator, 8191UL);
	adr[3] = RINGallocatorAlloc (&allocator, 16383UL);
    adr[4] = RINGallocatorAlloc (&allocator, 4096UL);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 5 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[0] == alloc[0] );
	ASSERT( adr[1] == alloc[1] );
	ASSERT( adr[2] == alloc[2] );
	ASSERT( adr[3] == alloc[3] );
	ASSERT( adr[4] == alloc[4] );

	RINGallocatorFree (&allocator, adr[2]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT(4 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[0] == alloc[0] );
	ASSERT( adr[1] == alloc[1] );
	ASSERT( adr[3] == alloc[2] );
	ASSERT( adr[4] == alloc[3] );

	RINGallocatorFree (&allocator, adr[1]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 3 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[0] == alloc[0] );
	ASSERT( adr[3] == alloc[1] );
	ASSERT( adr[4] == alloc[2] );

    RINGallocatorFree (&allocator, adr[3]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[0] == alloc[0] );
	ASSERT( adr[4] == alloc[1] );

    RINGallocatorFree (&allocator, adr[0]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 1 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[4] == alloc[0] );

	adr[5] = RINGallocatorAlloc (&allocator, 67000UL);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[4] == alloc[0] );
	ASSERT( adr[5] == alloc[1] );

    RINGallocatorFree (&allocator, adr[5]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 1 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[4] == alloc[0] );

	adr[5] = RINGallocatorAlloc (&allocator, 10000UL);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 2 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[4] == alloc[0] );
	ASSERT( adr[5] == alloc[1] );

    RINGallocatorFree (&allocator, adr[5]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 1 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));
	ASSERT( adr[4] == alloc[0] );

    RINGallocatorFree (&allocator, adr[4]);
	RINGallocatorDump (&allocator, stdout);
	ASSERT( 0 == RINGallocatorList (&allocator, alloc, ARRAYSIZE(alloc) ));

    free(buffer);
}

#endif
