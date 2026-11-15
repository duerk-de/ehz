#ifndef EFR_INCLUDED
#define EFR_INCLUDED

#include "db_defines.cpp"
#include "obis.cpp"
#include "cfg.cpp"

class efr_power_meter_t : public obis::power_meter_t
{
protected:
	uint minavg;
	int grace;
	uint last_n;

public:
	efr_power_meter_t( cfgsection_t & cfg ) : 
		obis::power_meter_t( cfg ),
		last_n( 0 )
	{
		reload_self( cfg );
	}
	
	void reload_self( cfgsection_t & cfg )
	{
		minavg = cfg.entry<uint>( "minavg", 0 );
		grace = cfg.entry<uint>( "grace", 1 );
	}
	
	void reload( cfgsection_t & cfg )
	{
		reload_self( cfg );
		obis::power_meter_t::reload( cfg );
	}
	
	virtual void calc( obis::readings_t const& readings )
	{
		if( readings[0].direct_power_valid )
		{
			timestamp_min = readings[0].timestamp;
			timestamp = readings[0].timestamp;
			pwr = readings[0].direct_power * pwr_milli;
			pwr_error = 1 * pwr_milli;
		} else {
			int n_avg = readings[0].calculated_power;
			uint n = d_min( minavg + 1, readings.num() );
			for( uint i = minavg; i < d_min( d_max( minavg + 1, last_n + 1 ), readings.num() / 2 ); i++ )
			{
				uint i_n = i + 1;
				int i_n_avg_0 = 0;
				int i_n_avg_1 = 0;
				for( uint k = 0; k < i_n; k++ )
					i_n_avg_0 += readings[k].calculated_power;
				for( uint k = i_n; k < 2 * i_n; k++ )
					i_n_avg_1 += readings[k].calculated_power;
				if( i_n_avg_1 > i_n_avg_0 + 1 + grace || i_n_avg_0 > i_n_avg_1 + 1 + grace )
					break;
				n_avg = i_n_avg_0;
				n = i_n;
			}
			last_n = n;
			
			timestamp_min = readings[n-1].timestamp;
			timestamp = readings[0].timestamp;
			pwr = n_avg * sec_t<int>::hour * pwr_milli / ( 10 * n );
			pwr_error = 1 * sec_t<int>::hour * pwr_milli / ( 10 * n );
		}
		energy_forward = readings[0].energy_import * ( energy_milli / 10ull );
		energy_reverse = readings[0].energy_export * ( energy_milli / 10ull );
		voltage1 = readings[0].voltage1 * ( volt_milli / 10 );
		voltage2 = readings[0].voltage2 * ( volt_milli / 10 );
		voltage3 = readings[0].voltage3 * ( volt_milli / 10 );
		frequency = readings[0].frequency * ( freq_milli / 10 );
		apply_offset();
	}
};


#endif // EFR_INCLUDED

