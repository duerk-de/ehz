#ifndef ENDLESS_READER_INCLUDED
#define ENDLESS_READER_INCLUDED

#include "db_defines.cpp"
#include "db_time.cpp"
#include "io.cpp"
#include "parser.cpp"
#include "ctrl.cpp"

template <uint len>
struct endless_reader_t
{
	io_t & io;
	syncer_t & syncer;
	parser_t & parser;
	parser_t & parser_part;
	uint bytepos;
	uint buf[len];
	uint endpos;
	
	constexpr endless_reader_t( io_t & _io, syncer_t & _syncer,
	                            parser_t & _parser, parser_t & _parser_part ) :
		io( _io ),
		syncer( _syncer ),
		parser( _parser ),
		parser_part( _parser_part ),
		bytepos()
	{ }
	
	bool try_read()
	{
		if( bytepos == len * sizeof(buf[0]) )
		{
			d_fprint_err( "buffer overflow\n" );
			bytepos = 0;
		}

		uint byteread = io.read( (byte *)buf + bytepos, len * sizeof(buf[0]) - bytepos );
		if( !byteread )
			return( false );
		bytepos += byteread;

		if( !syncer.sync( buf, bytepos ) )
			return( false );
		
		if( !syncer.findend( buf, bytepos, endpos ) )
			return( false );
		
		return( true );
	}

	bool read( uint timeout = 0, uint maxbytes = type_t<uint>::max() )
	{
		constexpr static uint timestep = 25000;
		ulint t0 = getintnanos();
		uint startbytepos = bytepos;
		if( maxbytes < type_t<uint>::max() - sizeof( buf[0] ) )
			maxbytes = ( maxbytes + ( sizeof( buf[0] ) - 1 ) ) / sizeof( buf[0] ) * sizeof( buf[0] );
		for( ;; )
		{
			if( try_read() )
				break;
			if( bytepos - startbytepos >= maxbytes )
			{
				if( maxbytes == type_t<uint>::max() )
					d_fprint_err( "maxbytes\n" );
				return( false );
			}
			uint timepassed = ( getintnanos() - t0 ) / 1000;
			if( !( timepassed < timeout ) )
			{
				d_fprint_err( "timeout: %3fms/%3fms\n", timepassed, timeout );
				d_fprint_err( "%*x\n", d_min( 1024u, bytepos ), buf );
				return( false );
			}
			uint timesleep = timeout - timepassed;
			if( timesleep > timestep )
				timesleep = timestep;
			sleepus( timesleep );
		}
		return( true );
	}
	
	bool run_part( ulint & timestamp, uint bytes_required, bool & endfound )
	{
		endfound = read( io.timeout, bytes_required );
		
		uint endbytepos = bytepos / sizeof( buf[0] );
		timestamp = getnanos();
		bool okay = parser_part.run( buf, endbytepos, timestamp, true );
		
		if( endbytepos * sizeof( buf[0] ) < bytes_required )
		{
			d_fprint_err( "not bytes_required\n" );
			return( false );
		}
		return( okay );
	}
	
	bool run( ulint timestamp, bool endfound )
	{
		if( !endfound )
			if( !read( io.timeout ) )
				return( false );
		
		bool okay = parser.run( buf, endpos, timestamp, false );
		syncer.shift( buf, bytepos, endpos );

		return( okay );
	}
};

#endif // ENDLESS_READER_INCLUDED

