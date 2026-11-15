#ifndef CFG_INCLUDED
#define CFG_INCLUDED

#include "db_defines.cpp"
#include "db_consoleio.cpp"
#include "db_mem.cpp"
#include "file.cpp"
#include "calc.cpp"

namespace cfgcalc
{
	typedef calculator_t::varlist_t vars_t;
	typedef calculator_t::var_t var_t;
	
	calculator_t::varlist_t usvars()
	{
		vars_t vars;
		vars.add( var_t::from_const( usec_t<ulint>::usec, "us" ) );
		vars.add( var_t::from_const( usec_t<ulint>::usec, "usec" ) );
		vars.add( var_t::from_const( usec_t<ulint>::usec, "usecs" ) );
		vars.add( var_t::from_const( usec_t<ulint>::usec, "microseconds" ) );
		vars.add( var_t::from_const( usec_t<ulint>::msec, "ms" ) );
		vars.add( var_t::from_const( usec_t<ulint>::msec, "msec" ) );
		vars.add( var_t::from_const( usec_t<ulint>::msec, "msecs" ) );
		vars.add( var_t::from_const( usec_t<ulint>::msec, "milliseconds" ) );
		vars.add( var_t::from_const( usec_t<ulint>::sec , "s" ) );
		vars.add( var_t::from_const( usec_t<ulint>::sec , "sec" ) );
		vars.add( var_t::from_const( usec_t<ulint>::sec , "secs" ) );
		vars.add( var_t::from_const( usec_t<ulint>::sec , "seconds" ) );
		vars.add( var_t::from_const( usec_t<ulint>::min , "m" ) );
		vars.add( var_t::from_const( usec_t<ulint>::min , "min" ) );
		vars.add( var_t::from_const( usec_t<ulint>::min , "mins" ) );
		vars.add( var_t::from_const( usec_t<ulint>::min , "minutes" ) );
		vars.add( var_t::from_const( usec_t<ulint>::hour, "h" ) );
		vars.add( var_t::from_const( usec_t<ulint>::hour, "hour" ) );
		vars.add( var_t::from_const( usec_t<ulint>::hour, "hours" ) );
		vars.add( var_t::from_const( usec_t<ulint>::day , "d" ) );
		vars.add( var_t::from_const( usec_t<ulint>::day , "day" ) );
		vars.add( var_t::from_const( usec_t<ulint>::day , "days" ) );
		return( vars );
	}
	double calc( cstr_t str, vars_t const& vars = vars_t() )
	{
		double d = 0;
		calculator_t::eval( d, str.ptr, str.len, vars );
		return( d );
	}
};

struct cfgsection_t
{
	char const* buf;
	uint namebeg;
	uint nameend;
	
	struct entry_t
	{
		uint varbeg;
		uint varend;
		uint valbeg;
		uint valend;
		bool evaluated;

		bool hasvar( char const* buf, char const* _name ) const
		{
			uint l = d_strlen( _name );
			if( l != varend - varbeg )
				return( false );
			return( d_memcmp( buf + varbeg, _name, varend - varbeg ) );
		}
		
		template<typename t>
		bool getval( char const* buf, t & val ) const
		{
			uint numparsed = 0;
			val = str2int( buf, valbeg, valend, '\xa', numparsed );
			return( valend - valbeg == numparsed );
		}
	};
	
	constexpr static uint max_entries = 128;
	entry_t entries[max_entries];
	uint n_entries;
	
	constexpr cfgsection_t() :
		buf( nullptr ),
		namebeg( 0 ),
		nameend( 0 ),
		entries{},
		n_entries( 0 )
	{ }
	
	cfgsection_t const& operator =( cfgsection_t const& c )
	{
		buf = c.buf;
		namebeg = c.namebeg;
		nameend = c.nameend;
		cpy( entries, c.entries, sizeof(entries) );
		n_entries = c.n_entries;
		return( *this );
	}
	
	constexpr cfgsection_t( char const* _buf, uint _namebeg, uint _nameend ) :
		buf( _buf ),
		namebeg( _namebeg ),
		nameend( _nameend ),
		entries{},
		n_entries( 0 )
	{ }
	
	bool hasname( uint _namebeg, uint _nameend )
	{
		if( _nameend - _namebeg != nameend - namebeg )
			return( false );
		return( d_memcmp( buf + namebeg, buf + _namebeg, nameend - namebeg ) );
	}
	
	bool hasname( cstr_t name )
	{
		uint l = name.slen();
		if( l != nameend - namebeg )
			return( false );
		return( d_memcmp( name.ptr, buf + namebeg, l ) );
	}
	
	bool add( uint _varbeg, uint _varend, uint _valbeg, uint _valend )
	{
		if( n_entries >= max_entries )
			return( false );
		entries[n_entries] = {
			.varbeg = _varbeg,
			.varend = _varend,
			.valbeg = _valbeg,
			.valend = _valend,
			.evaluated = false
		};
		n_entries++;
		return( true );
	}

	template<typename t>
	t entry( cstr_t var, t def )
	{
		t val;
		for( uint i = 0; i < n_entries; i++ )
		{
			if( entries[i].hasvar( buf, var.ptr ) )
			{
				if( !entries[i].getval( buf, val ) )
				{
					d_fprint( "cfg: invalid value for entry '%S' of section [%*s]\n",
							  var, nameend - namebeg, buf + namebeg );
					break;
				}
				entries[i].evaluated = true;
				return( val );
			}
		}
		if( buf )
			d_fprint( "cfg: entry '%S' of section [%*s] not set, using default\n",
					  var, nameend - namebeg, buf + namebeg );
		return( def );
	}

	template <uint retbuflen>
	void entry( char (&retbuf)[retbuflen], cstr_t var, cstr_t def )
	{
		cstr_t val = entry( var, cstr_t() );
		if( !val.ptr || !val.len )
		{
			if( def.len > retbuflen - 1 )
				d_fprint( "cfg: default value for entry '%*s' of section [%*s] too long (%i/%i), truncating\n",
					      var.len, var.ptr, nameend - namebeg, buf + namebeg, def.len, retbuflen - 1 );
			d_fstrn( retbuf, retbuflen, "%*s", def.len, def.ptr );
		} else {
			if( val.len > retbuflen - 1 )
				d_fprint( "cfg: value for entry '%*s' of section [%*s] too long (%i/%i), truncating\n",
					      var.len, var.ptr, nameend - namebeg, buf + namebeg, val.len, retbuflen - 1 );
			d_fstrn( retbuf, retbuflen, "%*s", val.len, val.ptr );
		}
	}
	
	double calc( cstr_t var, cstr_t def, cfgcalc::vars_t const& vars = cfgcalc::vars_t() )
	{
		char buf[0x100];
		entry( buf, var, def );
		return( cfgcalc::calc( cstr_t( buf ), vars ) );
	}

	template <typename t>
	static constexpr t toint( double d )
	{
		return( (t)( d < 0 ? d - 0.5 : d + 0.5 ) );
	}

	template <typename t>
	t microseconds( cstr_t var, cstr_t def )
	{
		return( toint<t>( calc( var, def, cfgcalc::usvars() ) ) );
	}

	template <typename t>
	t hex( cstr_t var, cstr_t def )
	{
		char buf[0x100];
		entry( buf, var, def );
		uint num;
		uint len = d_strlen( buf, sizeof(buf) );
		t r = str2int<t>( buf, 0, len, '\x10', num );
		if( num != len )
			d_fprint( "cfg: value for entry '%s' of section [%*s] not in hex format\n",
			          var, nameend - namebeg, buf + namebeg );
		return( r );
	}
	
	void print() const
	{
		for( uint i_entry = 0; i_entry < n_entries; i_entry++ )
		{
			d_fprint( "%3b  %3i..%3i=%3i..%3i  '%*s' = '%*s'\n",
					  entries[i_entry].evaluated,
					  entries[i_entry].varend,
					  entries[i_entry].varbeg,
					  entries[i_entry].valend,
					  entries[i_entry].valbeg,
					  entries[i_entry].varend - entries[i_entry].varbeg,
					  buf + entries[i_entry].varbeg,
					  entries[i_entry].valend - entries[i_entry].valbeg,
					  buf + entries[i_entry].valbeg );
		}
	}

	bool check() const
	{
		bool error = false;
		uint neval = 0;
		for( uint i_entry = 0; i_entry < n_entries; i_entry++ )
			if( entries[i_entry].evaluated )
				neval++;
		if( neval )
		{
			for( uint i_entry = 0; i_entry < n_entries; i_entry++ )
			{
				if( !entries[i_entry].evaluated )
				{
					d_fprint( "cfg: entry '%*s' of section [%*s] not used\n",
							  entries[i_entry].varend - entries[i_entry].varbeg,
							  buf + entries[i_entry].varbeg,
							  nameend - namebeg, buf + namebeg );
					error = true;
				}
			}
		} else {
			d_fprint( "cfg: section [%*s] not used\n",
					  nameend - namebeg, buf + namebeg );
			error = true;
		}
		return( error );
	}
};

template<>
bool cfgsection_t::entry_t::getval<cstr_t>( char const* buf, cstr_t & val ) const
{
	val.ptr = buf + valbeg;
	val.len = valend - valbeg;
	return( valend > 0 );
}

template<>
bool cfgsection_t::entry_t::getval<bool>( char const* buf, bool & val ) const
{
	static const char * strs[][2] = { 
		{ "false", "true" },
		{ "off", "on" },
	};
	for( uint i = 0; i < lenof( strs ); i++ )
	{
		if( valend - valbeg == d_strlen( strs[i][0] )
		 && d_memcmp( buf + valbeg, strs[i][0], valend - valbeg ) )
		{
			val = false;
			return( true );
		}
		if( valend - valbeg == d_strlen( strs[i][1] )
		 && d_memcmp( buf + valbeg, strs[i][1], valend - valbeg ) )
		{
			val = true;
			return( true );
		}
	}
	return( false );
}

template<>
bool cfgsection_t::entry_t::getval<double>( char const* buf, double & val ) const
{
	uint numparsed = 0;
	val = str2doublel( buf, valbeg, valend, numparsed );
	return( valend - valbeg == numparsed );
}


template <uint nsections>
class cfg_t : public not_copyable_t
{
protected:
	cstr_t filename;
	file_to_buf_t file;
	bool error;
	cfgsection_t empty_section;
	cfgsection_t sections[nsections];
	uint n_sections;
	
public:
	cfg_t( cstr_t _filename ) : 
		filename( _filename ),
		error( false ),
		n_sections( 0 )
	{
		ENSURE( filename.terminated(), "config file name not null terminated\n" );
		reload();
	}
	
	cfgsection_t & section( cstr_t name )
	{
		for( uint i = 0; i < n_sections; i++ )
			if( sections[i].hasname( name ) )
				return( sections[i] );
		d_fprint( "cfg: section [%S] not found\n", name );
		return( empty_section );
	}
	
	void reload()
	{
		n_sections = 0;
		error = false;
		file.reload( filename.ptr, filename.len );
		parse();
	}
	
	void print() const
	{
		for( uint i_section = 0; i_section < n_sections; i_section++ )
		{
			d_fprint( "[ '%*s' ]\n",
			          sections[i_section].nameend - sections[i_section].namebeg,
			          sections[i_section].buf + sections[i_section].namebeg );
			sections[i_section].print();
		}
	}
	
	bool check()
	{
		for( uint i_section = 0; i_section < n_sections; i_section++ )
			if( !sections[i_section].check() )
				error = true;
		return( error );
	}
	
protected:
	constexpr static inline bool isblank( char c )
	{
		return( c == ' ' || c == '\t' );
	}
	
	constexpr static inline bool ischar( char c )
	{
		return( ( c >= 'a' && c <= 'z' )
		     || ( c >= 'A' && c <= 'Z' )
		     || ( c >= '0' && c <= '9' )
		     || ( c == '_' ) );
	}
	
	constexpr static inline bool islineend( char c )
	{
		return( c == '\n' );
	}
	
	constexpr static inline bool iscomment( char c )
	{
		return( c == '#' );
	}
	
	constexpr static inline bool isprintable( char c )
	{
		return( c >= '\x20' && c < '\x7f' );
	}
	
	void parse()
	{
		int isection = -1;
		uint p = 0;
		uint iline = 0;
		while( p < file.len )
		{
			int pline = p;
			while( p < file.len && isblank( file.str()[p] ) )
				p++;
			if( p < file.len )
			{
				char const * errmsg = "unknown error";
				bool okay = false;
				switch( file.str()[p] )
				{
					case '[':
					{
						p++;
						while( p < file.len && isblank( file.str()[p] ) )
							p++;
						int secbeg = p;
						while( p < file.len && ischar( file.str()[p] ) )
							p++;
						int secend = p;
						while( p < file.len && isblank( file.str()[p] ) )
							p++;
						if( !( p < file.len && file.str()[p] == ']' ) )
						{
							errmsg = "section name end not found";
							break;
						}
						p++;
						while( p < file.len && isblank( file.str()[p] ) )
							p++;
						if( p < file.len && !( islineend( file.str()[p] ) || iscomment( file.str()[p] ) ) )
						{
							okay = true;
							break;
						}
						isection = -1;
						for( uint i = 0; i < n_sections; i++ )
							if( sections[i].hasname( secbeg, secend ) )
								isection = i;
						if( isection < 0 )
						{
							if( n_sections >= lenof(sections) )
							{
								errmsg = "maximum number of sections exceeded";
								break;
							}
							sections[n_sections] = cfgsection_t( file.str(), secbeg, secend );
							isection = n_sections;
							n_sections++;
						}
						okay = true;
						break;
					}
					case '#':
						p++;
						[[fallthrough]];
					case '\n':
						if( !( p < file.len || islineend( file.str()[p] ) || iscomment( file.str()[p] ) ) )
							break;
						okay = true;
						break;
					default:
					{
						if( !ischar( file.str()[p] ) )
						{
							errmsg = "not a character";
							break;
						}
						int varbeg = p;
						while( p < file.len && ischar( file.str()[p] ) )
							p++;
						int varend = p;
						while( p < file.len && isblank( file.str()[p] ) )
							p++;
						if( !( p < file.len && file.str()[p] == '=' ) )
						{
							errmsg = "assignment operator not found";
							break;
						}
						p++;
						while( p < file.len && isblank( file.str()[p] ) )
							p++;
						int valbeg = p;
						int valend = p;
						for( ;; )
						{
							while( p < file.len && !islineend( file.str()[p] ) &&
							                  !isblank( file.str()[p] ) )
								p++;
							valend = p;
							if( p < file.len && isblank( file.str()[p] ) )
							{
								while( p < file.len && isblank( file.str()[p] ) )
									p++;
								if( p < file.len && !islineend( file.str()[p] ) &&
								               !iscomment( file.str()[p] ) )
									continue;
							}
							break;
						}
						if( p < file.len && !( islineend( file.str()[p] ) || iscomment( file.str()[p] ) ) )
						{
							errmsg = "additional data at end of line";
							break;
						}
						if( isection < 0 )
						{
							if( n_sections >= lenof(sections) )
							{
								errmsg = "maximum number of section exceeded";
								break;
							}
							sections[n_sections] = cfgsection_t( file.str(), 0, 0 );
							isection = n_sections;
							n_sections++;
						}
						if( !sections[isection].add( varbeg, varend, valbeg, valend ) )
						{
							errmsg = "could not add section";
							break;
						}
						okay = true;
						break;
					}
				}
				if( !okay )
				{
					error = true;
					char c = p < file.len ? file.str()[p] : 0;
					uint pnline = pline;
					while( pnline < file.len && ! islineend( file.str()[pnline] ) )
						pnline++;
					d_fprint_err( "error parsing line #%i, char #%i"
					              " of file %s (%s, '%c' (0x%2,2,16u) in \"%*s\")\n",
					              iline + 1, p - pline + 1, filename,
					              errmsg, isprintable( c ) ? c : '?', c,
					              pnline - pline, file.str() + pline );
				}
			}
			while( p < file.len && !islineend( file.str()[p] ) )
				p++;
			if( p < file.len )
				p++;
			iline++;
		}
	}
};

#endif // CFG_INCLUDED

