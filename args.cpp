#ifndef ARGS_INCLUDED
#define ARGS_INCLUDED

#include "db_cmdarg.cpp"
#include "ctrl.cpp"
#include "unixio.cpp"

struct args_t
{
	cmdarg<char const*> cfg;
	
	args_t( int argc, char** argv ) : 
		cfg( nullptr, "cfg", "cfgfile" )
	{
		for( int argi=1; argi<argc; argi++ )
			ENSURE( cfg.get( argv[argi] ), "unknown argument %*s\n",
			        (uint)sizeof( unix_io_t::device ), argv[argi] );
	}
};


#endif // ARGS_INCLUDED

