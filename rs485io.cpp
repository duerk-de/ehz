#ifndef RS485IO_INCLUDED
#define RS485IO_INCLUDED

#include "usbttyio.cpp"
#include "request_reader.cpp"
#include "ctrl.cpp"
#include <linux/usbdevice_fs.h>

class rs485_io_t : public usbtty_io_t
{
public:
	uint delay;
	uint error_delay;
	uint timing_print_interval;
	
	uint delay_from_spec() const
	{
		return( usec_t<uint>::sec / baud() * 11 * 7 / 2 );
	}
	
	rs485_io_t( cfgsection_t & cfg, signals_t::data_t const& signals ) :
		usbtty_io_t( cfg, signals )
	{
		selfreload( cfg );
	}
	
	void selfreload( cfgsection_t & cfg )
	{
		timeout = cfg.entry( "timeout", 50000u );
		delay = cfg.entry( "delay", 50000u );
		error_delay = cfg.entry( "error_delay", 250000u );
		timing_print_interval = cfg.entry( "timing_print_interval", 10u );
	}
	
	void handle_error()
	{
		d_fprint_err( "rs485 handle_error @ %S, delay: %3fms\n", name, error_delay );
		teardown();
		if( error_delay )
			sleepus( error_delay );
		setup();
		byte b = 0;
		request_reader_t::write( *this, &b, sizeof( b ) );
		byte c[1024];
		int r = request_reader_t::read( *this, c, sizeof( c ), error_delay );
		(void)r;
	}
};

#endif // RS485IO_INCLUDED

