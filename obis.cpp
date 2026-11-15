#ifndef OBIS_INCLUDED
#define OBIS_INCLUDED

#include "db_defines.cpp"
#include "db_time.cpp"
#include "sml.cpp"
#include "data.cpp"
#include "ctrl.cpp"
#include "ringbuf.cpp"

namespace obis
{
	struct reading_t
	{
		uint relative_time;
		uint energy_import;
		uint energy_export;
		int calculated_power;
		int direct_power;
		bool direct_power_valid;
		uint voltage1;
		uint voltage2;
		uint voltage3;
		uint frequency;
		ulint timestamp;
		bool valid;
	};
	
	typedef ringbuf_t<reading_t> readings_t;

	struct power_meter_t : public ::power_meter_t
	{
		power_meter_t( cfgsection_t & cfg ) : 
			::power_meter_t( cfg )
		{ }
		virtual void calc( obis::readings_t const& readings ) = 0;
	};

/*
0100620affff  1-0:98.10.255*255  
0100600100ff  1-0:96. 1.  0*255  
0100010800ff  1-0: 1. 8.  0*255  Positive active energy, total  [kWh]  A+
0100020800ff  1-0: 2. 8.  0*255  Negative active energy, total  [kWh]  A-
0100100700ff  1-0:16. 7.  0*255  Active instantaneous power positive and negative, sum  [kW]
0100200700ff  1-0:32. 7.  0*255  Instantaneous voltage in phase 1  [V]  U(L1)
0100340700ff  1-0:52. 7.  0*255  Instantaneous voltage in phase 2  [V]  U(L2)
0100480700ff  1-0:72. 7.  0*255  Instantaneous voltage in phase 3  [V]  U(L3)
01001f0700ff  1-0:31. 7.  0*255  Instantaneous current in phase 1  [A]  I(L1)
0100330700ff  1-0:51. 7.  0*255  Instantaneous current in phase 2  [A]  I(L2)
0100470700ff  1-0:71. 7.  0*255  Instantaneous current in phase 3  [A]  I(L3)
0100510701ff  1-0:81. 7.  1*255  
0100510702ff  1-0:81. 7.  2*255  
0100510704ff  1-0:81. 7.  4*255  
010051070fff  1-0:81. 7. 15*255  
010051071aff  1-0:81. 7. 26*255  
01000e0700ff  1-0:14. 7.  0*255  Frequency of signal in the installation  Hz  f
010000020000  1-0: 0. 2.  0*  0  Firmware version
0100605a0201  1-0: 6.90.  2*  1  
010061610000  1-0: 6.97.  0*  0  
010060320104  1-0: 6.50.  1*  4  
010060320404  1-0: 6.50.  3*  4  
*/

	static const byte name_fnc_energy_p[] = { 1, 0, 1, 8, 0, 0xff };
	static const byte name_fnc_energy_n[] = { 1, 0, 2, 8, 0, 0xff };
	static const byte name_fnc_pwr[] = { 1, 0, 16, 7, 0, 0xff };
	static const byte name_fnc_volt1[] = { 1, 0, 32, 7, 0, 0xff };
	static const byte name_fnc_volt2[] = { 1, 0, 52, 7, 0, 0xff };
	static const byte name_fnc_volt3[] = { 1, 0, 72, 7, 0, 0xff };
	static const byte name_fnc_curr1[] = { 1, 0, 31, 7, 0, 0xff };
	static const byte name_fnc_curr2[] = { 1, 0, 51, 7, 0, 0xff };
	static const byte name_fnc_curr3[] = { 1, 0, 71, 7, 0, 0xff };
	static const byte name_fnc_freq[] = { 1, 0, 14, 7, 0, 0xff };
	static const uint name_val_time[] = { 2, 1 };
	static const uint name_val_count[] = { 5 };
	
	struct parsed_t : public sml::parsed_t
	{
		enum { fnc_import, fnc_export, fnc_pwr,
			   fnc_volt1, fnc_volt2, fnc_volt3,
			   fnc_curr1, fnc_curr2, fnc_curr3,
			   fnc_freq,
			   fnc_max = fnc_freq };
		enum { val_time, val_amount, val_max = val_amount };
		
		sml::val_t arr_vals_fnc_energy_p[val_max+1];
		sml::val_t arr_vals_fnc_energy_n[val_max+1];
		sml::val_t arr_vals_fnc_pwr[1];
		sml::val_t arr_vals_fnc_volt1[1];
		sml::val_t arr_vals_fnc_volt2[1];
		sml::val_t arr_vals_fnc_volt3[1];
		sml::val_t arr_vals_fnc_curr1[1];
		sml::val_t arr_vals_fnc_curr2[1];
		sml::val_t arr_vals_fnc_curr3[1];
		sml::val_t arr_vals_fnc_freq[1];
		
		sml::fnc_t arr_fncs[fnc_max+1];
		
		rawbuf_t & rawbuf;
		power_meter_t & total_meter;
		bool ispart;

		static constexpr uint buflen_secs = 5 * sec_t<uint>::min;
		readings_t total_readings;

		parsed_t( rawbuf_t & _rawbuf, power_meter_t & _total_meter, bool _ispart ) :
			arr_vals_fnc_energy_p{ [val_time]   = { .name{ name_val_time  },
				                                    .parsed{} },
				                   [val_amount] = { .name{ name_val_count },
								                    .parsed{} } },
			arr_vals_fnc_energy_n{ [val_time]   = { .name{ name_val_time  }, 
				                                    .parsed{} },
				                   [val_amount] = { .name{ name_val_count }, 
								                    .parsed{} } },
			arr_vals_fnc_pwr  { [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_volt1{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_volt2{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_volt3{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_curr1{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_curr2{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_curr3{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_vals_fnc_freq{ [0] = { .name{ name_val_count }, .parsed{} } },
			arr_fncs{ [fnc_import] = { .name{ name_fnc_energy_p },
				                       .vals{ arr_vals_fnc_energy_p } },
			          [fnc_export] = { .name{ name_fnc_energy_n },
						               .vals{ arr_vals_fnc_energy_n } },
			          [fnc_pwr] = { .name{ name_fnc_pwr },
						            .vals{ arr_vals_fnc_pwr } },
			          [fnc_volt1] = { .name{ name_fnc_volt1 },
						              .vals{ arr_vals_fnc_volt1 } },
			          [fnc_volt2] = { .name{ name_fnc_volt2 },    
						              .vals{ arr_vals_fnc_volt2 } },
			          [fnc_volt3] = { .name{ name_fnc_volt3 },    
						              .vals{ arr_vals_fnc_volt3 } },
			          [fnc_curr1] = { .name{ name_fnc_curr1 },    
						              .vals{ arr_vals_fnc_curr1 } },
			          [fnc_curr2] = { .name{ name_fnc_curr2 },    
						              .vals{ arr_vals_fnc_curr2 } },
			          [fnc_curr3] = { .name{ name_fnc_curr3 },
						              .vals{ arr_vals_fnc_curr3 } },
			          [fnc_freq] = { .name{ name_fnc_freq },
						             .vals{ arr_vals_fnc_freq } } },
			rawbuf( _rawbuf ),
			total_meter( _total_meter ),
			ispart( _ispart ),
			total_readings( buflen_secs * 2 )
		{
			fncs = { arr_fncs };
		}
		
		void add_reading( sml::fnc_t const& fnc_import, sml::fnc_t const& fnc_export,
		                  sml::fnc_t const& fnc_power, sml::fnc_t const& fnc_volt1,
		                  sml::fnc_t const& fnc_volt2, sml::fnc_t const& fnc_volt3,
		                  sml::fnc_t const& fnc_freq,
		                  ulint timestamp, readings_t & readings )
		{
			reading_t reading = {
				.relative_time = (uint)( ( fnc_import.vals[val_time].parsed.value
				                         + fnc_export.vals[val_time].parsed.value ) / 2 ),
				.energy_import = (uint)( fnc_import.vals[val_amount].parsed.value ),
				.energy_export = (uint)( fnc_export.vals[val_amount].parsed.value ),
				.calculated_power = 0,
				.direct_power = fnc_power.vals[0].parsed.found ? (int)fnc_power.vals[0].parsed.value : 0,
				.direct_power_valid = fnc_power.vals[0].parsed.found,
				.voltage1 = (uint)fnc_volt1.vals[0].parsed.value,
				.voltage2 = (uint)fnc_volt2.vals[0].parsed.value,
				.voltage3 = (uint)fnc_volt3.vals[0].parsed.value,
				.frequency = (uint)fnc_freq.vals[0].parsed.value,
				.timestamp = timestamp,
				.valid = false
			};
			// todo: check if everything was found, otherwise return error
			
			if( !readings.num() )
			{
				readings.add( reading_t( reading ) );
				return;
			}
			
			if( reading.relative_time == readings[0].relative_time )
			{
				// todo: return error
				d_fprint_err( "invalid data\n" );
				return;
			}
			reading.calculated_power = ( (int)( reading.energy_import - readings[0].energy_import )
			                           - (int)( reading.energy_export - readings[0].energy_export ) )
			                           / (int)( reading.relative_time - readings[0].relative_time );
			reading.valid = true;
			
			if( !readings[0].valid )
				readings[0] = reading;
			else
				readings.add( reading_t( reading ) );
		}
		
		virtual bool evaluate( ulint timestamp )
		{
			if( !ispart )
				rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
				                       "> meter external\n"
				                       "active_power    : %9,1I W\n" 
				                       "voltage_1       : %9,4I dV\n" 
				                       "voltage_2       : %9,4I dV\n" 
				                       "voltage_3       : %9,4I dV\n" 
				                       "current_1       : %9,1I cA\n"
				                       "current_2       : %9,1I cA\n"
				                       "current_3       : %9,1I cA\n"
				                       "frequency       : %9,1I dHz\n"
				                       "forward_active  : %9,8I dWh\n"
				                       "reverse_active  : %9,8I dWh\n",
				                       fncs[fnc_pwr].vals[0].parsed.value,
				                       fncs[fnc_volt1].vals[0].parsed.value,
				                       fncs[fnc_volt2].vals[0].parsed.value,
				                       fncs[fnc_volt3].vals[0].parsed.value,
				                       fncs[fnc_curr1].vals[0].parsed.value,
				                       fncs[fnc_curr2].vals[0].parsed.value,
				                       fncs[fnc_curr3].vals[0].parsed.value,
				                       fncs[fnc_freq].vals[0].parsed.value,
				                       fncs[fnc_import].vals[val_amount].parsed.value,
				                       fncs[fnc_export].vals[val_amount].parsed.value );
			if( !fncs[fnc_import].vals[val_time].parsed.found
			 || !fncs[fnc_import].vals[val_amount].parsed.found
			 || !fncs[fnc_export].vals[val_time].parsed.found
			 || !fncs[fnc_export].vals[val_amount].parsed.found
			 || !fncs[fnc_pwr].vals[0].parsed.found )
			{
				d_fprint_err( "fnc not found\n" );
				return( false );
			}
			add_reading( fncs[fnc_import], fncs[fnc_export], fncs[fnc_pwr],
						 fncs[fnc_volt1], fncs[fnc_volt2], fncs[fnc_volt3],
						 fncs[fnc_freq],
						 timestamp, total_readings );
			total_meter.calc( total_readings );
			return( true );
		}
	};
}

#endif // OBIS_INCLUDED

