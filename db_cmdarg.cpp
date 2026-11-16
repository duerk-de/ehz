__GIT_SYMLINKS__
target=../_inc/db_cmdarg.cpp
target_resolved=/dibom/src/c/_inc/db_cmdarg.cpp
kind=file
sha256=90d1ce0112e2780330a20d0016177421e24189bd02a549f1f3195c2e7e7e2d2f
size=2612
created=2026-11-16T20:49:12.729457
--END-HEADER--
#ifndef CMDARG_INCLUDED
#define CMDARG_INCLUDED

#include "db_mem.cpp"
#include "db_str.cpp"
#include "db_consoleio.cpp"

template<typename t> class cmdarg
{
public:
	char* name;
	t val;
	bool set;
	char const* description;
	char sep;
	
	cmdarg( t defval, char const* _argname,
	        char const* _description = nullptr, char _sep = '=' )
	{
		sep = _sep;
		int len = d_strlen(_argname) + 2;
		name = new char[len];
		int off = str2strn( name, len, _argname );
		name[off++] = sep;
		name[off] = '\0';
		val = defval;
		set = false;
		description = _description;
	}
	
	cmdarg( cmdarg const& a ) = delete;
	cmdarg operator =( cmdarg const& a ) = delete;
	
	~cmdarg()
	{
		delete [] name;
	}
	
	inline bool isset() const
	{
		return( set );
	}
	
	inline bool check_isset() const
	{
		if( set )
			return( true );
		if( description )
			d_fprint( "%s (%*s) not set\n", description, d_strlen(name)-1, name );
		else
			d_fprint( "%*s not set\n", d_strlen(name)-1, name );
		return( false );
	}
	
	static bool d_strbegwith( const char *str1, const char *str2 )
	{
		return( d_strcmp( str1, str2, d_strlen( str2 ) ) );
	}

	bool isarg( char const* iarg ) const
	{
		return( d_strbegwith( iarg, name ) );
	}
	
	bool get( char const* iarg );
};

template<> bool cmdarg<bool>::isarg( char const* iarg ) const
{
	int len = d_strlen( iarg );
	return( d_strcmp( name, iarg, len ) && name[len] == sep );
}

template<> bool cmdarg<bool>::get( char const* iarg )
{
	if( !isarg( iarg ) )
		return( false );
	val = true;
	set = true;
	return( true );
}

template<> bool cmdarg<char const*>::get( char const* iarg )
{
	if( !isarg( iarg ) )
		return( false );
	val = iarg + d_strlen( name );
	set = true;
	return( true );
}

template<> bool cmdarg<int>::get( char const* iarg )
{
	if( !isarg( iarg ) )
		return( false );
	uint l = d_strlen( iarg );
	uint i = d_strlen( name );
	uint k = 0;
	int v = str2int<int>( iarg, i, l, '\xa', k );
	if( i + k != l )
		return( false );
	val = v;
	set = true;
	return( true );
}

template<> bool cmdarg<double>::get( char const* iarg )
{
	if( !isarg( iarg ) )
		return( false );
	uint l = d_strlen( iarg );
	uint i = d_strlen( name );
	uint k = 0;
	double v = str2doublel( iarg, i, l, k );
	if( i + k != l )
		return( false );
	val = v;
	set = true;
	return( true );
}

template<> bool cmdarg<float>::get( char const* iarg )
{
	if( !isarg( iarg ) )
		return( false );
	uint l = d_strlen( iarg );
	uint i = d_strlen( name );
	uint k = 0;
	float v = (float)str2doublel( iarg, i, l, k );
	if( i + k != l )
		return( false );
	val = v;
	set = true;
	return( true );
}

#endif // CMDARG_INCLUDED

