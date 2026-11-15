#ifndef FILE_INCLUDED
#define FILE_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "ctrl.cpp"


#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#define  _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

class file_t : public not_copyable_t
{
protected:
	static constexpr bool dbg = false;

	void _try_open( char const * file, uint len, int oflags, int sflags )
	{
		_close();
		ENSURE( d_strlen( file, len ) < len, "file name not null terminated\n" );
		if( dbg )
			d_fprint( "file_t:: opening file %*s\n", len, file );
		do
			fd = ::open( file, oflags, sflags );
		while( fd < 0 && errno == EINTR );
	}

	void _close()
	{
		if( fd < 0 )
			return;
		if( dbg )
			d_fprint( "file_t:: closing file\n" );
		::close( fd );
		fd = -1;
	}
	
	int fd;

public:

	void trunc( lint size )
	{
		if( dbg )
			d_fprint( "file_t:: truncating to %I\n", size );
		int ret;
		do
			ret = ::ftruncate( fd, size );
		while( ret < 0 && errno == EINTR );
		ENSURE( ret >= 0, "error truncating: %1,1,16u\n", errno );
	}

	void seek( lint pos )
	{
		if( dbg )
			d_fprint( "file_t:: seeking to %I\n", pos );
		lint ret;
		do
			ret = ::lseek64( fd, pos, SEEK_SET );
		while( ret < 0 && errno == EINTR );
		ENSURE( ret >= 0, "error seeking: %1,1,16u\n", errno );
	}
	
	lint size()
	{
		struct stat64 stat;
		int ret;
		do
			ret = ::fstat64( fd, &stat );
		while( ret < 0 && errno == EINTR );
		ENSURE( ret >= 0, "error getting size: %1,1,16u\n", errno );
		if( dbg )
			d_fprint( "file_t:: file size is %I\n", stat.st_size );
		return( stat.st_size );
	}
	
	void read( byte * buf, uint len )
	{
		if( dbg )
			d_fprint( "file_t:: reading %i\n", len );
		int readden;
		do {
			readden = ::read( fd, buf, len );
			ENSURE( readden < 0 || (uint)readden <= len, "fatal read error: %1,1,16u\n", errno );
			if( readden >= 0 )
			{
				buf += readden;
				len -= readden;
			}
		} while( ( readden < 0 && errno == EINTR ) || ( readden >= 0 && len ) );
		ENSURE( !len, "error reading: %1,1,16u\n", errno );
	}
	
	void write( byte const* buf, uint len )
	{
		if( dbg )
			d_fprint( "file_t:: writing %i\n", len );
		int written;
		do {
			written = ::write( fd, buf, len );
			ENSURE( written < 0 || (uint)written <= len, "fatal write error: %1,1,16u\n", errno );
			if( written >= 0 )
			{
				buf += written;
				len -= written;
			}
		} while( ( written < 0 && errno == EINTR ) || ( written >= 0 && len ) );
		ENSURE( !len, "error writing: %1,1,16u\n", errno );
	}

	constexpr file_t() :
		fd( -1 )
	{ }
	
	~file_t()
	{
		_close();
	}
	
	void open_read( char const * file, uint len )
	{
		int oflags = O_LARGEFILE;
		int sflags = 0;
		_try_open( file, len, oflags, sflags );
		ENSURE( fd >= 0, "error opening %*s with %1,1,16u/%1,1,16u: %1,1,16u\n",
		                 len, file, oflags, sflags, errno );
	}

	void open_write( char const * file, uint len )
	{
		int oflags = O_RDWR | O_CREAT | O_LARGEFILE;
		int sflags = S_IREAD | S_IWRITE | S_IRGRP | S_IROTH;
		_try_open( file, len, oflags, sflags );
		ENSURE( fd >= 0, "error opening %*s with %1,1,16u/%1,1,16u: %1,1,16u\n",
		                 len, file, oflags, sflags, errno );
	}
	
	static bool is_available( char const * file, uint len )
	{
		int oflags = O_LARGEFILE;
		int sflags = 0;
		file_t f;
		f._try_open( file, len, oflags, sflags );
		return( f.fd >= 0 );
	}
};

struct file_to_buf_t : public not_copyable_t
{
	byte * buf;
	uint len;
	
	constexpr file_to_buf_t() :
		buf( nullptr ),
		len( 0 )
	{ }

	~file_to_buf_t()
	{
		if( buf )
			delete [] buf;
	}
	
	file_to_buf_t( char const* filename, uint filenamelen ) :
		file_to_buf_t()
	{
		reload( filename, filenamelen );
	}
	
	file_to_buf_t & reload( char const* filename, uint filenamelen )
	{
		file_t file;
		file.open_read( filename, filenamelen );
		lint filesize = file.size();
		if( len != filesize )
		{
			delete [] buf;
			len = filesize;
			ENSURE( len == filesize, "file does not fit into buffer\n" );
			buf = new byte[len];
			ENSURE( buf, "alloc error\n" );
		}
		file.read( buf, len );
		return( *this );
	}

	inline char * str() const
	{
		return( (char *)buf );
	}
};

class buffered_file_base_t : public not_copyable_t
{
protected:
	static constexpr bool dbg = false;
	
	file_t file;
	lint filepos;
	lint filesize;
	byte * buf;
	uint bufsize;
	uint bufpos;
	bool writeable;

	void init( uint _bufsize, bool truncate, lint _pos, bool pos_from_back )
	{
		filepos = 0;
		filesize = file.size();
		if( _bufsize != bufsize )
		{
			if( buf )
				delete [] buf;
			buf = new byte[_bufsize*2];
			ENSURE( buf, "alloc error\n" );
			bufsize = _bufsize;
		}
		if( dbg )
			d_fprint( "zro %i+%i\n", buf - buf, bufsize * 2 );
		zro( buf, bufsize * 2 );
		if( truncate )
		{
			filesize = 0;
			bufpos = 0;
		} else {
			update( ( pos_from_back ? filesize - _pos : _pos ) / bufsize, true, 0 );
		}
	}

public:
	constexpr buffered_file_base_t() :
		filepos( 0 ),
		filesize( 0 ),
		buf( nullptr ),
		bufsize( 0 ),
		bufpos( 0 ),
		writeable( false )
	{ }
	
	~buffered_file_base_t()
	{
		if( buf )
			delete [] buf;
	}

	void open_read_forward( uint _bufsize, char const* _file, uint _len )
	{
		file.open_read( _file, _len );
		init( _bufsize, false, 0, false );
		writeable = false;
	}

	void open_read_backward( uint _bufsize, char const* _file, uint _len )
	{
		file.open_read( _file, _len );
		init( _bufsize, false, 0, true );
		writeable = false;
	}

	void open_write_truncate( uint _bufsize, char const* _file, uint _len )
	{
		file.open_write( _file, _len );
		init( _bufsize, true, 0, false );
		writeable = true;
	}
	
	byte * at( lint pos )
	{
		ENSURE( pos >= 0, "negative file position\n" );
		uint newpos = pos / bufsize;
		if( newpos != bufpos )
			update( newpos, false, 0 );
		if( dbg )
			d_fprint( "at %I (%I/%i)\n", pos, pos - (lint)bufpos * (lint)bufsize, bufsize*2 );
		return( buf + ( pos - (lint)bufpos * (lint)bufsize ) );
	}
	
	inline constexpr lint const& fsize() const
	{
		return( filesize );
	}
	
	inline constexpr uint const& bsize() const
	{
		return( bufsize );
	}

	void trunc( lint size )
	{
		if( writeable )
		{
			if( size > (lint)bufpos * (lint)bufsize )
				update( 0, false, size - (lint)bufpos * (lint)bufsize );
			file.trunc( size );
			filesize = size;
		}
	}

protected:
	void update( uint newpos, bool readonly, uint writeonlylen )
	{
		if( readonly )
		{
			read( buf, (lint)newpos * (lint)bufsize, bufsize * 2 );
		}
		else if( writeonlylen )
		{
			if( writeable )
				write( buf, (lint)bufpos * (lint)bufsize, d_min( bufsize * 2, writeonlylen ) );
		}
		else if( newpos == bufpos )
		{
		}
		else if( newpos == bufpos + 1 )
		{
			if( writeable )
				write( buf, (lint)bufpos * (lint)bufsize, bufsize );
			move( buf, buf + bufsize, bufsize );
			read( buf + bufsize, (lint)( newpos + 1 ) * (lint)bufsize, bufsize );
		}
		else if( newpos == bufpos - 1 )
		{
			if( writeable )
				write( buf + bufsize, (lint)( bufpos + 1 ) * (lint)bufsize, bufsize );
			move( buf + bufsize, buf, bufsize );
			read( buf, (lint)newpos * (lint)bufsize, bufsize );
		}
		else
		{
			if( writeable )
				write( buf, (lint)bufpos * (lint)bufsize, bufsize * 2 );
			read( buf, (lint)newpos * (lint)bufsize, bufsize * 2 );
		}
		bufpos = newpos;
	}

	void write( byte const* writebuf, lint writepos, uint writelen )
	{
		if( dbg )
			d_fprint( "write %I+%i\n", writebuf - buf, writelen );
		if( filepos != writepos )
			file.seek( writepos );
		file.write( writebuf, writelen );
		filepos = writepos + writelen;
		if( writepos + writelen > filesize )
			filesize = writepos + writelen;
	}
	
	void move( byte * dst, byte const* src, uint len )
	{
		if( dbg )
			d_fprint( "move %i+%i -> %i\n", src - buf, len, dst - buf );
		cpy( dst, src, len );
	}

	void read( byte * readbuf, lint readpos, uint readlen )
	{
		if( readpos + readlen >= filesize )
		{
			uint zrolen;
			byte * zrobuf;
			if( readpos >= filesize )
			{
				zrolen = readlen;
				zrobuf = readbuf;
			} else {
				zrolen = ( readpos + readlen ) - filesize;
				zrobuf = readbuf + ( readlen - zrolen );
			}
			if( dbg )
				d_fprint( "zro %I+%i\n", zrobuf - buf, zrolen );
			zro( zrobuf, zrolen );
			readlen -= zrolen;
		}
		if( readlen )
		{
			if( dbg )
				d_fprint( "read %I+%i @ %I\n", readbuf - buf, readlen, readpos );
			if( filepos != readpos )
				file.seek( readpos );
			file.read( readbuf, readlen );
			filepos = readpos + readlen;
		}

	}
};

class buffered_file_t
{
public:
	static constexpr uint stdbufsize = 1 << 18;

protected:
	buffered_file_base_t file;

	byte * buf;
	uint len;
	lint pos;
	bool eof;
	
	constexpr buffered_file_t() :
		buf( nullptr ),
		len( 0 ),
		pos( 0 ),
		eof( false )
	{ }
	
	~buffered_file_t()
	{
		file.trunc( pos );
	}
	
public:
	bool advance( uint _len )
	{
		pos += _len;
		read();
		return( len );
	}

protected:
	void read()
	{
		buf = file.at( pos );
		len = file.bsize();
		eof = pos + len >= file.fsize();
		if( eof )
			len = file.fsize() - pos;
	}
};

class buffered_file_backward_t
{
public:
	static constexpr uint stdbufsize = 256 * bin_t<uint>::k;

protected:
	buffered_file_base_t file;

	byte * buf;
	uint len;
	lint pos;
	bool eof;
	
	constexpr buffered_file_backward_t() :
		buf( nullptr ),
		len( 0 ),
		pos( 0 ),
		eof( false )
	{ }
	
public:
	bool advance( uint _len )
	{
		ENSURE( _len <= pos, "file position underflow\n" );
		pos -= _len;
		read();
		return( len );
	}

protected:
	void read()
	{
		len = file.bsize();
		eof = pos <= len;
		if( eof )
			len = pos;
		buf = file.at( pos - len );
	}
};

struct buffered_file_read_t : public buffered_file_t
{
	constexpr buffered_file_read_t() = default;

	template <uint filenamelen>
	buffered_file_read_t( uint bufsize, char const (&filename)[filenamelen] )
	{
		reload( bufsize, filename, filenamelen );
	}
	
	void reload( uint bufsize, char const* filename, uint filenamelen )
	{
		file.open_read_forward( bufsize, filename, filenamelen );
		read();
	}
	
	inline constexpr byte const* buf() const
	{
		return( buffered_file_t::buf );
	}

	inline char const* str() const
	{
		return( (char const*)buffered_file_t::buf );
	}

	inline constexpr uint const& len() const
	{
		return( buffered_file_t::len );
	}

	inline constexpr bool const& eof() const
	{
		return( buffered_file_t::eof );
	}
};

struct buffered_file_read_backward_t : public buffered_file_backward_t
{
	constexpr buffered_file_read_backward_t() = default;

	template <uint filenamelen>
	buffered_file_read_backward_t( uint bufsize, char const (&filename)[filenamelen] )
	{
		reload( bufsize, filename, filenamelen );
	}
	
	void reload( uint bufsize, char const* filename, uint filenamelen )
	{
		file.open_read_backward( bufsize, filename, filenamelen );
		pos = file.fsize();
		read();
	}
	
	inline constexpr byte const* buf() const
	{
		return( buffered_file_backward_t::buf );
	}

	inline char const* str() const
	{
		return( (char const*)buffered_file_backward_t::buf );
	}

	inline constexpr uint const& len() const
	{
		return( buffered_file_backward_t::len );
	}

	inline constexpr bool const& eof() const
	{
		return( buffered_file_backward_t::eof );
	}
};

struct buffered_file_write_t : public buffered_file_t
{
	constexpr buffered_file_write_t() = default;
	
	template <uint filenamelen>
	buffered_file_write_t( uint bufsize, char const (&filename)[filenamelen] )
	{
		reload( bufsize, filename, filenamelen );
	}
	
	void reload( uint bufsize, char const* filename, uint filenamelen )
	{
		file.trunc( pos );
		file.open_write_truncate( bufsize, filename, filenamelen );
		read();
	}
	
	inline constexpr byte * buf() const
	{
		return( buffered_file_t::buf );
	}

	inline char * str() const
	{
		return( (char *)buffered_file_t::buf );
	}

	inline constexpr uint const& len() const
	{
		return( file.bsize() );
	}
};


#endif // FILE_INCLUDED

