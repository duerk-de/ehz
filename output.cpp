#ifndef OUTPUT_INCLUDED
#define OUTPUT_INCLUDED

#include "db_defines.cpp"
#include "db_consoleio.cpp"
#include "db_file.cpp"
#include "cfg.cpp"
#include "fileio.cpp"
#include "udp.cpp"
#include "ringbuf.cpp"
#include "data.cpp"
#include "mars.cpp"
#include "statistics.cpp"


template <uint num>
struct output_t
{
	signals_t::data_t const& signals;
	
	ulint log1dtime;
	ulint log2dtime;
	constexpr static int oflags = O_RDWR | O_CREAT | O_APPEND | O_LARGEFILE;
	constexpr static int iflags = S_IREAD | S_IWRITE | S_IRGRP | S_IROTH;
	file_io_t out;
	file_io_t raw;
	file_io_t state;
	file_io_t log1;
	file_io_t log2;
	ulint log1time;
	ulint log2time;
	udps_t udps;
	
	ringbuf_t<power_meter_t> bufs[num];
	rawbuf_t & rawbuf;
	power_meter_t * const (&meters)[num];
	cstr_t const (&meternames)[num];
	statistics_t const& statistics;
	mars_t const& venus;

	output_t( cfgsection_t & cfg,
	          signals_t::data_t const& _signals,
	          rawbuf_t & _rawbuf,
	          power_meter_t * const (&_meters)[num],
	          cstr_t const (&_meternames)[num],
	          statistics_t const& _statistics,
	          mars_t const& _venus ) :
		signals( _signals ),
		log1dtime( 0 ),
		log2dtime( 0 ),
		out( signals, oflags, iflags ),
		raw( signals, oflags, iflags ),
		state( signals, oflags, iflags ),
		log1( signals, oflags, iflags ),
		log2( signals, oflags, iflags ),
		log1time( 0 ),
		log2time( 0 ),
		udps( cfg ),
		rawbuf( _rawbuf ),
		meters( _meters ),
		meternames( _meternames ),
		statistics( _statistics ),
		venus( _venus )
	{
		reload( cfg );
	}
	
	~output_t()
	{
		out.teardown();
		raw.teardown();
		state.teardown();
		log1.teardown();
		log2.teardown();
	}
	
	void reload( cfgsection_t & cfg )
	{
		out.teardown();
		cfg.entry( out.device, "outfile", "" );
		if( out.device[0] )
		{
			d_fprint( "using outfile %S\n", cstr_t( out.device ) );
			out.setup();
		}
		
		raw.teardown();
		cfg.entry( raw.device, "rawfile", "" );
		if( raw.device[0] )
		{
			d_fprint( "using rawfile %S\n", cstr_t( raw.device ) );
			raw.setup();
		}

		state.teardown();
		cfg.entry( state.device, "statefile", "" );
		if( state.device[0] )
		{
			d_fprint( "using statfile %S\n", cstr_t( state.device ) );
			state.setup();
		}
		
		log1dtime = (ulint)cfg.entry( "log1dtime", 30 ) * nsec_t<ulint>::sec;
		
		log1.teardown();
		cfg.entry( log1.device, "log1file", "" );
		if( log1.device[0] )
		{
			d_fprint( "using logfile %S with an interval of %t\n",
			          cstr_t( log1.device ),
			          (double)log1dtime / (double)nsec_t<ulint>::sec );
			log1.setup();
		}
		
		log2dtime = (ulint)cfg.entry( "log2dtime", 30 ) * nsec_t<ulint>::sec;
		
		log2.teardown();
		cfg.entry( log2.device, "log2file", "" );
		if( log2.device[0] )
		{
			d_fprint( "using log2file %S with an interval of %t\n",
			          cstr_t( log2.device ),
			          (double)log2dtime / (double)nsec_t<ulint>::sec );
			log2.setup();
		}

		udps.reload( cfg );
		
		for( uint i = 0; i < num; i++ )
			bufs[i].resize_truncfirst( d_max( 1ull,
			                                  div_ceil( d_max( log1dtime, log2dtime ),
		                                                nsec_t<ulint>::sec ) ) );
	}
	
	template <typename t>
	static constexpr t div_round( t val, t div )
	{
		return( ( val + ( val >= 0 ? div : -div ) / 2 ) / div );
	}
	
	template <typename t>
	static constexpr t div_ceil( t val, t div )
	{
		return( ( val + ( val >= 0 ? div - 1 : - ( div - 1 ) ) ) / div );
	}
	
	void mkraw( ulint timestamp )
	{
		char wbuf[32];
		for( uint i = 0; i < num; i++ )
		{
			if( !bufs[i].num() )
			{
				set( wbuf, (int)sizeof(wbuf) - 1, (byte)' ' );
				wbuf[sizeof(wbuf) - 1] = '\0';
			} else {
				power_meter_t const& data = bufs[i][0];
				uint dtime = (uint)( timestamp - data.timestamp );
				int pwr = div_round( data.pwr, power_meter_t::pwr_milli );
				int dpwr = div_round( data.pwr_error, power_meter_t::pwr_milli );
				uint feng = (uint)div_round( data.energy_forward, power_meter_t::energy_milli );
				uint reng = (uint)div_round( data.energy_reverse, power_meter_t::energy_milli );
				rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
										"(-%2,2im:%2,2is)"
										"%5,1iW(±%2,1iW)"
										"%6,1u.%1,1u/%4,1u.%1,1ukWh\n",
										dtime / sec_t<uint>::min, dtime % sec_t<uint>::min,
										pwr, dpwr,
										feng / 1000, ( feng / 100 ) % 10,
										reng / 1000, ( reng / 100 ) % 10 );
			}
		}

		for( uint i = 0; i < num; i++ )
			rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
			                       "%S%S", i ? cstr_t( ", " ) : cstr_t( "" ), meternames[i] );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len, "\n" );

		timefclass tf( (double)timestamp / (double)nsec_t<ulint>::sec );
		tf.printf( wbuf, sizeof(wbuf), "%04y-%02m-%02d(%1o)%02h:%02i:%02s" );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
								"%S\n\n", cstr_t( wbuf ) );
	}

	void writeout( file_io_t & out ) const
	{
		char linebuf[1024];
		uint linelen = 0;
		for( uint i = 0; i < num; i++ )
		{
			if( i )
				linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen, "\t" );

			power_meter_t const& data = bufs[i][0];
			linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen,
								"%13,0,0,0,13d\t"
								"%6,0,0,0,6d\t",
								(double)data.timestamp / (double)nsec_t<ulint>::sec,
								(double)data.pwr / (double)power_meter_t::pwr_milli );
		}

		linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen,
							"%13,0,0,0,13d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%6,0,0,0,6d\t"
							"%u\t",
							(double)venus.data.timestamp / (double)nsec_t<ulint>::sec,
							(double)venus.data.out / (double)power_meter_t::pwr_milli,
							(double)venus.data.pack_power / (double)power_meter_t::pwr_milli,
							(double)venus.data.offset / (double)power_meter_t::pwr_milli,
							(double)venus.stat.consumption / (double)power_meter_t::pwr_milli,
							(double)venus.stat.maximum_negative_setpoint_error / (double)power_meter_t::pwr_milli,
							(double)venus.stat.negative_variance / (double)power_meter_t::pwr_milli,
							(double)venus.data.level * 1e-5,
							(double)venus.data.cell_voltage * 1e-6,
							(double)venus.data.cell_voltage_diff * 1e-6,
							(double)venus.data.cell_voltage_max * 1e-6,
							(double)venus.data.cell_voltage_min * 1e-6,
							(double)venus.fe( venus.cell.data.data().hivolt ),
							(double)venus.fe( venus.cell.data.data().full ),
							(double)venus.data.powerdiff / (double)power_meter_t::pwr_milli,
							venus.data.cell_voltages_num_constant );

		linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen, "\n" );
		out.write( (byte const*)linebuf, linelen );
	}
	
	void writestate( file_io_t & file ) const
	{
		char linebuf[1024];
		uint linelen = 0;
		linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen,
		                    "%13,0,0,0,13d\t"
		                    "%S\t"
		                    "%S\t"
		                    "%i\t"
		                    "%i\t"
		                    "%8,0,0,0,8d\t"
		                    "%6f\t"
		                    "%6f\t"
		                    "%6f\t"
		                    "%6f\t"
		                    "%6f\t"
		                    "%6F\t"
		                    "%2f\t"
		                    "%2f\n",
		                    (double)venus.data.timestamp / (double)nsec_t<ulint>::sec,
		                    cstr_t( venus.state.state_names[venus.state.oldstate] ),
		                    cstr_t( venus.state.state_names[venus.state.state] ),
		                    venus.state.oldstate,
		                    venus.state.state,
		                    (double)( venus.state.time - venus.state.oldtime ) / (double)nsec_t<ulint>::sec,
		                    venus.state.voltage_max,
		                    venus.state.voltage,
		                    venus.state.voltage_min,
		                    venus.state.voltage_max - venus.state.voltage,
		                    venus.state.voltage_max - venus.state.voltage_min,
		                    venus.state.energy - venus.state.oldenergy,
		                    venus.state.energyforward - venus.state.oldenergyforward,
		                    venus.state.energyreverse - venus.state.oldenergyreverse );
		file.write( (byte const*)linebuf, linelen );
	}
		
	void writeraw( file_io_t & raw ) const
	{
		raw.truncate();
		raw.write( (byte const*)rawbuf.buf, rawbuf.len + 1 );
	}
		
	void writelog( ulint timestamp, ulint logdtime, ulint & logtime, file_io_t & log ) const
	{
		if( timestamp >= logtime + logdtime )
		{
			char linebuf[1024];
			uint linelen = 0;
			for( uint i = 0; i < num; i++ )
			{
				if( i )
					linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen, "\t" );

				ulint time = 0;
				int pwr = 0;
				int fpwr = 0;
				int rpwr = 0;
				ulint feng = 0;
				ulint reng = 0;
				uint volt1 = 0;
				uint volt2 = 0;
				uint volt3 = 0;
				uint freq = 0;
				uint n = 0;
				for( uint j = 0; j < bufs[i].num(); j++ )
				{
					power_meter_t const& data = bufs[i][j];
					if( data.timestamp < logtime )
						break;
					time += data.timestamp / 1000ull;
					pwr += data.pwr;
					if( data.pwr < 0 )
						rpwr += -data.pwr;
					else
						fpwr += data.pwr;
					feng += data.energy_forward;
					reng += data.energy_reverse;
					volt1 += data.voltage1;
					volt2 += data.voltage2;
					volt3 += data.voltage3;
					freq += data.frequency;
					n++;
				}
				linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen,
									"%13,0,0,0,13d\t"
									"%6,0,0,0,6d\t"
									"%6,0,0,0,6d\t"
									"%6,0,0,0,6d\t"
									"%U\t"
									"%U\t"
									"%6,0,0,0,6d\t"
									"%6,0,0,0,6d\t"
									"%6,0,0,0,6d\t"
									"%6,0,0,0,6d",
									(double)time / (double)( n * nsec_t<ulint>::sec ) * (double)1000ull,
									(double)pwr / (double)( n * power_meter_t::pwr_milli ),
									(double)fpwr / (double)( n * power_meter_t::pwr_milli ),
									(double)rpwr / (double)( n * power_meter_t::pwr_milli ),
									(ulint)( feng / ( n * power_meter_t::energy_milli ) ),
									(ulint)( reng / ( n * power_meter_t::energy_milli ) ),
									(double)volt1 / (double)( n * power_meter_t::volt_milli ),
									(double)volt2 / (double)( n * power_meter_t::volt_milli ),
									(double)volt3 / (double)( n * power_meter_t::volt_milli ),
									(double)freq / (double)( n * power_meter_t::freq_milli ) );
			}
			
			lint time = 0;
			lint harvested = 0;
			lint consumed = 0;
			if( statistics.getvals( timestamp / nsec_t<ulint>::msec + statistics.getlogoff(),
			                        time, harvested, consumed ) )
			{
				if( num )
					linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen, "\t" );
				linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen,
									"%13,0,0,0,13d\t"
								    "%3,4F\t"
								    "%3,4F\t",
								    statistics.msec_to_dbl( time ),
								    harvested,
								    consumed );
				linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen,
							        "%6,0,0,0,6d\t"
							        "%6,0,0,0,6d\t"
							        "%6,0,0,0,6d\t"
							        "%6,0,0,0,6d",
			                        (double)venus.data.level * 1e-5,
			                        (double)venus.stat.cell_voltage_max * 1e-3,
			                        (double)venus.stat.cell_voltage_min * 1e-3,
			                        (double)venus.data.voltlevel * 1e-5 );
			}
			
			linelen += d_fstrn( linebuf + linelen, sizeof(linebuf) - linelen, "\n" );
			log.write( (byte const*)linebuf, linelen );
			logtime = timestamp;
		}
	}
	
	void write( ulint timestamp )
	{
		bool changed = false;
		for( uint i = 0; i < num; i++ )
		{
			if( !bufs[i].num() || meters[i]->changed( bufs[i][0] ) )
			{
				bufs[i].add( power_meter_t( *meters[i] ) );
				changed =  true;
			}
		}
		if( venus.state.changed() )
			writestate( state );
		if( changed )
		{
			mkraw( timestamp );
			// ::out( rawbuf.buf, rawbuf.len );
			udps.write( (byte const*)rawbuf.buf, rawbuf.len );
			if( raw.device[0] )
				writeraw( raw );
			if( out.device[0] )
				writeout( out );
			if( log1.device[0] )
				writelog( timestamp, log1dtime, log1time, log1 );
			if( log2.device[0] )
				writelog( timestamp, log2dtime, log2time, log2 );
		}
	}
};


#endif // OUTPUT_INCLUDED

