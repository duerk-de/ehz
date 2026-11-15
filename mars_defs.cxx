#ifndef MARSDEFS_INCLUDED
#define MARSDEFS_INCLUDED


	struct main_t
	{
		constexpr static ushort id = 30000;
		struct data_t
		{
			ushort dc_voltage; // V
			ushort power; // -+W
			ushort temp_tot; // d°C
			ushort temp_mos; // d°C
			ushort ac_voltage; // V
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr main_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .dc_voltage = fe<ushort>( 0 ),
				         .power = fe<ushort>( 0 ),
				         .temp_tot = fe<ushort>( 0 ),
				         .temp_mos = fe<ushort>( 0 ),
				         .ac_voltage = fe<ushort>( 0 ) },
				  "venus main" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
/*
	struct cell_deprecated_t
	{
		constexpr static ushort id = 30100;
		struct data_t
		{
			ushort voltage; // cV
			ushort current; // A
			ushort voltage_max; // mV
			ushort voltage_min; // mV
			ushort temp_max; // c°C
			ushort temp_min; // c°C
			ushort reserved1;
			ushort reserved2;
			ushort reserved3;
			ushort reserved4;
			ushort reserved5;
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr cell_deprecated_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .voltage = fe<ushort>( 0 ),
				         .current = fe<ushort>( 0 ),
				         .voltage_max = fe<ushort>( 0 ),
				         .voltage_min = fe<ushort>( 0 ),
				         .temp_max = fe<ushort>( 0 ),
				         .temp_min = fe<ushort>( 0 ),
				         .reserved1 = fe<ushort>( 0 ),
				         .reserved2 = fe<ushort>( 0 ),
				         .reserved3 = fe<ushort>( 0 ),
				         .reserved4 = fe<ushort>( 0 ),
				         .reserved5 = fe<ushort>( 0 ) },
				  "venus cell_deprecated" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
*/
	struct pack_t
	{
		constexpr static ushort id = 32100;
		struct data_t
		{
			ushort voltage; // dV
			ushort current; // ??
			uint power; // ??
			ushort soc; // %
			ushort max_energy; // +-Wh
			ushort charge_limit; // e-3
			ushort discharge_limit; // e-3
			ushort temp3;
			ushort reserved1;
			ushort reserved2;
			ushort reserved3;
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr pack_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .voltage = fe<ushort>( 0 ),
				         .current = fe<ushort>( 0 ),
				         .power = fe<uint>( 0 ),
				         .soc = fe<ushort>( 0 ),
				         .max_energy = fe<ushort>( 0 ),
				         .charge_limit = fe<ushort>( 0 ),
				         .discharge_limit = fe<ushort>( 0 ),
				         .temp3 = fe<ushort>( 0 ),
				         .reserved1 = fe<ushort>( 0 ),
				         .reserved2 = fe<ushort>( 0 ),
				         .reserved3 = fe<ushort>( 0 ) },
				  "venus pack" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
	struct cell_t
	{
		constexpr static ushort id = 34000;
		struct data_t
		{
			ushort voltage;
			ushort current;
			ushort soc;
			ushort cycles; // charge cycle count
			ushort full; // 2 or 4 if full
			ushort voltage_max;
			ushort voltage_min;
			ushort reserved1;
			ushort reserved2;
			ushort hivolt; // 4 if one cell voltage > 3.6V until discharge
			ushort reserved3;
			ushort temp1;
			ushort temp2;
			ushort temp3;
			ushort temp4;
			ushort temp5;
			ushort temp6;
			ushort reserved4; // something if full ?
			ushort reserved5;
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr cell_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .voltage = fe<ushort>( 0 ),
			             .current = fe<ushort>( 0 ),
			             .soc = fe<ushort>( 0 ),
			             .cycles = fe<ushort>( 0 ),
			             .full = fe<ushort>( 0 ),
			             .voltage_max = fe<ushort>( 0 ),
			             .voltage_min = fe<ushort>( 0 ),
			             .reserved1 = fe<ushort>( 0 ),
			             .reserved2 = fe<ushort>( 0 ),
			             .hivolt = fe<ushort>( 0 ),
			             .reserved3 = fe<ushort>( 0 ),
			             .temp1 = fe<ushort>( 0 ),
			             .temp2 = fe<ushort>( 0 ),
			             .temp3 = fe<ushort>( 0 ),
			             .temp4 = fe<ushort>( 0 ),
			             .temp5 = fe<ushort>( 0 ),
			             .temp6 = fe<ushort>( 0 ),
			             .reserved4 = fe<ushort>( 0 ),
			             .reserved5 = fe<ushort>( 0 ) },
				  "venus cell" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
	struct ac_t
	{
		constexpr static ushort id = 32200;
		struct data_t
		{
			ushort voltage; // dV
			ushort voltage2; // dV
			uint power; // +-W
			ushort frequency; // dHz
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr ac_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .voltage = fe<ushort>( 0 ),
				         .voltage2 = fe<ushort>( 0 ),
				         .power = fe<uint>( 0 ),
				         .frequency = fe<ushort>( 0 ) },
				  "venus ac" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
	struct offgrid_t
	{
		constexpr static ushort id = 32300;
		struct data_t
		{
			ushort voltage; // dV
			ushort reserved; // cA
			ushort power; // +-W
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr offgrid_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .voltage = fe<ushort>( 0 ),
				         .reserved = fe<ushort>( 0 ),
				         .power = fe<uint>( 0 ) },
				  "venus offgrid" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
	struct meters_t
	{
		constexpr static ushort id = 33000;
		struct data_t
		{
			uint charged;
			uint discharged;
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr meters_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .charged = fe<uint>( 0 ),
				         .discharged = fe<uint>( 0 ) },
				  "venus meters" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
/*
	struct temp1_t
	{
		constexpr static ushort id = 35000;
		struct data_t
		{
			ushort tot; // +-d°
			ushort mos1; // +-d°
			ushort mos2; // +-d°
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr temp1_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .tot = fe<ushort>( 0 ),
				         .mos1 = fe<ushort>( 0 ),
				         .mos2 = fe<ushort>( 0 ) },
				  "venus temp1" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
	struct temp2_t
	{
		constexpr static ushort id = 35010;
		struct data_t
		{
			ushort batt_max; // +-d°
			ushort batt_min; // +-d°
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr temp2_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .batt_max = fe<ushort>( 0 ),
				         .batt_min = fe<ushort>( 0 ) },
				  "venus temp2" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
	struct limits_t
	{
		constexpr static ushort id = 35110;
		struct data_t
		{
			ushort voltage; // dV
			ushort charge_current; // e-3
			ushort discharge_current; // e-3
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr limits_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .voltage = fe<ushort>( 0 ),
				         .charge_current = fe<ushort>( 0 ),
				         .discharge_current = fe<ushort>( 0 ) },
				  "venus limits" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};
*/

	struct imode_t
	{
		constexpr static ushort id = 35100;
		struct data_t
		{
			enum state_t : ushort { sleep = flip_endianess<ushort>( 0 ),
				                    standby = flip_endianess<ushort>( 1 ),
				                    charge = flip_endianess<ushort>( 2 ),
				                    discharge = flip_endianess<ushort>( 3 ),
				                    backup = flip_endianess<ushort>( 4 ),
				                    ota = flip_endianess<ushort>( 5 ) };
			state_t state;
			static char const* state_str( state_t state )
			{
				switch( state )
				{
					case sleep: return( "sleep" );
					case standby: return( "standby" );
					case charge: return( "charge" );
					case discharge: return( "discharge" );
					case backup: return( "slbackupeep" );
					case ota: return( "sleotaep" );
					default: return( "invalid" );
				}
			}
		} __attribute__((packed));
		modbus_read_t<id, data_t> data;
		request_reader_t reader;
		constexpr imode_t( io_t & io, modbus_crc_t & crc ) :
			data( crc, { .state = data_t::sleep },
				  "venus imode" ),
			reader( io, data ) { }
		bool read() { return( reader.run() && data.is_response_valid() ); }
	};

	struct ctrl_offgrid_t
	{
		constexpr static ushort id = 41200;
		struct data_t
		{
			enum enabled_t : ushort { on = flip_endianess<ushort>( 0 ),
				                      off = flip_endianess<ushort>( 1 ),
				                      unkown = flip_endianess<ushort>( type_t<ushort>::max() ) };
			enabled_t enabled;
		} __attribute__((packed));
		modbus_write_t <id - modbus_t::write_offset, data_t> set;
		request_reader_t writer;
		constexpr ctrl_offgrid_t( io_t & io, modbus_crc_t & crc ) :
			set( crc, { .enabled = data_t::unkown },
				 "venus set backup" ),
			writer( io, set ) { }
		bool write() { set.make_request(); return( writer.run() && set.is_response_valid() ); }
	};

	struct ctrl_enable_t
	{
		constexpr static ushort id = 42000;
		struct data_t
		{
			enum enabled_t : ushort { off = flip_endianess<ushort>( 0x55bb ),
				                      on = flip_endianess<ushort>( 0x55aa ) };
			enabled_t enabled;
		} __attribute__((packed));
		modbus_write_t <id - modbus_t::write_offset, data_t> set;
		request_reader_t writer;
		constexpr ctrl_enable_t( io_t & io, modbus_crc_t & crc ) :
			set( crc, { .enabled = data_t::off },
				 "venus set enable" ),
			writer( io, set ) { }
		bool write() { set.make_request(); return( writer.run() && set.is_response_valid() ); }
	};

	struct ctrl_mode_t
	{
		constexpr static ushort id = 42010;
		struct data_t
		{
			enum state_t : ushort { stop = flip_endianess<ushort>( 0 ),
				                    charge = flip_endianess<ushort>( 1 ),
				                    discharge = flip_endianess<ushort>( 2 ),
				                    unkown = flip_endianess<ushort>( type_t<ushort>::max() ) };
			state_t state;
		} __attribute__((packed));
		modbus_write_t <id - modbus_t::write_offset, data_t> set;
		request_reader_t writer;
		constexpr ctrl_mode_t( io_t & io, modbus_crc_t & crc ) :
			set( crc, { .state = data_t::unkown },
				 "venus set mode" ),
			writer( io, set ) { }
		bool write() { set.make_request(); return( writer.run() && set.is_response_valid() ); }
	};

	struct ctrl_amount_t
	{
		constexpr static ushort id = 42020;
		struct data_t
		{
			ushort charge; // W
			ushort discharge; // W
		} __attribute__((packed));
		modbus_write_t <id - modbus_t::write_offset, data_t> set;
		request_reader_t writer;
		constexpr ctrl_amount_t( io_t & io, modbus_crc_t & crc ) :
			set( crc, { .charge = fe<ushort>( 0 ),
				        .discharge = fe<ushort>( 0 ) },
				 "venus set amount" ),
			writer( io, set ) { }
		bool write() { set.make_request(); return( writer.run() && set.is_response_valid() ); }
	};

#endif // MARSDEFS_INCLUDED

