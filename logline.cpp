#ifndef LOGLINE_INCLUDED
#define LOGLINE_INCLUDED

#include "ctrl.cpp"
#include "reader.cpp"

struct logline_t : public reader_line_t
{
	template <typename t>
	struct datat_t
	{
		bool valid;
		t val;
		datat_t() :
			valid( false ),
			val( 0 )
		{ }
		void reset()
		{
			valid = false;
		}
		datat_t diff( datat_t const& s ) const
		{
			datat_t d;
			d.valid = valid && s.valid;
			d.val = val - s.val;
			return( d );
		}
	};
	
	typedef datat_t<lint> data_t;
	
	struct entry_t
	{
		data_t time;
		data_t power;
		data_t power_forward;
		data_t power_reverse;
		data_t forward;
		data_t reverse;
		void reset()
		{
			time.reset();
			power.reset();
			power_forward.reset();
			power_reverse.reset();
			forward.reset();
			reverse.reset();
		}
		entry_t diff( entry_t const& s ) const
		{
			entry_t d;
			d.time = time.diff( s.time );
			d.power = power.diff( s.power );
			d.power_forward = power_forward.diff( s.power_forward );
			d.power_reverse = power_reverse.diff( s.power_reverse );
			d.forward = forward.diff( s.forward );
			d.reverse = reverse.diff( s.reverse );
			return( d );
		}
	};
	
	entry_t total;
	entry_t solar;
	entry_t external;
	entry_t generator;
	
	uint day;
	
	uint iline;
	uint icol;
	uint ntabs;
	char const* ptr;
	
	logline_t() :
		day( 0 ),
		iline( 0 ),
		icol( 0 ),
		ntabs( 0 ),
		ptr( nullptr )
	{ }
	
	static uint calcday( ulint time )
	{
		static constexpr uint hours_per_day = sec_t<ulint>::day / sec_t<ulint>::hour;
		static constexpr ulint hour = 4 - 1;
		uint hours = time / msec_t<uint>::hour;
		return( ( hours - hour ) / hours_per_day );
	}
	
	virtual void reset()
	{
		total.reset();
		solar.reset();
		external.reset();
		generator.reset();
		day = 0;
		icol = 0;
		ptr = nullptr;
	}
	
	logline_t diff( logline_t const& s ) const
	{
		logline_t d;
		d.total = total.diff( s.total );
		d.solar = solar.diff( s.solar );
		d.external = external.diff( s.external );
		d.generator = generator.diff( s.generator );
		d.day = day - s.day;
		return( d );
	}
	
	static inline bool getdouble( char const* ptr, uint b, uint e, double & val )
	{
		uint iparsed;
		double d = str2doublel( ptr, b, e, iparsed );
		if( iparsed != e - b )
			return( false );
		val = d;
		return( true );
	}

	virtual void set( char const* _ptr, uint _l )
	{
		reset();
		iline++;
		ptr = _ptr;
		ntabs = 0;
		if( ptr )
			for( uint i = 0; i < _l; i++ )
				if( ptr[i] == '\t' )
					ntabs++;
	}
	virtual void add( uint b, uint e )
	{
		uint ndata;
		switch( ntabs + 1 )
		{
			case 3 * 8:
				ndata = 8;
				break;
			case 3 * 10:
				ndata = 10;
				break;
			case 4 * 10:
				ndata = 10;
				break;
			default:
				if( ntabs + 1 > 4 * 10 )
					ndata = 10;
				else
					ndata = 0;
				break;
		}
		
		if( !ndata )
			return;
		
		uint ientry = icol / ndata;
		uint idata = icol % ndata;

		icol++;
		
		entry_t * entry;
		switch( ientry )
		{
			case 0:
				entry = &total;
				break;
			case 1:
				entry = &solar;
				break;
			case 2:
				entry = &external;
				break;
			case 3:
				entry = &generator;
				break;
			default:
				entry = nullptr;
				break;
		}
		if( !entry )
			return;

		data_t * data;
		double scale;
		switch( ndata )
		{
			case 8:
				switch( idata )
				{
					case 0:
						data = &entry->time;
						scale = 1e3;
						break;
					case 1: // power
						data = &entry->power;
						scale = 1e3;
						break;
					case 2:
						data = &entry->forward;
						scale = 0;
						break;
					case 3:
						data = &entry->reverse;
						scale = 0;
						break;
					case 4: // voltage 1
					case 5: // voltage 2
					case 6: // voltage 3
					case 7: // frequency
					default:
						data = nullptr;
						break;
				}
				break;
			case 10:
				switch( idata )
				{
					case 0:
						data = &entry->time;
						scale = 1e3;
						break;
					case 1: // power
						data = &entry->power;
						scale = 1e3;
						break;
					case 2: // power forward
						data = &entry->power_forward;
						scale = 1e3;
						break;
					case 3: // power reverse
						data = &entry->power_reverse;
						scale = 1e3;
						break;
						break;
					case 4:
						data = &entry->forward;
						scale = 0;
						break;
					case 5:
						data = &entry->reverse;
						scale = 0;
						break;
					case 6: // voltage 1
					case 7: // voltage 2
					case 8: // voltage 3
					case 9: // frequency
					default:
						data = nullptr;
						break;
				}
				break;
			default:
				data = nullptr;
				break;

		}
		if( !data )
			return;
		
		double d = 0;
		data->valid = getdouble( ptr, b, e, d );
		data->val = scale ? d * scale : d;

		switch( idata )
		{
			case 0:
				if( data->valid && data->val < 1000000000000 )
					data->valid = false;
				break;
			default:
				break;
		}
	}
	
	bool parse()
	{
		if( !total.time.valid )
			return( false );
		if( !solar.time.valid || !external.time.valid
		 || !solar.forward.valid || !external.forward.valid
		 || !solar.reverse.valid || !external.reverse.valid )
			return( false );
		if( !solar.time.val || !external.time.val )
			return( false );
		
		day = calcday( total.time.val );
		
		return( day != 0 );
	}
};

#endif // LOGLINE_INCLUDED

