#ifndef SOCKET_INCLUDED
#define SOCKET_INCLUDED

#include "db_defines.cpp"
#include "ctrl.cpp"
#include "cfg.cpp"
#include "linbuf.cpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

struct udp_t
{
	static constexpr ushort bswap16( ushort u )
	{
		return( ( ( u & 0xff00u ) >>  8 )
		      | ( ( u & 0x00ffu ) <<  8 ) );
	}

	static constexpr uint bswap32( uint u )
	{
		return( ( ( u & 0xff000000u ) >> 24 )
		      | ( ( u & 0x00ff0000u ) >>  8 )
		      | ( ( u & 0x0000ff00u ) <<  8 )
		      | ( ( u & 0x000000ffu ) << 24 ) );
	}

	int sock;
	struct sockaddr_in sin;

	~udp_t()
	{
		if( sock >= 0 )
			close( sock );
	}
	
	constexpr udp_t() :
		sock( -1 ),
		sin{ .sin_family = AF_INET,
		     .sin_port = bswap16( 0 ),
		     .sin_addr = { .s_addr = bswap32( 0 ) },
		     .sin_zero = { 0 } }
	{ }
	
	udp_t( udp_t const& u ) = delete;
	udp_t( udp_t && u ) :
		sock( u.sock ),
		sin{ u.sin }
	{
		u.sock = -1;
	}
	
	udp_t operator =( udp_t const& u ) = delete;
	void operator =( udp_t && u ) = delete;
	
	constexpr static inline bool temporarily_interrupted( int ret, int err )
	{
		return( ret < 0 && ( err == EINTR ) );
	}

	bool reload( char const* addr, uint port )
	{
		struct sockaddr_in _sin = sin;
		_sin.sin_addr.s_addr = inet_addr( addr );
		_sin.sin_port = htons( port );
		
		if( cmp( _sin, sin ) )
			return( false );

		if( sock >= 0 )
			close( sock );
		cpyv( sin, _sin );
		do
			sock = socket( AF_INET, SOCK_DGRAM, 0 );
		while( temporarily_interrupted( sock, errno ) );
		ENSURE( sock >= 0, "error creating socket: %x\n", errno );

		int ret;
		int broadcastEnable = 1;
		do
			ret = setsockopt( sock, SOL_SOCKET, SO_BROADCAST,
			                  &broadcastEnable, sizeof(broadcastEnable) );
		while( temporarily_interrupted( ret, errno ) );
		ENSURE( ret >= 0, "error setsockopt: %x\n", errno );
		
		return( true );
	}

	void write( byte const* buf, uint len ) const
	{
		uint pos = 0;
		while( len )
		{
			int nwritten;
			do
				nwritten = sendto( sock, buf + pos, len, 0,
				                   (struct sockaddr *)&sin, sizeof(sin));
			while( temporarily_interrupted( nwritten, errno ) );
			if( nwritten < 0 )
			{
				d_fprint_err( "error writing: %1,1,16u (%i bytes left)\n", errno, len );
				break;
			}
			len -= nwritten;
			pos += nwritten;
		}
	}
};


struct udps_t : public not_copyable_t
{
	linbuf_t<udp_t> cons;

	udps_t( cfgsection_t & cfg )
	{
		reload( cfg );
	}
	
	void reload( cfgsection_t & cfg )
	{
		uint num = cfg.entry( "udpnum", 0u );
		cons.resize_trunclast( num );
		while( cons.num() < num )
			cons.add( udp_t() );
		for( uint i = 0; i < num; i++ )
		{
			char var[64];
			char addr[64];
			uint port;
			d_fstrn( var, sizeof(var), "%s%i", "udpaddr", i + 1 );
			cfg.entry( addr, var, "" );
			d_fstrn( var, sizeof(var), "%s%i", "udpport", i + 1 );
			port = cfg.entry( var, 0u );
			if( cons[i].reload( addr, port ) )
				d_fprint( "creating socket #%i for %S:%i\n",
						  i+1, cstr_t( addr ), port );
		}
	}

	void write( byte const* buf, uint len ) const
	{
		for( uint i = 0; i < cons.num(); i++ )
			cons[i].write( buf, len );
	}
};


#endif // SOCKET_INCLUDED

