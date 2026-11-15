#ifndef USBTTYIO_INCLUDED
#define USBTTYIO_INCLUDED

#include "unixio.cpp"
#include "signals.cpp"
#include "cfg.cpp"
#include "ctrl.cpp"
#include "db_file.cpp"
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/serial.h>

class usbtty_io_t : public unix_io_t
{
protected:
	struct baud_t
	{
		uint baud;
		speed_t speed;
	};

	static constexpr baud_t bauds[] = { {       0,       B0 },
	                                    {      50,      B50 },
	                                    {      75,      B75 },
	                                    {     110,     B110 },
	                                    {     134,     B134 },
	                                    {     150,     B150 },
	                                    {     200,     B200 },
	                                    {     300,     B300 },
	                                    {     600,     B600 },
	                                    {    1200,    B1200 },
	                                    {    1800,    B1800 },
	                                    {    2400,    B2400 },
	                                    {    4800,    B4800 },
	                                    {    9600,    B9600 },
	                                    {   19200,   B19200 },
	                                    {   38400,   B38400 },
	                                    {   57600,   B57600 },
	                                    {  115200,  B115200 },
	                                    {  230400,  B230400 },
	                                    {  460800,  B460800 },
	                                    {  500000,  B500000 },
	                                    {  576000,  B576000 },
	                                    {  921600,  B921600 },
	                                    { 1000000, B1000000 },
	                                    { 1152000, B1152000 },
	                                    { 1500000, B1500000 },
	                                    { 2000000, B2000000 },
	                                    { 2500000, B2500000 },
	                                    { 3000000, B3000000 },
	                                    { 3500000, B3500000 },
	                                    { 4000000, B4000000 } };
	
	speed_t speed;
	bool set_latency;
	bool low_latency;
	
	template <typename t>
	static inline void on( t &i, uint s )
	{
		i |= s;
	}
	template <typename t>
	static inline void off( t &i, uint s )
	{
		i &= ~s;
	}
	
	static speed_t get_speed( uint baud )
	{
		for( uint i = 0; i < lenof( bauds ); i++ )
			if( baud == bauds[i].baud )
				return( bauds[i].speed );
		d_fprint_err( "valid bauds:\n" );
		for( uint i = 0; i < lenof( bauds ); i++ )
			d_fprint_err( "%7i\n", bauds[i].baud );
		ENSURE( false, "invalid baud: %i\n", baud );
		return( B0 );
	}
	
	static uint get_baud( speed_t speed )
	{
		for( uint i = 0; i < lenof( bauds ); i++ )
			if( speed == bauds[i].speed )
				return( bauds[i].baud );
		return( 0 );
	}
	
public:
	usbtty_io_t( cfgsection_t & cfg, signals_t::data_t const& _signals ) :
		unix_io_t( _signals, O_RDWR | O_SYNC | O_RSYNC | O_DSYNC, 0 )
	{
		reload( cfg );
	}
	
	void reload( cfgsection_t & cfg )
	{
		char old_device[lenof(device)];
		cpy( old_device, device, sizeof(device) );
		cfg.entry( device, "device", "/dev/ttyUSB0" );
		speed = get_speed( cfg.entry( "baud", 9600u ) );
		set_latency = cfg.entry( "set_latency", false );
		low_latency = cfg.entry( "low_latency", false );
		timeout = cfg.entry( "timeout", 5000000u );
		if( !old_device[0] )
			d_fprint( "using device %S @ baud %u\n",
			          name, get_baud( speed ) );
		else
			if( !d_memcmp( old_device, device, sizeof(device) ) )
				d_fprint( "changing to device %S @ baud %u\n",
				          name, get_baud( speed ) );
	}

	uint baud() const
	{
		return( get_baud( speed ) );
	}

	virtual void post_setup()
	{
		int ret;
		if( set_latency )
		{
			struct serial_struct s;
			zro( &s, sizeof(s) );
			ret = ioctl( fd, TIOCGSERIAL, &s );
			ENSURE( ret == 0, "error %i ioctl(TIOCGSERIAL) %S: 0x%1,1,16u\n",
					ret, name, errno );
			if( low_latency )
				on( s.flags, ASYNC_LOW_LATENCY );
			else
				off( s.flags, ASYNC_LOW_LATENCY );
			ret = ioctl( fd, TIOCSSERIAL, &s );
			ENSURE( ret == 0, "error %i ioctl(TIOCSSERIAL) %S: 0x%1,1,16u\n",
					ret, name, errno );

		}
		struct termios tty;
		zro( &tty, sizeof(tty) );
		ret = tcgetattr( fd, &tty );
		ENSURE( ret == 0, "error %i tcgetattr %S: 0x%1,1,16u\n",
				ret, name, errno );
		off( tty.c_iflag, IGNBRK );
		off( tty.c_iflag, BRKINT );
		off( tty.c_iflag, PARMRK );
		off( tty.c_iflag, ISTRIP );
		off( tty.c_iflag, INLCR );
		off( tty.c_iflag, IGNCR );
		off( tty.c_iflag, ICRNL );
		off( tty.c_iflag, IXON );
		off( tty.c_oflag, OPOST );
		off( tty.c_lflag, ECHO );
		off( tty.c_lflag, ECHONL );
		off( tty.c_lflag, ICANON );
		off( tty.c_lflag, ISIG );
		off( tty.c_lflag, IEXTEN );
		off( tty.c_cflag, CSIZE );
		off( tty.c_cflag, PARENB );
		off( tty.c_cflag, PARODD );
		off( tty.c_cflag, CSTOPB );
		on( tty.c_cflag, CS8 );
		ret = cfsetspeed( &tty, speed );
		ENSURE( ret == 0, "error %i cfsetspeed %S: 0x%1,1,16u\n",
				ret, name, errno );
		ret = tcflush( fd, TCIOFLUSH );
		ENSURE( ret == 0, "error %i tcflush %S: 0x%1,1,16u\n",
				ret, name, errno );
		ret = tcsetattr( fd, TCSANOW, &tty );
		ENSURE( ret == 0, "error %i tcsetattr %S: 0x%1,1,16u\n",
				ret, name, errno );
		ret = tcflush( fd, TCIOFLUSH );
		ENSURE( ret == 0, "error %i tcflush %S: 0x%1,1,16u\n",
				ret, name, errno );
	}
	virtual void pre_teardown()
	{
	}
};

#endif // USBTTYIO_INCLUDED

