#ifndef MAIN_INCLUDED
#define MAIN_INCLUDED

#include "args.cpp"
#include "ehz.cpp"
#include "signals.cpp"


int main( int argc, char** argv )
{
	char buf[256];
	buildheader( buf, lenof(buf), "ehz", "2025-26", false );
	d_fprint( "%S\n", cstr_t( buf ) );

	signals_t::init();
	
	args_t args( argc, argv );
	ehz_t ehz( signals_t::signals, args );
	
	d_fprint_err( "terminated\n" );
    return( 0 );
}

#endif // MAIN_INCLUDED

