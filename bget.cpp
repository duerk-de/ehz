#ifndef BGET_INCLUDED
#define BGET_INCLUDED

#include "request_response.cpp"
#include "modbus.cpp"
#include "data.cpp"
#include "cfg.cpp"

struct bget_t
{
	struct inst_data_t
	{
		uint volt;
		uint current;
		uint active_power;
		uint apparent_power;
		uint reactive_power;
		ushort frequency;
		ushort power_factor;
	} __attribute__((packed));
	
	struct total_data_t
	{
		uint total;
		uint t1;
		uint t2;
		uint t3;
		uint t4;
	} __attribute__((packed));
	
	constexpr static ushort data_beg = 0x100;
	struct data_t
	{
		inst_data_t inst;
		uint reserved;
		total_data_t forward_active;
		total_data_t reverse_active;
		total_data_t total_active;
		total_data_t forward_reactive;
		total_data_t reverse_reactive;
		total_data_t total_reactive;
	} __attribute__((packed));

	struct power_meter_t : public ::power_meter_t
	{
		power_meter_t( cfgsection_t & cfg ) : ::power_meter_t( cfg )
		{ }
		
		void calc( ulint _timestamp, data_t const& readings )
		{
			timestamp_min = _timestamp;
			timestamp = _timestamp;
			pwr = (int)fe( readings.inst.active_power ) * pwr_milli;
			pwr_error = 1 * pwr_milli;
			energy_forward = (ulint)(int)fe( readings.forward_active.total ) * ( 10ull * energy_milli );
			energy_reverse = (ulint)(int)fe( readings.reverse_active.total ) * ( 10ull * energy_milli );
			voltage1 = fe( readings.inst.volt );
			voltage2 = fe( readings.inst.volt );
			voltage3 = fe( readings.inst.volt );
			frequency = fe( readings.inst.frequency ) * ( freq_milli / 10 );
			apply_offset();
		}
	};
	
	
	rawbuf_t & rawbuf;
	power_meter_t & meter;

	modbus_crc_t crc;
	modbus_read_t<data_beg, data_t> reader;

	template <typename t>
	static constexpr t fe( t v )
	{
		return( flip_endianess( v ) );
	}
	
	bget_t( cfgsection_t & cfg, rawbuf_t & _rawbuf, power_meter_t & _meter,
	        cstr_t _name ) :
		rawbuf( _rawbuf ),
		meter( _meter ),
		reader( crc, data_t(), _name ) // fixme: calling deprecated constructor: no initializing of data
	{
		reload( cfg );
	}

	void reload( cfgsection_t & cfg )
	{
		reader.set_address( cfg.entry<byte>( "address", 1 ) );
	}

	bool process( ulint timestamp )
	{
		if( !reader.is_response_valid() )
		{
			d_fprint_err( "bget response error\n" );
			return( false );
		}
		meter.calc( timestamp, reader.data() );

		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
						       "> %S\n"
						       "voltage         : %9,6i mV\n"
						       "current         : %9,5i mA\n"
						       "active_power    : %9,5i W\n"
						       "reactive_power  : %9,5i var\n"
						       "power_factor    : %9,4i\n"
						       "frequency       : %9,3i dHz\n"
						       "forward_active  : %9,8i daWh\n"
						       "reverse_active  : %9,8i daWh\n"
						       "forward_reactive: %9,8i daWh\n"
						       "reverse_reactive: %9,8i daWh\n",
						       reader.name,
		                       (int)fe( reader.data().inst.volt ),
		                       (int)fe( reader.data().inst.current ),
		                       (int)fe( reader.data().inst.active_power ),
		                       (int)fe( reader.data().inst.reactive_power ),
		                       (int)fe( reader.data().inst.power_factor ),
		                       (int)fe( reader.data().inst.frequency ),
		                       (int)fe( reader.data().forward_active.total ),
		                       (int)fe( reader.data().reverse_active.total ),
		                       (int)fe( reader.data().forward_reactive.total ),
		                       (int)fe( reader.data().reverse_reactive.total ) );
		return( true );
	}
};

#endif // BGET_INCLUDED

