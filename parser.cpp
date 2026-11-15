#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"

struct syncer_t
{
	virtual bool sync( uint * buf, uint & size ) = 0;
	virtual bool findend( uint const* buf, uint size, uint & endpos ) = 0;
	void shift( uint * buf, uint & size, uint endpos )
	{
		cpy( (byte *)buf, (byte *)( buf + endpos ), size - endpos * sizeof(uint) );
		size -= endpos * sizeof(uint);
	}
};

struct parser_t
{
	virtual void setup() = 0;
	virtual bool teardown( ulint timestamp ) = 0;
	virtual bool parse( byte const* file, uint filelen, bool maybe_truncated ) = 0;
	bool run( uint const* buf, uint size, ulint timestamp, bool maybe_truncated )
	{
		setup();
		bool okay = parse( (byte const*)buf, size * sizeof(uint), maybe_truncated );
		okay &= teardown( timestamp );
		return( okay );
	}
};

#endif // PARSER_INCLUDED

