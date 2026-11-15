#ifndef SIGNALS_INCLUDED
#define SIGNALS_INCLUDED

#include "ctrl.cpp"
#include <signal.h>

namespace signals_t
{
	struct data_t
	{
		bool term;
		bool hup;
		bool usr1;
		bool usr2;
	};

	data_t signals = {
		.term = false,
		.hup = false,
		.usr1 = false,
		.usr2 = false
	};

	void handler( int signal )
	{
		switch( signal )
		{
			case SIGTERM:
				d_fprint( "sigterm received\n" );
				signals.term = true;
				break;
			case SIGHUP:
				d_fprint( "sighup received\n" );
				signals.hup = true;
				break;
			case SIGUSR1:
				d_fprint( "sigusr1 received\n" );
				signals.usr1 = true;
				break;
			case SIGUSR2:
				d_fprint( "sigusr2 received\n" );
				signals.usr2 = true;
				break;
			default:
				d_fprint_err( "unexpected signal: 0x%1,1,16u\n", signal );
				break;
		}
	}

	void install_handler( int signal, void (*handler)(int) )
	{
		struct sigaction sa;
		sa.sa_handler = handler;
		sigemptyset( &sa.sa_mask );
		sa.sa_flags = 0;
		sigaction( signal, &sa, NULL );
	}

	void init()
	{
		install_handler( SIGTERM, handler );
		install_handler( SIGHUP, handler );
		install_handler( SIGUSR1, handler );
		install_handler( SIGUSR2, handler );
	}
};

#endif // SIGNALS_INCLUDED

