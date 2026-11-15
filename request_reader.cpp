#ifndef REQUEST_READER_INCLUDED
#define REQUEST_READER_INCLUDED

#include "db_defines.cpp"
#include "db_time.cpp"
#include "io.cpp"
#include "request_response.cpp"
#include "stat.cpp"
#include "ctrl.cpp"


struct request_reader_t
{
	io_t & io;
	request_response_t & rr;
	calc_stats_t<3> stats;
	
	static constexpr bool dbg = false;

	constexpr request_reader_t( io_t & _io, request_response_t & _rr ) :
		io( _io ),
		rr( _rr )
	{
		d_fprint( "request_reader of %S at %S: "
		          "request: %u, receive: %u, timeout: %3fms\n",
		          rr.name, io.name,
		          rr.get_request().len, rr.get_response().len, io.timeout );
	}
	
	static void write( io_t & io, byte const* buf, uint len,
	                   uint * write_time = nullptr )
	{
		// constexpr static uint delay_per_byte = 32000;
		ulint t0 = getintnanos();
		io.write( buf, len );
		ulint t1 = getintnanos();
		if( write_time )
			*write_time = ( t1 - t0 ) / 1000;
		// sleepus( len * delay_per_byte );
	}

	static uint read( io_t & io, byte * buf, uint len, uint timeout = 0,
	                  uint * wait_time = nullptr, uint * read_time = nullptr )
	{
		constexpr static uint timestep = 500;
		ulint t0 = getintnanos();
		ulint t1 = 0;
		ulint t2 = 0;
		uint pos = 0;
		for( ;; )
		{
			pos += io.read( buf + pos, len - pos );
			if( !t1 && pos )
				t1 = getintnanos();
			if( !( pos < len ) )
				break;
			uint timepassed = ( getintnanos() - t0 ) / 1000;
			if( !( timepassed < timeout ) )
				break;
			uint timesleep = timeout - timepassed;
			if( timesleep > timestep )
				timesleep = timestep;
			sleepus( timesleep );
		}
		t2 = getintnanos();
		if( wait_time )
			*wait_time = ( t1 ? t1 - t0 : 0 ) / 1000;
		if( read_time )
			*read_time = ( t1 ? t2 - t1 : t2 - t0 ) / 1000;
		return( pos );
	}
	
	void write_request( uint * twrite )
	{
		request_response_t::buf_t req = rr.get_request();
		write( io, req.buf, req.len, twrite );
		if( dbg )
			d_fprint( "request: %*x\n", req.len, req.buf );
	}
	
	bool read_response( uint * twait, uint * tread )
	{
		request_response_t::buf_t & res = rr.get_response();
		uint res_req_len = res.len;
		res.len = read( io, res.buf, res.len, io.timeout, twait, tread );
		if( dbg )
			d_fprint( "response: %*x (%u)\n", res.len, res.buf, res.len, res_req_len );
		if( res.len != res_req_len )
			d_fprint_err( "request_reader of %S at %S failed: "
			              "requested: %i, received: %i, timeout: %3fms, "
			              "twait: %3fms, tread: %3fms, "
			              "request: %*x, response: %*x\n",
			              rr.name, io.name,
			              res_req_len, res.len, io.timeout,
			              twait ? *twait : 0, tread ? *tread : 0,
			              rr.get_request().len, rr.get_request().buf, res.len, res.buf );
		return( res.len == res_req_len );
	}
	
	bool run()
	{
		uint dt[3];
		write_request( &dt[0] );
		return( read_response( &dt[1], &dt[2] ) );
	}
};

#endif // REQUEST_READER_INCLUDED

