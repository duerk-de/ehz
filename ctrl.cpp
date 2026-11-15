#ifndef CTRL_INCLUDED
#define CTRL_INCLUDED

#include "db_consoleio.cpp"
#include "backtrace.cpp"
#include <stdlib.h>

#ifdef __x86_64__
	#define ENSURE( expr, ... )              \
		if( !( expr ) )                      \
		{                                    \
			d_fprint_err( __VA_ARGS__ );     \
			bt::print( __FILE__, __LINE__ ); \
			exit( 1 );                       \
		}
#else
	#define ENSURE( expr, ... )          \
		if( !( expr ) )                  \
		{                                \
			d_fprint_err( __VA_ARGS__ ); \
			exit( 1 );                   \
		}
#endif

#endif // CTRL_INCLUDED

