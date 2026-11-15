#ifndef READER_INCLUDED
#define READER_INCLUDED

#include "ctrl.cpp"
#include "file.cpp"
#include "db_str.cpp"

struct reader_line_t
{
	virtual void set( char const* ptr, uint l ) = 0;
	virtual void add( uint b, uint e ) = 0;
};

class reader_t
{
public:
	static constexpr inline bool is_whitespace( char const& c )
	{
		return( c == '\t' || c == '\r' || c == ' ' );
	}
	
	static constexpr inline bool is_lineend( char const& c )
	{
		return( c == '\n' );
	}
	
	void parse_line( char const* ptr, uint len, reader_line_t & line )
	{
		line.set( ptr, len );
		uint i = 0;
		do
		{
			while( i < len && is_whitespace( ptr[i] ) )
				i++;
			uint b = i;
			while( i < len && !is_whitespace( ptr[i] ) )
				i++;
			uint e = i;
			if( b == e )
				break;
			line.add( b, e );
		}
		while( i < len );
	}
};

class reader_forward_t : protected reader_t
{
protected:
	buffered_file_read_t file;
	uint lastlinelen;
public:
	constexpr reader_forward_t() :
		lastlinelen( 0 )
	{ }

	reader_forward_t( char const* filename, uint filenamelen )
	{
		setfile( filename, filenamelen );
	}

	void setfile( char const* filename, uint filenamelen )
	{
		file.reload( file.stdbufsize, filename, filenamelen );
		lastlinelen = 0;
	}

	bool nextline( reader_line_t & line )
	{
		file.advance( lastlinelen );
		if( !file.len() && file.eof() )
		{
			parse_line( nullptr, 0, line );
			return( false );
		}
		char const* ptr = file.str();
		uint len;
		for( len = 0; len < file.len() && !is_lineend( ptr[len] ); len++ );
		ENSURE( len < file.len() || file.eof(), "line larger than buffer\n" );
		lastlinelen = len + 1;
		parse_line( ptr, len, line );
		return( true );
	}
};

class reader_backward_t : protected reader_t
{
protected:
	buffered_file_read_backward_t file;
	uint lastlinelen;
public:
	constexpr reader_backward_t() :
		lastlinelen( 0 )
	{ }
	
	reader_backward_t( char const* filename, uint filenamelen )
	{
		setfile( filename, filenamelen );
	}

	void setfile( char const* filename, uint filenamelen )
	{
		file.reload( file.stdbufsize, filename, filenamelen );
		lastlinelen = 0;
	}

	bool nextline( reader_line_t & line )
	{
		file.advance( lastlinelen );
		if( !file.len() && file.eof() )
		{
			parse_line( nullptr, 0, line );
			return( false );
		}
		char const* ptr = file.str();
		int len = file.len();
		ENSURE( len, "len\n" );
		if( len > 0 && is_lineend( ptr[len-1] ) )
			len--;
		for( ; len > 0 && !is_lineend( ptr[len-1] ); len-- );
		ENSURE( len > 0 || file.eof(), "line larger than buffer\n" );
		lastlinelen = file.len() - len;
		parse_line( ptr + len, file.len() - len, line );
		return( true );
	}
};

#endif // READER_INCLUDED

