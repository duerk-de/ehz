#ifndef EHZ_INCLUDED
#define EHZ_INCLUDED

#include "timing.cpp"
#include "signals.cpp"
#include "args.cpp"
#include "output.cpp"
#include "statistics.cpp"
#include "efr.cpp"
#include "obis.cpp"
#include "sml.cpp"
#include "usbttyio.cpp"
#include "rs485io.cpp"
#include "endless_reader.cpp"
#include "bget.cpp"
#include "mars.cpp"
#include "request_reader.cpp"
#include "cfg.cpp"
#include "db_time.cpp"


struct ehz_t
{
	struct cfgfile_t
	{
		cstr_t name;
		cfgfile_t( cstr_t _name ) :
			name( _name )
		{
			ENSURE( name.terminated(), "config file name not null terminated\n" );
			d_fprint( "parsing config file %S\n", name );
		}
	};

	timing_t<false, 16> timing;
	signals_t::data_t & signals;
	args_t const& args;
	cfgfile_t cfgfile;
	cfg_t<10> cfg;
	rawbuf_t rawbuf;
	efr_power_meter_t external_meter;
	efr_power_meter_t external_meter_part;
	obis::parsed_t external_parsed;
	obis::parsed_t external_parsed_part;
	sml::syncer_t sml_syncer;
	sml::parser_t external_parser;
	sml::parser_t external_parser_part;
	usbtty_io_t efr_io;
	endless_reader_t<(2*(1<<10))/sizeof(uint)> external_reader;
	rs485_io_t meters_io;
	bget_t::power_meter_t solar_meter;
	bget_t solar_bget;
	request_reader_t solar_reader;
	bget_t::power_meter_t generator_meter;
	bget_t generator_bget;
	request_reader_t generator_reader;
	rs485_io_t battery_io;
	mars_t venus;
	batt_t battery;
	power_meter_t total_meter;
	power_meter_t * meters[4];
	cstr_t meternames[lenof(meters)];
	output_t<lenof(meters)> out;
	statistics_t statistics;
	char err_exec[2048];
	uint hupdelay;
	uint hupcnt;
	static constexpr uint hupcntoff = type_t<uint>::max();
	bool use_generator;
	bool use_battery;
	uint partlen;

	calc_stats_t<6> stats;
	
	ehz_t( signals_t::data_t & _signals, args_t const& _args ) :
		signals( _signals ),
		args( _args ),
		cfgfile( args.cfg.val ? cstr_t( args.cfg.val, sizeof(unix_io_t::device) )
		                      : cstr_t( "ehz.cfg" ) ),
		cfg( cfgfile.name ),
		external_meter( cfg.section( "efr" ) ),
		external_meter_part( cfg.section( "efr" ) ),
		external_parsed( rawbuf, external_meter, false ),
		external_parsed_part( rawbuf, external_meter_part, true ),
		sml_syncer(),
		external_parser( external_parsed ),
		external_parser_part( external_parsed_part ),
		efr_io( cfg.section( "ehz" ), signals ),
		external_reader( efr_io, sml_syncer, external_parser, external_parser_part ),
		meters_io( cfg.section( "meters" ), signals ),
		solar_meter( cfg.section( "solar" ) ),
		solar_bget( cfg.section( "solar" ), rawbuf, solar_meter, "meter solar" ),
		solar_reader( meters_io, solar_bget.reader ),
		generator_meter( cfg.section( "generator" ) ),
		generator_bget( cfg.section( "generator" ), rawbuf, generator_meter, "meter generator" ),
		generator_reader( meters_io, generator_bget.reader ),
		battery_io( cfg.section( "battery" ), signals ),
		venus( cfg.section( "battery" ), battery_io, signals, rawbuf,
		       solar_meter, external_meter_part, generator_meter, statistics ),
		battery( venus, solar_meter, generator_meter ),
		meters{ &total_meter, &solar_meter, &external_meter, &generator_meter },
		meternames { "total", "solar", "external", "generator" },
		out( cfg.section( "out" ), signals, rawbuf, meters, meternames, statistics, venus ),
		statistics( cfg.section( "statistics" ), out.log1.device, rawbuf ),
		err_exec{ 0 },
		hupdelay( 0 ),
		hupcnt( hupcntoff ),
		use_generator( true ),
		use_battery( true ),
		partlen( 0 )
	{
		cfg.section( "error" ).entry( err_exec, "exec", "" );
		cfgsection_t & cfg_main = cfg.section( "main" );
		hupdelay = cfg_main.entry( "hupdelay", 5 );
		use_generator = cfg_main.entry( "generator", true );
		use_battery = cfg_main.entry( "battery", true );
		partlen = cfg_main.entry( "partlen", 0u );
		cfg.check();
		timing.reset( "constructors" );
		timing.reset( "statistics" );
		run();
	}
	
	void reload()
	{
		battery_off();
		d_fprint( "reloading config %S\n", cfgfile.name );
		cfg.reload();
		external_meter.reload( cfg.section( "efr" ) );
		solar_bget.reload( cfg.section( "solar" ) );
		solar_meter.reload( cfg.section( "solar" ) );
		generator_bget.reload( cfg.section( "generator" ) );
		generator_meter.reload( cfg.section( "generator" ) );
		venus.reload( cfg.section( "battery" ) );
		out.reload( cfg.section( "out" ) );
		statistics.reload( cfg.section( "statistics" ) );
		meters_io.teardown();
		efr_io.teardown();
		efr_io.reload( cfg.section( "ehz" ) ),
		meters_io.reload( cfg.section( "meters" ) ),
		meters_io.selfreload( cfg.section( "meters" ) ),
		efr_io.setup();
		meters_io.setup();
		battery_io.teardown();
		battery_io.reload( cfg.section( "battery" ) );
		battery_io.selfreload( cfg.section( "battery" ) );
		if( use_battery )
		{
			battery_io.setup();
			if( !venus.init() )
			{
				d_fprint_err( "venus init error\n" );
				signals.hup = true;
			}
		}
		cfg.section( "error" ).entry( err_exec, "exec", "" );
		cfgsection_t & cfg_main = cfg.section( "main" );
		hupdelay = cfg_main.entry( "hupdelay", 5 );
		use_generator = cfg_main.entry( "generator", true );
		use_battery = cfg_main.entry( "battery", true );
		partlen = cfg_main.entry( "partlen", 0u );
		cfg.check();
	}
	
	void run()
	{
		uint slowcnt = 0;
		
		efr_io.setup();
		meters_io.setup();
		if( use_battery )
		{
			battery_io.setup();
			if( !venus.init() )
			{
				d_fprint_err( "venus init error\n" );
				signals.hup = true;
			}
		}
		timing.add( "io init" );
		while( !signals.term )
		{
			timing.reset( "" );
			if( signals.hup )
			{
				signals.hup = false;
				hupcnt = hupdelay;
			}
			if( hupcnt != hupcntoff )
			{
				if( !hupcnt )
				{
					reload();
					hupcnt = hupcntoff;
				} else {
					hupcnt--;
				}
			}
			
			rawbuf.reset();
			ulint timestamp = 0;
			timing.add( "reset" );
			bool error = false;
			bool external_endfound;
			if( !read_external_part( timestamp, external_endfound ) )
			{
				error = true;
				d_fprint_err( "read_external_part\n" );
			}
			if( external_endfound )
			{
				d_fprint_err( "read_external_part external_endfound\n" );
				slowcnt++;
				if( slowcnt >= 3 )
					error = true;
			} else {
				slowcnt = 0;
			}
			timing.add( "read_external_part" );
			if( !error )
			{
				if( !process_battery( timestamp ) )
					error = true;
			} else {
				battery_off();
				slowcnt = 0;
			}
			timing.add( "process_battery" );
			if( !read_meters() )
			{
				error = true;
				battery_off();
				exec_err();
				meters_io.handle_error();
			}
			timing.add( "read_meters" );
			if( !error )
			{
				if( !parse_meters( timestamp ) )
				{
					error = true;
					battery_off();
					exec_err();
					meters_io.handle_error();
				}
				timing.add( "parse_meters" );
			}
			if( !read_external( timestamp, external_endfound ) )
				error = true;
			timing.add( "read_external" );
			if( error )
			{
				sleepus( meters_io.delay );
				continue;
			}
			total_meter.balance( generator_meter, external_meter );
			timing.add( "total_meter.balance" );
			
			statistics.add( timestamp, solar_meter.pwr,
			                generator_meter.pwr + external_meter.pwr );
			timing.add( "statistics" );
			
			out.write( timestamp );
			timing.add( "out.write" );
		}
		battery_off();
		timing.reset( "terminated" );
	}
	
	bool read_external_part( ulint & timestamp, bool & endfound )
	{
		if( external_reader.run_part( timestamp, partlen, endfound ) )
			return( true );
		d_fprint_err( "external part handle_error\n" );
		efr_io.teardown();
		exec_err();
		efr_io.setup();
		return( false );
	}
	
	bool read_external( ulint & timestamp, bool endfound )
	{
		if( external_reader.run( timestamp, endfound ) )
			return( true );
		d_fprint_err( "external handle_error\n" );
		efr_io.teardown();
		exec_err();
		efr_io.setup();
		return( false );
	}
	
	bool read_meters()
	{
		uint dts[6] = { 0 };
		solar_reader.write_request( &dts[0] );
		if( !solar_reader.read_response( &dts[1], &dts[2] ) )
			return( false );
		sleepus( meters_io.delay );
		timing.add( "read_solar" );
		if( use_generator )
		{
			// meters_io.delay() == 2ms@19200
			generator_reader.write_request( &dts[3] );
			if( !generator_reader.read_response( &dts[4], &dts[5] ) )
				return( false );
		}
		timing.add( "read_generator" );
		uint timing_print_interval = meters_io.timing_print_interval;
		if( timing_print_interval )
		{
			if( stats.slot == 0 )
				stats = calc_stats_t<lenof(stats.data)>( getnanos() / nsec_t<ulint>::sec );
			stats.add( { dts[0], dts[1], dts[2], dts[3], dts[4], dts[5] } );
			if( stats.data[0].num >= timing_print_interval )
			{
				auto s = stats.stats();
				stats = calc_stats_t<lenof(stats.data)>( getnanos() / nsec_t<ulint>::sec );
				d_fprint( "%T", (double)s.slot );
				for( uint i = 0; i < lenof( stats.data ); i++ )
					d_fprint( " %1,0F..%1,0F..%1,0F", 
							  s.data[i].min / 100, s.data[i].avg / 100, s.data[i].max / 100 );
				d_fprint( "\n" );
			}
		}
		return( true );
	}
	
	bool parse_meters( ulint const& timestamp )
	{
		if( !solar_bget.process( timestamp ) )
			return( false );
		if( !use_generator )
		{
			generator_meter = solar_meter;
			return( true );
		}
		if( !generator_bget.process( timestamp ) )
			return( false );
		return( true );
	}
	
	bool process_battery( ulint timestamp )
	{
		if( !use_generator )
			return( true );
		if( !use_battery )
			return( true );
		if( venus.run( timestamp ) )
		{
			// battery.run();
			return( true );
		}
		if( venus.cerrcnt > 1 )
			battery_off();
		// exec_err();
		battery_io.handle_error();
		return( false );
	}
	
	void battery_off()
	{
		if( !use_battery )
			return;
		venus.stop();
		// battery.set( battery.off );
	}

	void exec_err() const
	{
		uint l = d_strlen( err_exec, sizeof(err_exec) );
		if( l && l < sizeof(err_exec) )
		{
			uint ret = system( err_exec );
			if( ret )
				d_fprint_err( "error exec returned 0x%1,1,16u\n", ret );
		}
	}
};

#endif // EHZ_INCLUDED

