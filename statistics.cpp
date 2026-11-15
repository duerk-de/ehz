#ifndef STATISTICS_INCLUDED
#define STATISTICS_INCLUDED

#include "cfg.cpp"
#include "logline.cpp"
#include "ringbuf.cpp"
#include "linbuf.cpp"
#include "stat.cpp"
#include "interpolation.cpp"
#include "data.cpp"
#include "ctrl.cpp"
#include "db_time.cpp"


class statistics_t
{
public:
	static constexpr double msec_to_dbl( lint msec )
	{
		return( (double)msec / (double)msec_t<lint>::sec );
	}

	template <uint l>
	struct arr_t {
		lint data[l];
	};

	enum per_slot_data_t { per_slot_harvested, per_slot_consumed };
	static constexpr uint per_slot_max = (uint)per_slot_consumed + 1;
	typedef stats_t<per_slot_max> stats_per_slot_t;
	typedef calc_stats_t<per_slot_max> stats_per_slot_calc_t;
	
	enum across_days_data_t { across_days_harvested, across_days_consumed };
	static constexpr uint across_days_max = (uint)across_days_consumed + 1;
	typedef stats_t<across_days_max> stats_across_days_t;
	typedef calc_stats_t<across_days_max> stats_across_days_calc_t;
	
	static constexpr arr_t<across_days_max>
	across_days( stats_per_slot_t const& per_slot )
	{
		return( arr_t<across_days_max>{ .data = {
			per_slot.data[per_slot_harvested].avg, 
			per_slot.data[per_slot_consumed].avg } } );
	}

	enum time_averaged_data_t { time_averaged_harvested, time_averaged_consumed };
	static constexpr uint time_averaged_max = (uint)time_averaged_consumed + 1;
	typedef stats_t<time_averaged_max> time_averaged_stats_t;
	typedef calc_stats_t<time_averaged_max> time_averaged_calc_stats_t;

	static constexpr arr_t<time_averaged_max>
	time_averaged( stats_across_days_t const& across_days )
	{
		return( arr_t<time_averaged_max>{ .data = {
			across_days.data[across_days_harvested].max, 
			across_days.data[across_days_consumed].min } } );
	}
	
	struct cfg_t
	{
		ulint quant;
		ulint avg;
		ulint stat;
		ulint calc;
		bool operator ==( cfg_t const& c ) const
		{
			return( quant == c.quant
			     && avg   == c.avg
			     && stat  == c.stat
			     && calc  == c.calc );
		}
	};
	
protected:
	rawbuf_t & rawbuf;

	cfg_t rawcfg;

	bool outofrange;
	
	uint slots_per_day;
	uint stat_days;
	uint calc_slots;
	uint avg_slots;

	stats_per_slot_calc_t current_slot;
	ringbuf_t<stats_per_slot_t> stats_per_slot;

	ringbuf_t<stats_across_days_calc_t> stats_across_days_calc;
	ringbuf_t<stats_across_days_t> stats_across_days;
	
	ringbuf_t<time_averaged_stats_t> stats_time_averaged;
	
	lint logoff;
	cstr_t logfile;

public:
	statistics_t( cfgsection_t & cfg, cstr_t _logfile, rawbuf_t & _rawbuf ) :
		rawbuf( _rawbuf ),
		outofrange( false ),
		slots_per_day( 0 ),
		stat_days( 0 ),
		calc_slots( 0 ),
		avg_slots( 0 ),
		logoff( 0 ),
		logfile( _logfile )
	{
		reload( cfg );
	}
	
	ringbuf_t<time_averaged_stats_t> const& stats() const
	{
		return( stats_time_averaged );
	}
	
	uint const& slots() const
	{
		return( slots_per_day );
	}
	
	template <uint len>
	void eval( calculator_t::varlist_t & vars, char const (&str)[len] ) const
	{
		calculator_t calc( str, d_strlen( str, len ), vars );
		if( !calc.parse() )
		{
			calc.print_errors();
			ENSURE( false, "calculation failed\n\n" );
		}
		calc.calc();
		vars = calc.getvars();
	}
	
	bool reload( cfgsection_t & cfg )
	{
		logoff = cfg.microseconds<lint>( "logoff" , "-1day" ) / usec_t<lint>::msec;
		
		cfg_t newcfg{ .quant = cfg.microseconds<ulint>( "quant", "5min"   ) / usec_t<ulint>::msec,
		              .avg   = cfg.microseconds<ulint>( "avg"  , "4hours" ) / usec_t<ulint>::msec,
		              .stat  = cfg.microseconds<ulint>( "stat" , "14days" ) / usec_t<ulint>::msec,
		              .calc  = cfg.microseconds<ulint>( "calc" , "2days"  ) / usec_t<ulint>::msec };
		
		if( newcfg == rawcfg )
			return( false );

		slots_per_day = ( msec_t<ulint>::day + newcfg.quant / 2 ) / newcfg.quant;
		stat_days = ( newcfg.stat + msec_t<ulint>::day / 2 ) / msec_t<ulint>::day;
		calc_slots = newcfg.calc * slots_per_day / msec_t<ulint>::day;
		avg_slots = d_max( 1ull, newcfg.avg * slots_per_day / msec_t<ulint>::day );

		current_slot = stats_per_slot_calc_t();
		
		stats_per_slot.clear();
		stats_across_days_calc.clear();
		stats_across_days.clear();
		stats_time_averaged.clear();

		stats_per_slot.resize_truncfirst( stat_days * slots_per_day + calc_slots + avg_slots / 2 );
		stats_across_days_calc.resize_truncfirst( calc_slots + avg_slots - 1 );
		stats_across_days.resize_truncfirst( stats_across_days_calc.maxnum() );
		stats_time_averaged.resize_truncfirst( calc_slots );
		
		rawcfg = newcfg;

		outofrange = false;
		load_from_logfiles( 5, 0 );

		return( true );
	}
	
	static void logname( char * name, char const* base, uint namelen, uint i )
	{
		if( i )
			d_fstrn( name, namelen, "%*s.%i", namelen, base, i );
		else
			d_fstrn( name, namelen, "%*s", namelen, base );
	}

	void load_from_logfiles( uint maxseqerr, uint ifileoff )
	{
		outofrange = false;
		stats_per_slot.clear();
		stats_across_days_calc.clear();
		stats_across_days.clear();
		stats_time_averaged.clear();
		
		char * ilogfilename = new char[logfile.len];
		if( !ilogfilename )
		{
			d_fprint_err( "could not allocate %u bytes for file name\n", logfile.len );
			return;
		}
		
		uint seqerr = 0;
		for( uint ifile = ifileoff; seqerr <= maxseqerr && !outofrange; ifile++ )
		{
			logname( ilogfilename, logfile.ptr, logfile.len, ifile );
			if( !file_t::is_available( ilogfilename, logfile.len ) )
			{
				d_fprint( "unavailable logfile: %*s\n", logfile.len, ilogfilename );
				seqerr++;
				continue;
			}
			seqerr = 0;
			uint i = 0;
			logline_t curr;
			reader_backward_t reader( ilogfilename, logfile.len );
			while( reader.nextline( curr ) && !outofrange )
			{
				if( curr.parse() )
				{
					add_backwards( curr );
					i++;
				}
			}
			d_fprint( "%i lines added from logfile %*s\n", i, logfile.len, ilogfilename );
		}
		stats_per_slot.reverse();
		calc_stats();
		
		delete [] ilogfilename;
	}

protected:
	inline uint calcslot( ulint time ) const
	{
		ulint time_per_slot = msec_t<ulint>::day / slots_per_day;
		return( ( time + time_per_slot / 2u ) / time_per_slot );
	}
	
	inline lint calctime( uint slot ) const
	{
		ulint time_per_slot = msec_t<ulint>::day / slots_per_day;
		return( slot * time_per_slot );
	}

	void add_backwards( logline_t const& line )
	{
		if( stats_per_slot.isfull() )
		{
			outofrange = true;
			return;
		}
		
		if( !line.total.time.valid )
			return;

		lint harvested;
		lint consumed;

		if( line.solar.power_forward.valid && line.solar.power_reverse.valid
		 && line.external.power_forward.valid && line.external.power_reverse.valid )
		{
			harvested = line.solar.power_forward.val - line.solar.power_reverse.val;
			lint imported = line.external.power_forward.val - line.external.power_reverse.val;
			if( line.generator.power_forward.valid && line.generator.power_reverse.valid )
			{
				// current format
				lint generated = line.generator.power_forward.val - line.generator.power_reverse.val;
				consumed = generated + imported;
			} else {
				consumed = harvested + imported;
			}
		}
		else if( line.solar.power.valid && line.external.power.valid )
		{
			harvested = line.solar.power.val;
			lint imported = line.external.power.val;
			consumed = harvested + imported;
		} else {
			return;
		}
		
		uint slot = calcslot( line.total.time.val );
		if( slot != current_slot.slot )
		{
			if( current_slot.slot )
				stats_per_slot.add( current_slot.stats() );
			current_slot = stats_per_slot_calc_t( slot );
		}
		
		current_slot.add( { harvested, consumed } );
	}

	void calc_stats()
	{
		stats_across_days_calc.clear();
		stats_across_days.clear();
		stats_time_averaged.clear();

		if( !stats_per_slot.num() )
			return;
		
		for( uint dayslot = stats_across_days_calc.maxnum(); dayslot--; )
			stats_across_days_calc.add( stats_across_days_calc_t( stats_per_slot[0].slot - dayslot ) );
		for( uint i = 0; i < stats_per_slot.num(); i++ )
		{
			uint dayslot0 = stats_per_slot[0].slot - stats_per_slot[i].slot;
			for( uint days = 0; days < stat_days; days++ )
			{
				uint dayslotoff = days * slots_per_day;
				if( dayslot0 >= dayslotoff )
				{
					uint dayslot = dayslot0 - dayslotoff;
					if( dayslot < stats_across_days_calc.maxnum() )
						stats_across_days_calc[dayslot].add( across_days( stats_per_slot[i] ).data );
				}
			}
		}
		
		for( uint dayslot = stats_across_days_calc.num(); dayslot--; )
			if( stats_across_days_calc[dayslot].data[across_days_harvested].num
			 && stats_across_days_calc[dayslot].data[across_days_consumed].num )
				stats_across_days.add( stats_across_days_calc[dayslot].stats() );
		
		if( stats_across_days.num() >= avg_slots )
		{
			for( uint i = stats_across_days.num() - ( avg_slots - 1 ); i--; )
			{
				time_averaged_calc_stats_t time_averaged_calc( ( stats_across_days[i+(avg_slots-1)].slot
				                                               + stats_across_days[i].slot ) / 2 );
				for( uint k = 0; k < avg_slots; k++ )
					time_averaged_calc.add( time_averaged( stats_across_days[i+k] ).data );
				stats_time_averaged.add( time_averaged_calc.stats() );
			}
		}
		
		current_slot = stats_per_slot_calc_t();
	}

	void add_current()
	{
		stats_per_slot.add( current_slot.stats() );
		
		uint dayslot = 0;
		stats_across_days_calc.add( stats_across_days_calc_t( stats_per_slot[dayslot].slot ) );

		uint days = 0;
		uint dayslotoff = days * slots_per_day;
		for( uint i = 0; i < stats_per_slot.num(); i++ )
		{
			uint dayslot0 = stats_per_slot[dayslot].slot - stats_per_slot[i].slot;
			if( dayslot0 == dayslotoff )
			{
				stats_across_days_calc[dayslot].add( across_days( stats_per_slot[i] ).data );
				if( ++days >= stat_days )
					break;
				dayslotoff = days * slots_per_day;
			}
		}
		if( !stats_across_days_calc[dayslot].data[across_days_harvested].num
		 || !stats_across_days_calc[dayslot].data[across_days_consumed].num )
			return;
		
		stats_across_days.add( stats_across_days_calc[dayslot].stats() );
		
		if( stats_across_days.num() < avg_slots )
			return;

		uint i = 0;
		time_averaged_calc_stats_t time_averaged_calc( ( stats_across_days[i+(avg_slots-1)].slot
		                                               + stats_across_days[i].slot ) / 2 );
		for( uint k = 0; k < avg_slots; k++ )
			time_averaged_calc.add( time_averaged( stats_across_days[i+k] ).data );
		stats_time_averaged.add( time_averaged_calc.stats() );
	}
	
public:
	void add( ulint utime, lint harvested, lint consumed )
	{
		uint slot = calcslot( utime / nsec_t<ulint>::msec );
		if( slot != current_slot.slot )
		{
			if( current_slot.slot )
				add_current();
			current_slot = stats_per_slot_calc_t( slot );
		}
		current_slot.add( { harvested, consumed } );
		
		lint time;
		if( getvals( utime / nsec_t<ulint>::msec - msec_t<lint>::day,
		             time, harvested, consumed ) )
			rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
								   "> statistics\n"
								   "%T\n"
								   "harvest[max]    : %3,5F W\n"
								   "consume[min]    : %3,5F W\n",
								   msec_to_dbl( time ),
								   harvested,
								   consumed );
	}
	
	bool getvals( lint req_time, lint & time, lint & harvested, lint & consumed ) const
	{
		ipol_pt_t<lint> pt( 0, 0, 0, 0 );
		iterator_up_t it( stats_time_averaged.num() );
		while( it.next() )
		{
			time_averaged_stats_t const& slo = stats_time_averaged[it.lo()];
			time_averaged_stats_t const& shi = stats_time_averaged[it.hi()];
			pt = ipol_pt_t<lint>( it.lo(), calctime( slo.slot ),
								           calctime( shi.slot ), req_time );
			if( !pt.valid( pt.type() ) )
				continue;
			break;
		}
		if( !it.inrange() )
			return( false );
		time_averaged_stats_t const& slo = stats_time_averaged[it.lo()];
		time_averaged_stats_t const& shi = stats_time_averaged[it.hi()];
		time = pt.val( calctime( slo.slot ), calctime( shi.slot ) );
		harvested = pt.val( slo.data[time_averaged_harvested].avg,
							shi.data[time_averaged_harvested].avg );
		consumed = pt.val( slo.data[time_averaged_consumed].avg,
						   shi.data[time_averaged_consumed].avg );
		return( true );
	}
	
	lint getlogoff() const
	{
		return( logoff );
	}
	
	/*
	time_averaged_stats_t const& s = stats_time_averaged[i];
	calctime( s.slot + slots_per_day )
	s.data[time_averaged_harvested].avg
	s.data[time_averaged_consumed].avg
	*/

};


#endif // STATISTICS_INCLUDED

