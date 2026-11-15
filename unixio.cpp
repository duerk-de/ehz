#ifndef UNIX_IO_INCLUDED
#define UNIX_IO_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_file.cpp"
#include "io.cpp"
#include "signals.cpp"
#include <sys/ioctl.h>
#include <sys/poll.h>

struct unix_io_t : public io_t
{
	signals_t::data_t const& signals;
	int fd;
	const int oflags;
	const int sflags;
	char device[256];

	constexpr unix_io_t( signals_t::data_t const& _signals, int _oflags, int _sflags ) :
		io_t( device ),
		signals( _signals ),
		fd( -1 ),
		oflags( _oflags ),
		sflags( _sflags ),
		device{ 0 }
	{ }
	template <uint len>
	bool syscall_again( int ret, int err, char const (& fnc)[len] )
	{
		if( ret >= 0 )
			return( false );
		if( err == EINTR )
		{
			if( signals.term )
			{
				d_fprint_err( "%*s of %*s interrupted due to sigterm\n",
							  (uint)sizeof(fnc), fnc, (uint)sizeof(device), device );
				return( false );
			}
			d_fprint_err( "%*s of %*s interrupted due to a signal\n",
						  (uint)sizeof(fnc), fnc, (uint)sizeof(device), device );
			return( true );
		}
		d_fprint_err( "%*s of %*s interrupted: 0x%1,1,16u\n",
		              (uint)sizeof(fnc), fnc, (uint)sizeof(device), device, err );
		return( false );
	}
	bool readable()
	{
		pollfd pfd;
		int pret;
		do {
			pfd.fd = fd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			pret = poll( &pfd, 1, 0 );
		} while( syscall_again( pret, errno, __FUNCTION__ ) );
		ENSURE( pret >= 0, "error %i polling %*s: 0x%1,1,16u\n",
		        pret, (uint)sizeof(device), device, errno );
		return( pfd.revents & POLLIN );
	}
	virtual void post_setup() = 0;
	virtual void pre_teardown() = 0;
	virtual void setup()
	{
		do
			fd = _db_open( device, oflags, sflags );
		while( syscall_again( fd, errno, __FUNCTION__ ) );
		ENSURE( fd >= 0, "error %i opening %*s: 0x%1,1,16u\n",
		        fd, (uint)sizeof(device), device, errno );
		post_setup();
	}
	virtual void teardown()
	{
		pre_teardown();
		_db_close( fd );
		fd = -1;
	}
	virtual uint read( byte * buf, uint maxlen )
	{
		int nread;
		do {
			if( !readable() )
				return( 0 );
			nread = _db_read( fd, buf, maxlen );
		} while( syscall_again( nread, errno, __FUNCTION__ ) );
		ENSURE( nread >= 0, "error %i reading %*s: 0x%1,1,16u\n",
		        nread, (uint)sizeof(device), device, errno );
		ENSURE( (uint)nread <= maxlen, "fatal error reading %*s: %u > %u\n",
		        (uint)nread, maxlen );
		return( nread );
	}
	virtual void write( byte const* buf, uint len )
	{
		uint pos = 0;
		while( len )
		{
			int nwritten;
			do
				nwritten = _db_write( fd, buf + pos, len );
			while( syscall_again( nwritten, errno, __FUNCTION__ ) );
			ENSURE( nwritten >= 0, "error %i writing %*s: 0x%1,1,16u, %i bytes left\n",
					nwritten, (uint)sizeof(device), device, errno, len );
			len -= nwritten;
			pos += nwritten;
		}
	}
};

#endif // UNIX_IO_INCLUDED

