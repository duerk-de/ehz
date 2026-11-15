#ifndef REQUEST_RESPONSE_INCLUDED
#define REQUEST_RESPONSE_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_str.cpp"

struct request_response_t : public not_copyable_t
{
	struct buf_t
	{
		byte * buf;
		uint len;
	};
	cstr_t name;
	constexpr request_response_t( cstr_t _name ) :
		name( _name )
	{ }
	virtual buf_t get_request() = 0;
	virtual buf_t & get_response() = 0;
};

#endif // REQUEST_RESPONSE_INCLUDED

