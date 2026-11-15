#ifndef TIMING_INCLUDED
#define TIMING_INCLUDED

#include "db_defines.cpp"
#include "ctrl.cpp"
#include "linbuf.cpp"

template <bool enabled, uint maxnum>
struct timing_t
{
	struct t_t {
		ulint time;
		cstr_t name;
	};
	linarr_t<t_t, enabled ? maxnum : maxnum> ts;
	
	inline timing_t()
	{
		if( enabled )
			clear();
	}
	inline ~timing_t()
	{
		if( enabled )
			print();
	}
	inline void print()
	{
		if( enabled )
		{
			for( uint i = 1; i < ts.num(); i++ )
				d_fprint( "%2,8Fus: %S\n",
						  ( ( ts[i].time - ts[i-1].time ) + ( 10 - 1 ) ) / 10,
						  ts[i].name );
		}
	}
	inline void clear()
	{
		if( enabled )
		{
			ts.clear();
			add( "" );
		}
	}
	inline void add( cstr_t name )
	{
		if( enabled )
			ts.add( { .time = getnanos(),
					  .name = name } );
	}
	inline void reset( cstr_t name )
	{
		if( enabled )
		{
			add( name );
			print();
			clear();
		}
	}
};


#endif // TIMING_INCLUDED

