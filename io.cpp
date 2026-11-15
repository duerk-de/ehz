#ifndef IO_INCLUDED
#define IO_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_str.cpp"

struct io_t : public not_copyable_t
{
	cstr_t name;
	uint timeout;
	constexpr io_t( cstr_t _name ) :
		name( _name ),
		timeout( 0 )
	{ }
	virtual void setup() = 0;
	virtual void teardown() = 0;
	virtual uint read( byte * buf, uint maxlen ) = 0;
	virtual void write( byte const* buf, uint len ) = 0;
};

#endif // IO_INCLUDED

