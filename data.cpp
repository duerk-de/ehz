#ifndef DATA_INCLUDED
#define DATA_INCLUDED

#include "cfg.cpp"
#include "db_defines.cpp"
#include "db_mem.cpp"

struct power_meter_t
{
	ulint timestamp_min;
	ulint timestamp;
	int pwr;
	int pwr_error;
	ulint energy_forward;
	ulint energy_reverse;
	uint voltage1;
	uint voltage2;
	uint voltage3;
	uint frequency;
	ulint energy_forward_offset;
	ulint energy_reverse_offset;
	
	// data stored in these units: mW, mWh, mV, mHz
	
	constexpr static int pwr_milli = 1000;
	constexpr static ulint energy_milli = 1000ull;
	constexpr static uint volt_milli = 1000u;
	constexpr static uint freq_milli = 1000u;
	
	constexpr power_meter_t() :
		timestamp_min( 0 ),
		timestamp( 0 ),
		pwr( 0 ),
		pwr_error( 0 ),
		energy_forward( 0 ),
		energy_reverse( 0 ),
		voltage1( 0 ),
		voltage2( 0 ),
		voltage3( 0 ),
		frequency( 0 ),
		energy_forward_offset( 0 ),
		energy_reverse_offset( 0 )
	{ }
	
	power_meter_t( cfgsection_t & cfg )
	{
		reload( cfg );
	}
	
	void reload( cfgsection_t & cfg )
	{
		energy_forward_offset = cfg.entry( "forward_offset", 0ll );
		energy_reverse_offset = cfg.entry( "reverse_offset", 0ll );
	}
	
	bool changed( power_meter_t const& d ) const
	{
		return( timestamp != d.timestamp );
	}
	
	void apply_offset()
	{
		energy_forward += energy_forward_offset;
		energy_reverse += energy_reverse_offset;
	}
	
	void balance( power_meter_t const& production, power_meter_t const& external )
	{
		timestamp_min = d_min( production.timestamp_min, external.timestamp_min );
		timestamp = ( production.timestamp + external.timestamp ) / 2;
		pwr = production.pwr + external.pwr;
		pwr_error = production.pwr_error + external.pwr_error;
		energy_forward = production.energy_forward + external.energy_forward
		             - ( production.energy_reverse + external.energy_reverse );
		energy_reverse = 0;
		voltage1 = external.voltage1;
		voltage2 = external.voltage2;
		voltage3 = external.voltage3;
		frequency = ( production.frequency + external.frequency ) / 2;
	}
};

struct rawbuf_t
{
	constexpr static uint max = 1 << 12;
	char buf[max];
	uint len;
	
	constexpr rawbuf_t() :
		buf{ 0 },
		len( 0 )
	{ }
	
	void reset()
	{
		buf[0] = '\0';
		len = 0;
	}
};


#endif // DATA_INCLUDED

