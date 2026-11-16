#ifndef MARS_INCLUDED
#define MARS_INCLUDED

#include "request_response.cpp"
#include "io.cpp"
#include "request_reader.cpp"
#include "ringbuf.cpp"
#include "math.cpp"
#include "modbus.cpp"
#include "signals.cpp"
#include "statistics.cpp"
#include "data.cpp"
#include "cfg.cpp"

/*
	local baud=115200
	local par=none
	local addr=1
	local sett="-b $baud -p $par -m rtu -a $addr -1 -0 -o 0.1"
	local dev=/dev/serial/by-path/platform-3f980000.usb-usb-0:1.4.2:1.0-port0
*/

struct mars_t
{
	template <typename t>
	static constexpr t fe( t v )
	{
		return( flip_endianess( v ) );
	}
	
	template <typename t>
	static constexpr std::make_signed<t>::type fes( t v )
	{
		return( static_cast<std::make_signed<t>::type>( flip_endianess( v ) ) );
	}
	
	#include "mars_defs.cxx"
	
	// static constexpr ulint volt_milli = 1000ull;
	static constexpr ulint volt_micro = 1000000ull;

	struct data_t
	{
		struct balance_t
		{
			enum state_t { none, charging, discharging, finished };
			state_t state;
			uint cell_voltage_max;
			uint cell_voltage;
			uint cell_voltage_min;
			constexpr balance_t() :
				state( none ),
				cell_voltage_max( 0 ),
				cell_voltage( 0 ),
				cell_voltage_min( 0 )
			{ }
			void init()
			{
				state = none;
				cell_voltage_max = 0;
				cell_voltage = 0;
				cell_voltage_min = 0;
			}
		};

		ulint timestamp;
		int offset;
		int external;
		int out;
		int error;
		int consumption;
		uint gen_voltage;
		lint energy;
		int setpoint;
		bool limited;
		int full_power;
		int full_power_offset;
		int full_power_error;
		uint balance_num_import;
		balance_t balance;
		int empty_power;
		int low_power;
		int pack_power;
		uint pack_voltage;
		int pack_current;
		uint cell_voltage;
		uint cell_voltage_max;
		uint cell_voltage_min;
		uint cell_voltage_diff;
		uint cell_voltages_num_constant;
		int temp;
		int temp_diff;
		uint soc;
		uint level;
		int voltlevel;
		int voltlevel_low;
		int voltlevel_current;
		int powerdiff;
		int energy_forward;
		int energy_reverse;
		int solar_max;
		
		constexpr data_t() : 
			timestamp( 0 ),
			offset( 0 ),
			external( 0 ),
			out( 0 ),
			error( 0 ),
			consumption( 0 ),
			gen_voltage( 0 ),
			energy( 0 ),
			setpoint( 0 ),
			limited( false ),
			full_power( 0 ),
			full_power_offset( 0 ),
			full_power_error( 0 ),
			balance_num_import( 0 ),
			empty_power( 0 ),
			low_power( 0 ),
			pack_power( 0 ),
			pack_voltage( 0 ),
			pack_current( 0 ),
			cell_voltage( 0 ),
			cell_voltage_max( 0 ),
			cell_voltage_min( 0 ),
			cell_voltage_diff( 0 ),
			cell_voltages_num_constant( 0 ),
			temp( 0 ),
			temp_diff( 0 ),
			soc( 0 ),
			level( 0 ),
			voltlevel( 0 ),
			voltlevel_low( 0 ),
			voltlevel_current( 0 ),
			powerdiff( 0 ),
			energy_forward( 0 ),
			energy_reverse( 0 ),
			solar_max( 0 )
		{ }
	};
	
	struct stat_t
	{
		static constexpr ulint microvolt_per_millivolt = 1000ull;
		
		template <typename t>
		struct sorted_t : public not_copyable_t
		{
			t * val;
			uint * is;
			uint maxnum;
			uint num;
			
			constexpr sorted_t() :
				val( nullptr ),
				is( nullptr ),
				maxnum( 0 ),
				num( 0 )
			{ }
			
			void setup( uint _maxnum )
			{
				num = 0;
				if( _maxnum > maxnum )
				{
					delete [] val;
					delete [] is;
					val = new t[_maxnum];
					is = new uint[_maxnum];
					ENSURE( val && is, "stat malloc error\n" );
					maxnum = _maxnum;
				}
			}
			
			~sorted_t()
			{
				delete [] val;
				delete [] is;
			}
		};
		
		int consumption;
		sorted_t<int> sorted_setpoint_error;
		uint maximum_negative_setpoint_error;
		uint negative_variance;
		ulint cell_voltage;
		ulint cell_voltage_max;
		ulint cell_voltage_min;
		
		constexpr stat_t() :
			consumption( 0 ),
			maximum_negative_setpoint_error( 0 ),
			negative_variance( 0 ),
			cell_voltage( 0 ),
			cell_voltage_max( 0 ),
			cell_voltage_min( 0 )
		{ }
		
		void calc( ringbuf_t<data_t> const& datas, uint num, uint numignore )
		{
			if( num > datas.num() )
				num = datas.num();
			lint sumconsumption = 0;
			for( uint i = 0; i < num; i++ )
				sumconsumption += datas[i].consumption;
			consumption = sumconsumption / num;

			sorted_setpoint_error.setup( num );
			for( uint i = 0; i < num; i++ )
			{
				if( datas[i].limited )
					continue;
				sorted_setpoint_error.val[sorted_setpoint_error.num]
					= datas[i].error;
				sorted_setpoint_error.is[sorted_setpoint_error.num] = i;
				sorted_setpoint_error.num++;
			}
			bsort( sorted_setpoint_error.val, sorted_setpoint_error.is,
			       sorted_setpoint_error.num );
			
			uint numneg = 0;
			for( ; numneg < sorted_setpoint_error.num; numneg++ )
				if( sorted_setpoint_error.val[numneg] > 0 )
					break;
			
			if( numignore && numignore < numneg )
			{
				maximum_negative_setpoint_error = - sorted_setpoint_error.val[numignore];
				ulint sqpsum = 0;
				for( uint i = numignore; i < numneg; i++ )
					sqpsum += d_sq<ulint>( - sorted_setpoint_error.val[i] );
				ulint sqpvar = sqpsum / ( numneg - numignore );
				negative_variance = math::sqrt_heron( sqpvar,
				                                      negative_variance
				                                      ? (ulint)negative_variance
				                                      : sqpvar );
			} else {
				maximum_negative_setpoint_error = 0;
				negative_variance = 0;
			}

			cell_voltage = 0;
			cell_voltage_max = 0;
			cell_voltage_min = 0;
			for( uint i = 0; i < num; i++ )
			{
				cell_voltage += datas[i].cell_voltage;
				cell_voltage_max += datas[i].cell_voltage_max;
				cell_voltage_min += datas[i].cell_voltage_min;
			}
			cell_voltage = cell_voltage / num;
			cell_voltage_max = cell_voltage_max / num;
			cell_voltage_min = cell_voltage_min / num;
		}
	};
	
	template <typename t, uint num>
	t map( t const (&arr)[num][2], t val )
	{
		static constexpr uint in = 0;
		static constexpr uint out = 1;
		uint i = 0;
		for( ; i < num; i++ )
			if( val < arr[i][in] )
				break;
		if( i == 0 )
			return( arr[i][out] );
		if( i == num )
			return( arr[num-1][out] );
		return( ( arr[i][out] - arr[i-1][out] )
		      * ( val - arr[i-1][in] ) / ( arr[i][in] - arr[i-1][in] )
		      + arr[i-1][out] );
	}
	
	struct state_t
	{
		enum states_t { stopped, empty, normal, charge, discharge, balancing, full };
		static constexpr char state_names[][4] = { "stp", "emp", "nor",
			                                       "chg", "dis", "bal", "ful" };
		states_t oldstate;
		states_t state;
		bool statechanged;
		ulint oldtime;
		ulint time;
		lint oldenergy;
		lint energy;
		int oldenergyforward;
		int energyforward;
		int oldenergyreverse;
		int energyreverse;
		uint voltage_max;
		uint voltage;
		uint voltage_min;
		constexpr state_t() :
			oldstate( stopped ),
			state( normal ),
			statechanged( false ),
			oldtime( 0 ),
			time( 0 ),
			oldenergy( 0 ),
			energy( 0 ),
			oldenergyforward( 0 ),
			energyforward( 0 ),
			oldenergyreverse( 0 ),
			energyreverse( 0 ),
			voltage_max( 0 ),
			voltage( 0 ),
			voltage_min( 0 )
		{ }
		void set( ulint _time, data_t const& data, stat_t const& stat, states_t newstate )
		{
			if( newstate == state )
			{
				statechanged = false;
				return;
			}
			statechanged = true;
			oldstate = state;
			oldtime = time ? time : _time;
			state = newstate;
			time = _time;
			oldenergy = energy;
			energy = data.energy;
			oldenergyforward = energyforward;
			energyforward = data.energy_forward;
			oldenergyreverse = energyreverse;
			energyreverse = data.energy_reverse;
			bool wasbalancing = oldstate == balancing;
			voltage_max = wasbalancing ? data.balance.cell_voltage_max : stat.cell_voltage_max;
			voltage = wasbalancing ? data.balance.cell_voltage : stat.cell_voltage;
			voltage_min = wasbalancing ? data.balance.cell_voltage_min : stat.cell_voltage_min;
		}
		states_t get() const
		{
			return( state );
		}
		bool is( states_t _state ) const
		{
			return( state == _state );
		}
		bool changed() const
		{
			return( statechanged );
		}
		void print() const
		{
			if( changed() )
			{
				d_fprint( "state: %S -> %S"
						  ": %4t"
						  ", volts: %6f..%6f..%6f"
						  ", deltas: %6f %6f"
						  ", energy: %6F %2f %2f\n",
						  cstr_t( state_names[oldstate] ),
						  cstr_t( state_names[state] ),
						  (double)( time - oldtime ) / (double)nsec_t<ulint>::sec,
						  voltage_max,
						  voltage,
						  voltage_min,
						  voltage_max - voltage,
						  voltage_max - voltage_min,
						  energy - oldenergy,
						  energyforward - oldenergyforward,
						  energyreverse - oldenergyreverse );
			}
		}
	};

	signals_t::data_t & signals;
	rawbuf_t & rawbuf;
	power_meter_t const& solar_meter;
	power_meter_t const& external_meter;
	power_meter_t const& generator_meter;
	statistics_t const& statistics;
	modbus_crc_t crc;
	
	data_t data;
	ringbuf_t<data_t> datas;
	stat_t stat;
	stat_t fstat;
	uint cerrcnt;
	state_t state;
	
	main_t main;
	pack_t pack;
	ac_t ac;
	offgrid_t offgrid;
	meters_t meters;
	cell_t cell;
	imode_t imode;
	ctrl_offgrid_t ctrl_offgrid;
	ctrl_enable_t ctrl_enable;
	ctrl_mode_t ctrl_mode;
	ctrl_amount_t ctrl_amount;
	
	bool cap_with_solar_max;
	uint generator_max_ampere;
	bool zero_during_crossing;
	lint attenuation;
	uint timelag;
	uint statlen;
	uint fstatlen;
	uint statignore;
	uint statmult;
	int full_rate;
	int full_rate_integral;
	uint full_volt;
	uint full_volt_idle;
	uint full_volt_difference;
	uint balance_volt_difference;
	uint balance_volt_high;
	uint balance_volt_low;
	int balance_power_charge;
	int balance_power_discharge;
	int full_power_min;
	int full_power;
	int full_power_init;
	bool balance_with_backup;
	int balance_power_import_max;
	uint balance_num_import_max;
	bool balance_force;
	bool balance_enabled;
	int empty_rate;
	int low_night_rate;
	int low_solar_rate;
	uint empty_volt;
	uint low_night_volt;
	uint low_solar_volt;
	uint center_volt;
	int empty_power;
	bool charge_force;
	int charge_power;
	bool discharge_force;
	int discharge_power;
	int charge_power_max;
	int discharge_power_max;
	int discharge_power_min;
	uint nominal_capacity;
	
	mars_t( cfgsection_t & cfg, io_t & io, signals_t::data_t & _signals,
	        rawbuf_t & _rawbuf,
	        power_meter_t const& _solar_meter,
	        power_meter_t const& _external_meter,
	        power_meter_t const& _generator_meter,
	        statistics_t const& _statistics ) :
		signals( _signals ),
		rawbuf( _rawbuf ),
		solar_meter( _solar_meter ),
		external_meter( _external_meter ),
		generator_meter( _generator_meter ),
		statistics( _statistics ),
		cerrcnt( 0 ),
		main( io, crc ),
		pack( io, crc ),
		ac( io, crc ),
		offgrid( io, crc ),
		meters( io, crc ),
		cell( io, crc ),
		imode( io, crc ),
		ctrl_offgrid( io, crc ),
		ctrl_enable( io, crc ),
		ctrl_mode( io, crc ),
		ctrl_amount( io, crc ),
		charge_force( false ),
		discharge_force( false )
	{
		reload( cfg );
	}
	
	void reload( cfgsection_t & cfg )
	{
		byte address = cfg.entry( "address", 1 );
		main.data.set_address( address );
		pack.data.set_address( address );
		ac.data.set_address( address );
		offgrid.data.set_address( address );
		meters.data.set_address( address );
		cell.data.set_address( address );
		imode.data.set_address( address );
		ctrl_offgrid.set.set_address( address );
		ctrl_enable.set.set_address( address );
		ctrl_mode.set.set_address( address );
		ctrl_amount.set.set_address( address );
		
		cap_with_solar_max = cfg.entry( "cap_with_solar_max", true );
		generator_max_ampere = d_max( 0, cfg.entry( "generator_max_ampere", 1 ) );
		zero_during_crossing = cfg.entry( "zero_during_crossing", true );
		attenuation = (lint)( cfg.entry<double>( "attenuation", 1.0 ) * 1e3 );
		timelag = cfg.entry( "timelag", 0u );
		statlen = cfg.entry( "statlen", 16u );
		fstatlen = cfg.entry( "fstatlen", statlen / 2 );
		if( d_max( statlen, fstatlen ) != datas.maxnum() )
			datas.resize_truncfirst( d_max( statlen, fstatlen ) );
		statignore = (uint)( cfg.entry<double>( "statignore", 1e-2 ) * 1e3 );
		statmult = (uint)( cfg.entry<double>( "statmult", 3.0 ) * 1e3 );

		full_rate = cfg.entry( "full_rate", 1 );
		full_rate_integral = cfg.entry( "full_rate_integral", 1 );
		full_volt = (uint)( cfg.entry<double>( "full_volt", 3.6 ) * (double)volt_micro );
		full_volt_idle = (uint)( cfg.entry<double>( "full_volt_idle", 3.45 ) * (double)volt_micro );
		full_volt_difference = (uint)( cfg.entry<double>( "full_volt_difference", 0.01 ) * (double)volt_micro );
		balance_volt_difference = (uint)( cfg.entry<double>( "balance_volt_difference", 0.1 ) * (double)volt_micro );
		balance_volt_high = (uint)( cfg.entry<double>( "balance_volt_high", 3.575 ) * (double)volt_micro );
		balance_volt_low = (uint)( cfg.entry<double>( "balance_volt_low", 3.475 ) * (double)volt_micro );
		balance_power_charge = -(int)( cfg.entry<double>( "balance_power_charge", 100.0 ) * (double)power_meter_t::pwr_milli );
		balance_power_discharge = (int)( cfg.entry<double>( "balance_power_discharge", 100.0 ) * (double)power_meter_t::pwr_milli );
		full_power_min = -(int)( cfg.entry<double>( "full_power_min", 20.0 ) * (double)power_meter_t::pwr_milli );
		full_power = -(int)( cfg.entry<double>( "full_power", 500.0 ) * (double)power_meter_t::pwr_milli );
		full_power_init = -(int)( cfg.entry<double>( "full_power_init", 50.0 ) * (double)power_meter_t::pwr_milli );
		balance_with_backup = cfg.entry( "balance_with_backup", false );
		balance_power_import_max = (int)( cfg.entry<double>( "balance_power_import_max", 0 ) * (double)power_meter_t::pwr_milli );
		balance_num_import_max = cfg.entry( "balance_num_import_max", 3 );
		balance_force = cfg.entry( "balance_force", false );
		balance_enabled = cfg.entry( "balance_enabled", false );
		
		empty_rate = cfg.entry( "empty_rate", 1 );
		empty_volt = (uint)( cfg.entry<double>( "empty_volt", 3.1 ) * (double)volt_micro );
		empty_power = (int)( cfg.entry<double>( "empty_power", 500.0 ) * (double)power_meter_t::pwr_milli );
		low_night_rate = cfg.entry( "low_night_rate", 1 );
		low_night_volt = (uint)( cfg.entry<double>( "low_night_volt", 3.15 ) * (double)volt_micro );
		low_solar_rate = cfg.entry( "low_solar_rate", 1 );
		low_solar_volt = (uint)( cfg.entry<double>( "low_solar_volt", 3.15 ) * (double)volt_micro );
		center_volt = (uint)( cfg.entry<double>( "center_volt", 3.31 ) * (double)volt_micro );

		charge_power = -(int)( cfg.entry<double>( "charge_power", 0.0 ) * (double)power_meter_t::pwr_milli );
		discharge_power = (int)( cfg.entry<double>( "discharge_power", 0.0 ) * (double)power_meter_t::pwr_milli );

		charge_power_max = -(int)( cfg.entry<double>( "charge_power_max", 200.0 ) * (double)power_meter_t::pwr_milli );
		discharge_power_max = (int)( cfg.entry<double>( "discharge_power_max", 200.0 ) * (double)power_meter_t::pwr_milli );
		discharge_power_min = (int)( cfg.entry<double>( "discharge_power_min", 0.0 ) * (double)power_meter_t::pwr_milli );
		nominal_capacity = (uint)( cfg.entry<double>( "nominal_capacity", 1000.0 ) * (double)power_meter_t::pwr_milli );
	}
	
	bool init()
	{
		if( !set( 0 ) )
		{
			d_fprint_err( "initial set error\n" );
			return( false );
		}
		ctrl_enable.set.data().enabled = ctrl_enable_t::data_t::on;
		if( !ctrl_enable.write() )
		{
			d_fprint_err( "ctrl_enable.writer error\n" );
			return( false );
		}
		ctrl_offgrid.set.data().enabled = ctrl_offgrid_t::data_t::off;
		if( !ctrl_offgrid.write() )
		{
			d_fprint_err( "ctrl_offgrid.writer error\n" );
			return( false );
		}
		return( true );
	}

	bool set( int power )
	{
		ctrl_mode_t::data_t::state_t oldstate = ctrl_mode.set.data().state;
		if( power > 0 )
		{
			ctrl_amount.set.data() = { .charge = fe<ushort>( 0 ),
				                       .discharge = fe<ushort>( power / power_meter_t::pwr_milli ) };
			ctrl_mode.set.data().state = ctrl_mode_t::data_t::discharge;
		}
		else if( power < 0 )
		{
			ctrl_amount.set.data() = { .charge = fe<ushort>( - power / power_meter_t::pwr_milli ),
				                       .discharge = fe<ushort>( 0 ) };
			ctrl_mode.set.data().state = ctrl_mode_t::data_t::charge;
		}
		else
		{
			ctrl_amount.set.data() = { .charge = fe<ushort>( 0 ),
				                       .discharge = fe<ushort>( 0 ) };
		}
		bool okay = ctrl_amount.write();
		if( ctrl_mode.set.data().state != oldstate )
		{
			if( !ctrl_mode.write() )
			{
				okay = false;
				ctrl_mode.set.data().state = ctrl_mode_t::data_t::unkown;
			}
		}
		return( okay );
	}

	bool stop()
	{
		d_fprint( "stopping venus\n" );
		ctrl_amount.set.data() = { .charge = fe<ushort>( 0 ),
								   .discharge = fe<ushort>( 0 ) };
		bool okay = ctrl_amount.write();
		ctrl_mode.set.data().state = ctrl_mode_t::data_t::stop;
		if( !ctrl_mode.write() )
		{
			okay = false;
			ctrl_mode.set.data().state = ctrl_mode_t::data_t::unkown;
		}
		ctrl_offgrid.set.data().enabled = ctrl_offgrid_t::data_t::off;
		if( !ctrl_offgrid.write() )
		{
			ctrl_offgrid.set.data().enabled = ctrl_offgrid_t::data_t::unkown;
			okay = false;
		}
		return( okay );
	}
	
	bool run( ulint time )
	{
		if( !ac.read() )
		{
			cerrcnt++;
			return( false );
		}

		data.timestamp = time;

		data.offset = stat.negative_variance * statmult / 1000u;
		if( data.offset > stat.consumption / 2 )
			data.offset = stat.consumption / 2;
		if( stat.consumption < 0  )
			data.offset = 0;

		data.external = external_meter.pwr;
		data.out = (int)(short)fe( ac.data.data().power ) * power_meter_t::pwr_milli;
		data.error = data.external - data.offset;

		data.consumption = generator_meter.pwr + external_meter.pwr;
		data.gen_voltage = generator_meter.voltage1;
		data.energy = ( (lint)generator_meter.energy_forward - (lint)solar_meter.energy_forward )
		            + ( (lint)solar_meter.energy_reverse - (lint)generator_meter.energy_reverse );
		
		if( state.is( state.balancing ) )
		{
			if( data.out + data.error > balance_power_import_max )
				data.balance_num_import++;
			else
				data.balance_num_import = 0;
		} else {
			if( data.out + data.error > 0 )
				data.balance_num_import++;
			else
				data.balance_num_import = 0;
		}
		
		if( fstat.cell_voltage_max && stat.cell_voltage_max && data.cell_voltage_max
		 && fstat.cell_voltage_min && stat.cell_voltage_min && data.cell_voltage_min
		 && fstatlen && statlen )
		{
			bool empty = data.soc == 11900;
			bool full = data.soc == 100000;
			switch( state.get() )
			{
				case state.stopped:
					state.set( time, data, stat, state.stopped );
					break;
				case state.empty:
					if( charge_force )
						state.set( time, data, stat, state.charge );
					else if( !empty )
						state.set( time, data, stat, state.normal );
					else
						state.set( time, data, stat, state.empty );
					break;
				case state.full:
					if( discharge_force )
						state.set( time, data, stat, state.discharge );
					else if( !full )
						state.set( time, data, stat, state.normal );
					else
						state.set( time, data, stat, state.full );
					break;
				case state.charge:
					if( full )
					{
						charge_force = false;
						state.set( time, data, stat, state.full );
					}
					else if( data.cell_voltage_max >= full_volt && !full )
					{
						charge_force = false;
						state.set( time, data, stat, state.balancing );
					}
					else
					{
						if( !charge_force )
						{
							if( discharge_force )
								state.set( time, data, stat, state.discharge );
							else
								state.set( time, data, stat, state.normal );
						} else {
							state.set( time, data, stat, state.charge );
						}
					}
					break;
				case state.discharge:
					if( empty )
					{
						discharge_force = false;
						state.set( time, data, stat, state.empty );
					}
					else
					{
						if( !discharge_force )
						{
							if( charge_force )
								state.set( time, data, stat, state.charge );
							else
								state.set( time, data, stat, state.normal );
						} else {
							state.set( time, data, stat, state.discharge );
						}
					}
					break;
				case state.normal:
					if( charge_force )
					{
						state.set( time, data, stat, state.charge );
					}
					else if( discharge_force )
					{
						state.set( time, data, stat, state.discharge );
					}
					else if( full )
					{
						state.set( time, data, stat, state.full );
					}
					else if( empty )
					{
						state.set( time, data, stat, state.empty );
					}
					else if( data.cell_voltage_max >= full_volt
						  && ( balance_force
							|| data.balance_num_import <= balance_num_import_max ) )
					{
						data.balance_num_import = 0;
						state.set( time, data, stat, state.balancing );
					}
					else
					{
						state.set( time, data, stat, state.normal );
					}
					break;
				case state.balancing:
					if( !( balance_force
						|| data.balance_num_import <= balance_num_import_max ) )
					{
						if( charge_force )
						{
							state.set( time, data, stat, state.charge );
						}
						else if( discharge_force )
						{
							state.set( time, data, stat, state.discharge );
						}
						else
						{
							state.set( time, data, stat, state.normal );
						}
					} else {
						state.set( time, data, stat, state.balancing );
					}
					break;
			}
		}
		
		int charge_power_max_temp = charge_power_max;
		if( data.temp < 10000 )
		{
			uint j = 10000 - data.temp;
			int p = -(int)( nominal_capacity / d_pwr( 2u, j / 2500u ) );
			if( p > charge_power_max_temp )
				charge_power_max_temp = p;
		}

		int full_volt_err = data.cell_voltage_max
		                  ? (int)data.cell_voltage_max - (int)full_volt
		                  : 0;
		if( ( full_volt_err < 0 ? -1ll : 1ll )
		    * d_sq<lint>( full_volt_err ) * d_sq<lint>( full_rate )
		    > discharge_power_max * d_sq<lint>( 1000ll ) )
			data.full_power = discharge_power_max;
		else if( ( full_volt_err < 0 ? -1ll : 1ll )
		    * d_sq<lint>( full_volt_err ) * d_sq<lint>( full_rate )
		    < charge_power_max_temp * d_sq<lint>( 1000ll ) )
			data.full_power = charge_power_max_temp;
		else
			data.full_power = ( full_volt_err < 0 ? -1ll : 1ll )
			                * d_sq<lint>( full_volt_err ) * d_sq<lint>( full_rate )
			                / d_sq<lint>( 1000ll );
		int full_volt_idle_err = data.cell_voltage_max
		                       ? (int)data.cell_voltage_max - (int)full_volt_idle
		                       : 0;
		int full_power_idle = ( full_volt_idle_err < 0 ? -1ll : 1ll )
		                    * d_sq<lint>( full_volt_idle_err ) * d_sq<lint>( full_rate )
		                    / d_sq<lint>( 1000ll );
		int empty_volt_err = data.cell_voltage_min
			               ? (int)data.cell_voltage_min - (int)empty_volt
			               : 0;
		if( ( empty_volt_err < 0 ? -1ll : 1ll )
		    * d_sq<lint>( empty_volt_err ) * (lint)empty_rate
		    > discharge_power_max * d_sq<lint>( 1000ll ) )
			data.empty_power = discharge_power_max;
		else if( ( empty_volt_err < 0 ? -1ll : 1ll )
		    * d_sq<lint>( empty_volt_err ) * (lint)empty_rate
		    < charge_power_max_temp * d_sq<lint>( 1000ll ) )
			data.empty_power = charge_power_max_temp;
		else
			data.empty_power = ( empty_volt_err < 0 ? -1ll : 1ll )
			                 * d_sq<lint>( empty_volt_err ) * (lint)empty_rate
			                 / d_sq<lint>( 1000ll );
		int low_rate = data.solar_max == 0 ? low_night_rate : low_solar_rate;
		uint low_volt = data.solar_max == 0 ? low_night_volt : low_solar_volt;
		bool low_limit = low_volt > empty_volt;
		int low_volt_err = fstat.cell_voltage_min
		                 ? (int)fstat.cell_voltage_min - (int)low_volt
		                 : 0;
		if( ( low_volt_err < 0 ? -1ll : 1ll )
            * d_sq<lint>( low_volt_err ) * d_sq<lint>( low_rate )
		    > discharge_power_max * d_sq<lint>( 1000ll ) )
			data.low_power = discharge_power_max;
		else if( ( low_volt_err < 0 ? -1ll : 1ll )
            * d_sq<lint>( low_volt_err ) * d_sq<lint>( low_rate )
		    < charge_power_max_temp * d_sq<lint>( 1000ll ) )
			data.low_power = charge_power_max_temp;
		else
			data.low_power = ( low_volt_err < 0 ? -1ll : 1ll )
			               * d_sq<lint>( low_volt_err ) * d_sq<lint>( low_rate )
			               / d_sq<lint>( 1000ll );

		if( state.is( state.stopped ) )
		{
			data.setpoint = 0;
			data.limited = true;
		}
		else if( state.is( state.balancing ) )
		{
			if( state.changed() )
			{
				data.full_power_offset = full_power_init;
				data.full_power_error = 0;
				data.balance.init();
				data.balance.cell_voltage_max = data.cell_voltage_max;
				data.balance.cell_voltage = data.cell_voltage;
				data.balance.cell_voltage_min = data.cell_voltage_min;
				if( data.cell_voltage_max - data.cell_voltage_min < full_volt_difference )
					data.balance.state = data.balance.finished;
			}
			int power = 0;
			if( balance_enabled && data.balance.state != data.balance.finished )
			{
				switch( data.balance.state )
				{
					case data.balance.charging:
					case data.balance.discharging:
						break;
					default:
						data.balance.state = data.balance.charging;
				}
				if( data.cell_voltage_max >= balance_volt_high )
				{
					if( data.balance.state != data.balance.discharging )
					{
						data.balance.cell_voltage_max = data.cell_voltage_max;
						data.balance.cell_voltage = data.cell_voltage;
						data.balance.cell_voltage_min = data.cell_voltage_min;
						data.balance.state = data.balance.discharging;
					}
				}
				if( data.cell_voltage_max <= balance_volt_low )
				{
					if( data.balance.state != data.balance.charging )
					{
						data.balance.state = data.balance.charging;
					}
				}
				switch( data.balance.state )
				{
					case data.balance.charging:
						power = balance_power_charge;
						break;
					case data.balance.discharging:
						power = balance_power_discharge;
					default:
						break;
				}
				if( stat.cell_voltage_max - stat.cell_voltage_min
			     <= balance_volt_difference )
				{
					data.full_power_offset = 0;
					data.full_power_error = 0;
					data.balance.state = data.balance.finished;
				}
			} else {
				data.balance.cell_voltage_max = stat.cell_voltage_max;
				data.balance.cell_voltage = stat.cell_voltage;
				data.balance.cell_voltage_min = stat.cell_voltage_min;
				
				if( data.balance.state != data.balance.finished )
				{
					data.full_power_offset += (lint)full_volt_err * (lint)full_rate_integral / 1000ll;
					if( data.full_power_offset < full_power )
						data.full_power_offset = full_power;
					if( data.full_power_offset > full_power_min )
						data.full_power_offset = full_power_min;
					power = data.full_power_offset + data.full_power;
					if( power > full_power_min )
						power = full_power_min;
					if( power < full_power )
						power = full_power;
				} else {
					data.full_power_offset += (lint)full_volt_idle_err * (lint)full_rate_integral / 1000ll;
					if( data.full_power_offset < full_power )
						data.full_power_offset = full_power;
					if( data.full_power_offset > 0 )
						data.full_power_offset = 0;
					power = data.full_power_offset + full_power_idle;
					if( power > 0 )
						power = 0;
					if( power < full_power )
						power = full_power;
				}
			}
			data.limited = true;
			data.setpoint = power;
			data.full_power_error += data.setpoint % (int)power_meter_t::pwr_milli;
			if( data.full_power_error < - (int)power_meter_t::pwr_milli )
			{
				data.full_power_error -= (int)power_meter_t::pwr_milli;
				data.setpoint -= power_meter_t::pwr_milli;
			}
			if( data.full_power_error > (int)power_meter_t::pwr_milli )
			{
				data.full_power_error += (int)power_meter_t::pwr_milli;
				data.setpoint += power_meter_t::pwr_milli;
			}
		}
		else
		{
			int prev_setpoint = datas.num()
							  ? datas[0].setpoint
							  : 0;
			int prev_power_out = !timelag
			                   ? data.out
			                   : timelag-1 < datas.num()
			                   ? datas[timelag-1].out
			                   : datas.num()
			                   ? datas[datas.num()-1].out
			                   : 0;
			
			data.setpoint = prev_power_out + data.error;
			data.limited = false;
			if( attenuation )
				data.setpoint = ( data.setpoint - prev_setpoint )
							  * attenuation / 1000 + prev_setpoint;
			
			if( state.is( state.charge ) )
			{
				if( data.setpoint > charge_power )
				{
					data.setpoint = charge_power;
					data.limited = true;
				}
			}
			else if( state.is( state.discharge ) )
			{
				if( data.setpoint < discharge_power )
				{
					data.setpoint = discharge_power;
					data.limited = true;
				}
			}
			
			if( low_limit )
			{
				if( data.empty_power < 0 )
					data.empty_power = 0;
				if( data.low_power < 0 )
					data.low_power = 0;
			} else {
				if( data.empty_power < empty_power )
					data.empty_power = empty_power;
			}
			if( data.setpoint > data.empty_power )
			{
				data.setpoint = data.empty_power;
				data.limited = true;
			}
			if( low_limit )
			{
				if( data.setpoint > data.low_power )
				{
					data.setpoint = data.low_power;
					data.limited = true;
				}
			}
			
			if( data.full_power > full_power )
				data.full_power = full_power;
			if( data.setpoint < data.full_power )
			{
				data.setpoint = data.full_power;
				data.limited = true;
			}

			if( data.setpoint > 0 && data.setpoint < discharge_power_min
			 && !state.is( state.charge ) && !state.is( state.discharge ) )
			{
				data.setpoint = 0;
				data.limited = true;
			}
			
			if( state.is( state.empty ) )
			{
				if( data.setpoint > 0 )
				{
					data.limited = true;
				}
			}
			else if( state.is( state.full ) )
			{
				if( data.setpoint < 0 )
				{
					data.setpoint = 0;
					data.limited = true;
				}
			}
		}
		
		if( zero_during_crossing && datas.num() )
		{
			if ( ( datas[0].setpoint > 0 && data.setpoint < 0 ) || 
			     ( datas[0].setpoint < 0 && data.setpoint > 0 ) )
			{
				data.limited = true;
				data.setpoint = 0;
			}
		}
		
		static_assert( power_meter_t::pwr_milli == power_meter_t::volt_milli );
		int max_generator = data.gen_voltage * (int)generator_max_ampere;
		if( data.setpoint > 0 && data.setpoint + solar_meter.pwr > max_generator )
		{
			data.limited = true;
			data.setpoint = max_generator - solar_meter.pwr;
		}
		if( cap_with_solar_max )
		{
			if( data.setpoint > 0 && data.setpoint + data.solar_max > max_generator )
			{
				data.limited = true;
				data.setpoint = max_generator - data.solar_max;
			}
		}
		
		if( data.setpoint < charge_power_max_temp )
		{
			data.limited = true;
			data.setpoint = charge_power_max_temp;
		}
		if( data.setpoint > discharge_power_max )
		{
			data.limited = true;
			data.setpoint = discharge_power_max;
		}
		if( data.setpoint < charge_power_max )
		{
			data.limited = true;
			data.setpoint = charge_power_max;
		}
		if(	(
				data.setpoint < 0
			&&
				(
					imode.data.data().state != imode.data.data().charge
				||
					(int)fe( pack.data.data().charge_limit ) == 0
				)
			)
		||
			(
				data.setpoint > 0
			&&
				(
					imode.data.data().state != imode.data.data().discharge
				||
					(int)fe( pack.data.data().discharge_limit ) == 0
				)
			)
		)
		{
			data.limited = true;
		}

		if( !set( data.setpoint ) )
		{
			d_fprint_err( "set error\n" );
		}

		if( !main.read()
		 || !pack.read()
		 || !offgrid.read()
		 || !meters.read()
		 || !cell.read()
		 || !imode.read() )
		{
			cerrcnt++;
			return( false );
		}
		
		data.pack_power = -(int)(short)fe( main.data.data().power ) * 1000;
		data.pack_voltage = (uint)fe( cell.data.data().voltage ) * 10u;
		data.pack_current = data.pack_voltage ? data.pack_power * 1000 / (int)data.pack_voltage : 0;
		
		uint cell_voltage = data.pack_voltage * 1000u / 16u;
		uint cell_voltage_max = (uint)fe( cell.data.data().voltage_max ) * 1000u;
		uint cell_voltage_min = (uint)fe( cell.data.data().voltage_min ) * 1000u;

		if( cell_voltage == data.cell_voltage
		 && cell_voltage_max == data.cell_voltage_max
		 && cell_voltage_min == data.cell_voltage_min )
			data.cell_voltages_num_constant++;
		else
			data.cell_voltages_num_constant = 0;
		
		data.cell_voltage = cell_voltage;
		data.cell_voltage_max = cell_voltage_max;
		data.cell_voltage_min = cell_voltage_min;
		
		data.cell_voltage_diff = d_max( data.cell_voltage_max - data.cell_voltage,
		                                data.cell_voltage - data.cell_voltage_min );
		data.temp = ( (int)fes( cell.data.data().temp3 )
		            + (int)fes( cell.data.data().temp4 )
		            + (int)fes( cell.data.data().temp5 )
		            + (int)fes( cell.data.data().temp6 ) ) * 100 / 4;
		data.temp_diff = ( d_max( (int)fes( cell.data.data().temp3 ),
		                   d_max( (int)fes( cell.data.data().temp4 ),
		                   d_max( (int)fes( cell.data.data().temp5 ),
		                          (int)fes( cell.data.data().temp6 ) ) ) )
		                 - d_min( (int)fes( cell.data.data().temp3 ),
		                   d_min( (int)fes( cell.data.data().temp4 ),
		                   d_min( (int)fes( cell.data.data().temp5 ),
		                          (int)fes( cell.data.data().temp6 ) ) ) ) ) * 100;
		data.soc = (uint)fe( cell.data.data().soc ) * 100;
		
		static constexpr uint level_empty = 1000;
		static constexpr uint level_bal =  99000;
		static constexpr uint level_max = 100000;
		static constexpr uint level_full = 111111;
		static constexpr uint level_scale = 10;
		if( state.is( state.full ) )
			data.level = 111111;
		else if( state.is( state.balancing ) )
			data.level = 100000;
		else if( state.is( state.empty ) )
			data.level = 0;
		else
		{
			static constexpr uint mapping[][2] = {
				{  1190, 0 },
				{  1200,   level_empty          / level_scale },
				{  1300, ( level_empty + 1000 ) / level_scale },
				{  9800, ( level_bal   - 1000 ) / level_scale },
				{  9900, ( level_bal          ) / level_scale },
				{ 10000, ( level_full         ) / level_scale } };
			data.level = map( mapping, data.soc / level_scale ) * level_scale;
		}
		
		if( (int)empty_volt + empty_volt_err < (int)center_volt )
		{
			data.voltlevel = (lint)empty_volt_err
			               * (lint)( level_max / 2 )
			               / (lint)( (int)center_volt - (int)empty_volt );
			data.voltlevel_low = (lint)low_volt_err
			                   * (lint)( level_max / 2 )
			                   / (lint)( (int)center_volt - (int)low_volt );
		}
		else if( (int)full_volt_idle + full_volt_idle_err > (int)center_volt )
		{
			data.voltlevel = (lint)(-full_volt_idle_err)
			               * (lint)( level_max / 2 )
			               / (lint)( (int)full_volt_idle - (int)center_volt );
			data.voltlevel = level_max - data.voltlevel;
			data.voltlevel_low = data.voltlevel;
		}
		else
		{
			data.voltlevel = level_max / 2;
			data.voltlevel_low = data.voltlevel;
		}
		data.voltlevel_current = low_limit ? data.voltlevel_low : data.voltlevel;
		
		data.powerdiff = data.out - data.pack_power;
		data.energy_forward = (int)fe( meters.data.data().discharged );
		data.energy_reverse = (int)fe( meters.data.data().charged );

		lint consumed = 0;
		lint otime = 0;
		lint s_solar_max = 0;
		statistics.getvals( time / nsec_t<ulint>::msec - msec_t<lint>::day,
							otime, s_solar_max, consumed );
		data.solar_max = s_solar_max;
		
		ctrl_offgrid_t::data_t::enabled_t oldstate = ctrl_offgrid.set.data().enabled;
		ctrl_offgrid.set.data().enabled = state.is( state.balancing ) && balance_with_backup
		                                ? ctrl_offgrid_t::data_t::on
		                                : ctrl_offgrid_t::data_t::off;
		if( ctrl_offgrid.set.data().enabled != oldstate )
			if( !ctrl_offgrid.write() )
				ctrl_offgrid.set.data().enabled = ctrl_offgrid_t::data_t::unkown;
		
		if( datas.num() )
			state.print();
		datas.add( data_t( data ) );
		stat.calc( datas, statlen, datas.num() * statignore / 1000 );
		fstat.calc( datas, fstatlen, 0 );
		
		if( signals.usr1 )
		{
			signals.usr1 = false;
			discharge_force = !discharge_force;
			d_fprint( "discharge force signal: %2b\n", discharge_force );
		}
		if( signals.usr2 )
		{
			signals.usr2 = false;
			charge_force = !charge_force;
			d_fprint( "charge force signal: %2b\n", charge_force );
		}
		
		#include "mars_prints.cxx"

		cerrcnt = 0;
		return( true );
	}
};


struct batt_t
{
	enum measure_mode_t { off, charge, discharge };
	
	mars_t & mars;
	power_meter_t & solar;
	power_meter_t & generator;
	measure_mode_t mode;
	
	power_meter_t solar_beg;
	power_meter_t solar_end;
	power_meter_t generator_beg;
	power_meter_t generator_end;
	
	constexpr batt_t( mars_t & _mars, power_meter_t & _solar, power_meter_t & _generator ) :
		mars( _mars ),
		solar( _solar ),
		generator( _generator ),
		mode( off )
	{ }
	
	void set( [[ maybe_unused ]] measure_mode_t _mode )
	{
/*
		if( mode != off )
			d_fprint( "batt: canceling mode %i\n", mode );
		mode = _mode;
		d_fprint( "batt: starting mode %i\n", mode );
		switch( mode )
		{
			case charge:
				mars.mode = mars_t::discharge;
				break;
			case discharge:
				mars.mode = mars_t::charge;
				break;
			case off:
				break;
		}
*/
	}
	
	void run()
	{
/*
		switch( mode )
		{
			case charge:
				switch( mars.mode )
				{
					case discharge:
						if( mars.state != mars_t::empty )
							break;
						solar_beg = solar;
						generator_beg = generator;
						mars.mode = mars_t::charge;
						break;
					case charge:
						if( mars.state != mars_t::full )
							break;
						solar_end = solar;
						generator_end = generator;
						calc();
						mars.mode = mars_t::std;
						break;
					default:
						break;
				}
				break;
			case discharge:
				switch( mars.mode )
				{
					case charge:
						if( mars.state != mars_t::full )
							break;
						solar_beg = solar;
						generator_beg = generator;
						mars.mode = mars_t::discharge;
						break;
					case discharge:
						if( mars.state != mars_t::empty )
							break;
						solar_end = solar;
						generator_end = generator;
						calc();
						mars.mode = mars_t::std;
						break;
					default:
						break;
				}
				break;
			case off:
				break;
		}
*/
	}
	
	void calc()
	{
/*
		lint solar = ( solar_end.energy_forward - solar_beg.energy_forward )
		           - ( solar_end.energy_reverse - solar_beg.energy_reverse );
		lint generator = ( generator_end.energy_forward - generator_beg.energy_forward )
		               - ( generator_end.energy_reverse - generator_beg.energy_reverse );
		lint capacity = solar - generator;
		d_fprint( "capacity: %6F kWh\n", capacity );
*/
	}
};


#endif // MARS_INCLUDED

