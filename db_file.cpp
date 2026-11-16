__GIT_SYMLINKS__
target=../_inc/db_file.cpp
target_resolved=/dibom/src/c/_inc/db_file.cpp
kind=file
sha256=e0b6d0b6102d4c1d83c46548dd4afd477efafefa1dfd7664cbf5feadb70611f8
size=16915
created=2026-11-16T20:49:12.739756
--END-HEADER--
#ifndef DB_FILE_INCLUDED
#define DB_FILE_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_str.cpp"

#ifdef LINUX
	#ifndef _LARGEFILE64_SOURCE
	#define _LARGEFILE64_SOURCE
	#endif
	#define  _FILE_OFFSET_BITS 64
	#include <stdio.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <unistd.h>
	#include <sys/stat.h>
	int _db_read( int fd, void *buf, size_t nbyte )
	{
		return( read( fd, buf, nbyte ) );
	}
	int _db_write( int fd, const void *buf, size_t nbyte )
	{
		return( write( fd, buf, nbyte ) );
	}
	int _db_close( int fd )
	{
		return( close( fd ) );
	}
	int _db_open( const char *filename, int flags, int _flags = 0 )
	{
		return( open( filename, flags, _flags ) );
	}
	ulint _db_seek( int fd, ulint pos )
	{
		return( lseek64( fd, pos, SEEK_SET ) );
	}
	bool _db_unlink( const char *filename )
	{
		return( unlink( filename ) );
	}
	lint _db_filesize( int fd )
	{
		struct stat64 stat;
		if( fd && fstat64( fd, &stat ) == 0 )
			return( stat.st_size );
		else
			return( -1 );
	}
	int _db_trunc( int fd, ulint size )
	{
		return( ftruncate( fd, size ) );
	}
	bool _db_chdir( const char *dir )
	{
		return( !( chdir( dir ) < 0 ) );
	}
	double _db_filetime( char const* filename )
	{
		struct stat64 stat;
		if( filename && stat64( filename, &stat ) == 0 )
			return( stat.st_mtime );
		else
			return( 0 );
	}
	bool _db_rename( char const* oldname, char const* newname )
	{
		return( !rename( oldname, newname ) );
	}
	bool _db_mkdir( char const* foldername )
	{
		return( !mkdir( foldername, 0777 ) );
	}
#else
	#include <fcntl.h> // _O_RDONLY ...
	#include <io.h> // _open, _read, _write, _lseeki64, _close
	#include <stdio.h> // fopen, fread, fwrite, fseek, fclose
	#include <sys\stat.h> // _stati64
	int _db_read( int fd, void *buf, size_t nbyte )
	{
		return( _read( fd, buf, nbyte ) );
	}
	int _db_write( int fd, const void *buf, size_t nbyte )
	{
		return( _write( fd, buf, nbyte ) );
	}
	int _db_close( int fd )
	{
		return( _close( fd ) );
	}
	int _db_open( const char *filename, int flags, int _flags = 0 )
	{
		return( _open( filename, flags, _flags ) );
	}
	ulint _db_seek( int fd, ulint pos )
	{
		return( _lseeki64( fd, pos, SEEK_SET ) );
	}
	bool _db_unlink( const char *filename )
	{
		return( _unlink( filename ) );
	}
	lint _db_filesize( int fd )
	{
		struct _stati64 stat;
		if( fd && _fstati64( fd, &stat ) == 0 )
			return( stat.st_size );
		else
			return( -1 );
	}
	int _db_trunc( int fd, ulint size )
	{
		return( _chsize( fd, size ) );
	}
#endif


/// handles reading and writing files
class fileclass
{
protected:
	fileclass( fileclass const& f ) = delete;
	fileclass& operator =( fileclass const& f ) = delete;
	
public:

	int handle, rp, wp, rwmode;
	lint pos; ///< actual position in file
	lint size; ///< size of file
	char *name; ///< name of file
	
	fileclass( fileclass && f ) = default;
	fileclass& operator =( fileclass && f ) = default;
	/// constructor, stores file name and read / write mode
	/** \param crwmode standard read/write: 'r'/'w', buffered read/write 'R'/'W' */
	fileclass()
	{
		handle = null;
		pos = 0;
		size = 0;
		rp = false;
		wp = false;
		rwmode = 0;
		name = null;
	}
	fileclass( const char *filename, char crwmode = 0 )
	{
		handle = null;
		pos = 0;
		size = 0;
		rp = false;
		wp = false;
		rwmode = 0;
		name = null;
		set( filename, crwmode );
	}
	void set( const char *filename, char crwmode = 0, uint filenamelen = -1 )
	{
		if( isopen() )
			close();
		if( crwmode != (char)-1 )
			rwmode = crwmode;
		delete [] name;
		name = null;
		if( filename )
		{
			uint l = d_strlen( filename, filenamelen ) + 1;
			name = new char[l];
			d_fstrn( name, l, "%*s", l - 1, filename );
		}
	}
	
	/// destructor, closes file
	~fileclass()
	{
		if( isopen() )
			close();
		if( name )
			delete [] name;
	}
	
	/// reads data
	/** \return true/false */
	inline bool read( void *buf, int length, lint position = -1 )
	{
		if( !seek( position ) )
			return( false );
		if( !rp || pos + length > size )
			return( false );
		if( handle )
		{
			int l = 0;
			while( length > 0 )
			{
				l = _db_read( handle, buf, length );
				if( l <= 0 )
					break;
				pos += l;
				length -= l;
			}
			if( !length )
				return( true );
			printf( "read error: %i/%i, errno: %i\n", l, length, errno );
		}
		return( false );
	}
	
	/// reads data
	/** \return true/false */
	inline int read_blind( void *buf, int length )
	{
		int l;
		
		if( handle )
		{
			l = _db_read( handle, buf, length );
			pos += l;
		} else {
			l = 0;
		}
		return( l );
	}
	
	/// writes data
	/** \return true/false */
	inline bool write( void *buf, int length, lint position = -1 )
	{
		int l;
		if( !seek( position ) )
			return( false );
		if( !wp )
			return( false );
		if( handle )
		{
			l = _db_write( handle, buf, length );
			pos += l;
			if( pos > size )
				size = pos;
			if( l == length )
				return( true );
			printf( "write error: %i/%i, errno: %i\n", l, length, errno );
		}
		return( false );
	}
	
	/// changes actual position in file
	/** \return true/false */
	inline bool seek( lint position = -1 )
	{
		if( position == -1 )
			position = pos;
		if( position == pos )
			return( true );
		if( position < 0 || position > size )
			return( false );
		if( handle )
		{
			pos = _db_seek( handle, position );
			if( pos == position )
				return( true );
			printf( "seek error, errno: %i\n", errno );
		}
		return( false );
	}
	
	/// checks if file is open
	/** \return true/false */
	inline bool isopen() const
	{
		return( handle );
	}
	
	/// drops/deletes/unlinks file (file is not unlinked by opening)
	/** \return true/false */
	inline bool drop()
	{
		if( name && !isopen() && !_db_unlink( name ) )
			return( true );
		return( false );
	}
	
	/// opens file
	/** \return true/false */
	inline bool open()
	{
		if( name && handle == null )
		{
			pos = 0;
			rp = false;
			wp = false;
			#ifdef LINUX
			switch( rwmode )
			{
				case 'r':
					handle = _db_open( name, O_LARGEFILE );
					rp = true;
					break;
				case 'w':
					handle = _db_open( name, O_RDWR | O_CREAT | O_LARGEFILE, S_IREAD | S_IWRITE | S_IRGRP | S_IROTH );
					rp = true;
					wp = true;
					break;
			}
			#else
			switch( rwmode )
			{
				case 'r':
					handle = _db_open( name, _O_RDONLY | _O_BINARY );
					rp = true;
					break;
				case 'w':
					handle = _db_open( name, _O_RDWR | _O_SEQUENTIAL | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE );
					rp = true;
					wp = true;
					break;
			}
			#endif
			if( handle == -1 )
				handle = null;
			size = _db_filesize( handle );
			if( size == -1 && handle )
			{
				close();
				return( false );
			}
			if( handle )
				return( true );
		}
		return( false );
	}
	
	/// sets file size
	/** \return true/false */
	inline bool setsize( lint _size )
	{
		if( handle && !_db_trunc( handle, _size ) )
		{
			size = _size;
			return( true );
		}
		return( false );
	}
	
	/// closes file
	/** \return true/false */
	inline bool close()
	{
		if( handle )
		{
			_db_close( handle );
			handle = null;
		}
		return( true );
	}
	
	enum : int { err_open = -1, err_toolarge = -2, err_args = -3, err_malloc = -4, err_read = -5, err_unknown = -6 };
	
	static int readall( char const* filename, byte** buf, int* bufsize, char mode = 'r' )
	{
		int length = -6;
		fileclass file( filename, mode );
		if( file.open() )
		{
			if( file.size > 0 )
			{
				if( file.size < type_t<int>::max() )
				{
					lint filesize = file.size;
					if( buf && bufsize ) 
					{
						if( *bufsize < filesize )
						{
							delete [] *buf;
							*bufsize = filesize + 1;
							*buf = new byte[filesize];
							if( !*buf )
								*bufsize = 0;
						}
						if( !( *bufsize < filesize ) )
						{
							if( file.read( *buf, filesize ) )
								length = filesize;
							else
								length = -5;
						} else {
							length = -4;
						}
					} else {
						length = -3;
					}
				} else {
					length = -2;
				}
			} else {
				length = 0;
			}
			file.close();
		} else {
			length = -1;
		}
		return( length );
	}
};

/// writes buf in a new file
/** \return true/false */
bool writefile( void *buf, int length, const char *filename, char mode = 'w' )
{
	fileclass file( filename, mode );
	file.drop();
	if( file.open() && file.write( buf, length ) )
	{
		if( file.size > length )
			file.setsize( length );
		file.close();
		return( true );
	} else {
		return( false );
	}
}

byte *readfile( const char *filename, int &length )
{
	byte *buf = nullptr;
	int bufsize = 0;
	length = fileclass::readall( filename, &buf, &bufsize );
	if( length <= 0 )
	{
		length = 0;
		delete [] buf;
		buf = nullptr;
	}
	return( buf );
}

class buffileclass
{
public:
	fileclass file;
	byte *buf;
	int bufsize1, bufsize2, buflength;
	lint beg, mid, end;
	
	buffileclass()
	{
		bufsize1 = 0;
		bufsize2 = 0;
		buflength = bufsize1 + bufsize2;
		beg = -1;
		mid = -1;
		end = -1;
		buf = null;
	}
	buffileclass( int _bufsize1, int _bufsize2 )
	{
		bufsize1 = _bufsize1;
		bufsize2 = _bufsize2;
		buflength = bufsize1 + bufsize2;
		beg = -1;
		mid = -1;
		end = -1;
		buf = new byte[buflength];
	}
	buffileclass( const char *filename, char crwmode, int _bufsize1, int _bufsize2 )
	{
		set( filename, crwmode );
		bufsize1 = _bufsize1;
		bufsize2 = _bufsize2;
		buflength = bufsize1 + bufsize2;
		beg = -1;
		mid = -1;
		end = -1;
		buf = new byte[buflength];
	}
	~buffileclass()
	{
		delete [] buf;
	}
	void set( const char *filename, char crwmode )
	{
		file.set( filename, crwmode );
	}
	void set( const char *filename, char crwmode, int _bufsize1, int _bufsize2, uint filenamelen = -1 )
	{
		file.set( filename, crwmode, filenamelen );
		dropa( buf );
		bufsize1 = _bufsize1;
		bufsize2 = _bufsize2;
		buflength = bufsize1 + bufsize2;
		beg = -1;
		mid = -1;
		end = -1;
		buf = new byte[buflength];
	}
	const lint &size() const
	{
		return( file.size );
	}
	const char *name() const
	{
		return( file.name );
	}
	
	inline byte *put( const lint &_pos )
	{
		lint pos;
		bool okay;
		
		okay = true;		
		if( _pos <= beg || _pos >= mid )
		{
			if( beg > -1 )
				write();
			if( _pos < file.size && file.rp )
			{
				if( !read( _pos ) )
				{
					okay = false;
				} else {
					mid = beg + bufsize1;
					end = mid + bufsize2;
				}
			} else {
				pos = ( _pos / bufsize1 ) * bufsize1;
				zro( buf, buflength );
				while( okay && file.size < pos )
					if( !file.write( buf, bufsize1, file.size ) )
						okay = false;
				if( okay )
				{
					beg = pos;
					mid = pos + bufsize1;
					end = mid + bufsize2;
				}
			}
		}
		if( okay )
			return( buf + _pos - beg );
		else
			return( null );
	}
	
	int write()
	{
		return( file.write( buf, (int)(end-beg), beg ) );
	}
	
	inline char *getc( const lint &pos )
	{
		return( (char*)get( pos ) );
	}
	
	inline byte *get( const lint &pos )
	{
		bool okay;
		
		if( pos >= beg && pos < mid )
		{
			return( buf + (int)( pos - beg ) );
		} else {
			okay = true;
			if( file.wp )
				if( !write() )
					okay = false;
			if( okay && pos < file.size && read( pos ) )
				return( buf + (int)( pos - beg ) );
			else
				return( null );
		}
	}
	
	bool read( lint pos )
	{
		lint re;
		bool done;
		
		done = false;
		pos = ( pos / bufsize1 ) * bufsize1;
		
		if( pos == beg )
		{
			done = true;
		} else {
			if( pos < 0 )
			{
				beg = -1;
				end = -1;
				mid = -1;
				zro( buf, buflength );
			} else {
				if( pos >= file.size )
				{
					beg = file.size;
					end = file.size;
					mid = file.size;
					zro( buf, buflength );
				} else {
					done = true;
					re = pos + buflength;
					if( pos > beg )
					{
						if( pos < end )
						{
							cpy( buf, buf+(int)(pos-beg), (int)(end-pos) );
							if( re > file.size )
							{
								re = file.size;
								if( re > end )
									if( !file.read( buf+(int)(end-pos), (int)(re-end), end ) )
										done = false;
								zro( buf+(int)(re-pos), buflength-(int)(re-pos) );
							} else {
								if( !file.read( buf+(int)(end-pos), (int)(re-end), end ) )
									done = false;
							}
						} else {
							if( re > file.size )
							{
								re = file.size;
								if( !file.read( buf, (int)(re-pos), pos ) )
									done = false;
								zro( buf+(int)(re-pos), buflength-(int)(re-pos) );
							} else {
								if( !file.read( buf, (int)(re-pos), pos ) )
									done = false;
							}
						}
					} else {
						if( re > file.size )
						{
							re = file.size;
							if( re > beg )
							{
								cpyr( buf+(int)(beg-pos), buf, (int)(re-beg) );
								if( !file.read( buf, (int)(beg-pos), pos ) )
									done = false;
								zro( buf+(int)(re-pos), buflength-(int)(re-pos) );
							} else {
								if( !file.read( buf, (int)(re-pos), pos ) )
									done = false;
								zro( buf+(int)(re-pos), buflength-(int)(re-pos) );
							}
						} else {
							if( re > beg )
							{
								cpyr( buf+(int)(beg-pos), buf, (int)(re-beg) );
								if( !file.read( buf, (int)(beg-pos), pos ) )
									done = false;
							} else {
								if( !file.read( buf, (int)(re-pos), pos ) )
									done = false;
							}
						}
					}
					beg = pos;
					end = re;
					mid = beg + bufsize1;
					if( mid > end )
						mid = end;
				}
			}
		}
		return( done );
	}
	int setsize( lint _size )
	{
		return( file.setsize( _size ) );
	}
	int isopen()
	{
		return( file.isopen() );
	}
	int drop()
	{
		return( file.drop() );
	}
	int open()
	{
		zro( buf, bufsize1 + bufsize2 );
		return( file.open() );
	}
	int close()
	{
		return( file.close() );
	}
};

class savefileclass
{
protected:
	buffileclass file;
	lint filepos;
	bool okay;
	bool dowrite;

public:
	savefileclass()
	{
		filepos = 0ull;
		dowrite = false;
	}
	~savefileclass()
	{
		close();
	}

	bool const& open_write( char const* filename )
	{
		dowrite = true;
		file.set( filename, 'w', 1<<20, 1<<20 );
		filepos = 0;
		okay = file.open();
		return( okay );
	}
	bool const& open_read( char const* filename )
	{
		dowrite = false;
		file.set( filename, 'r', 1<<20, 1<<20 );
		filepos = 0;
		okay = file.open();
		return( okay );
	}
	bool close()
	{
		if( file.file.rwmode == 'w' )
		{
			file.write();
			file.setsize( filepos );
		} else {
			if( file.file.rwmode == 'r' )
				okay &= file.file.size == filepos;
		}
		file.close();
		return( okay );
	}
	inline void invalidate()
	{
		okay = false;
	}
	inline bool const& isokay() const
	{
		return( okay );
	}
	bool const& iswriting() const
	{
		return( dowrite );
	}
	bool const& rw( byte * data, uint datalength )
	{
		if( dowrite )
			return( write( data, datalength ) );
		else
			return( read( data, datalength ) );
	}
	template <typename T> bool const& rw( T & data )
	{
		return( rw( (byte*)&data, sizeof( data ) ) );
	}
protected:
	bool const& write( byte const* data, uint datalength )
	{
		byte *filedatapointer;
		uint i, l;

		if( okay )
		{
			if( datalength < (uint)file.bufsize2 )
			{
				if( ( filedatapointer = file.put( filepos ) ) )
				{
					cpy( filedatapointer, data, datalength );
					filepos += datalength;
				} else {
					okay = false;
				}
			} else {
				i = 0;
				while( okay && i < datalength )
				{
					if( ( filedatapointer = file.put( filepos ) ) )
					{
						l = d_min( (uint)file.bufsize2, datalength - i );
						cpy( filedatapointer, data+i, l );
						i += l;
						filepos += l;
					} else {
						okay = false;
					}
				}
			}
		}
		return( okay );
	}
	template <typename T> bool const& write( T const& data )
	{
		return( write( (byte const*)&data, sizeof( data ) ) );
	}
	bool const& read( byte * data, uint datalength )
	{
		byte *filedatapointer;
		uint i, l;

		i = 0;
		if( okay )
		{
			l = datalength;
			while( l && i < datalength )
			{
				if( ( filedatapointer = file.get( filepos ) ) )
				{
					l = d_min( (uint)( file.end - filepos ), datalength - i );
					cpy( data+i, filedatapointer, l );
					i += l;
					filepos += l;
				} else {
					l = 0;
				}
			}
		} else {
			okay = false;
		}
		if( !okay )
			zro( data+i, datalength-i );
		return( okay );
	}
	template <typename T> bool const& read( T & data )
	{
		return( read( (byte*)&data, sizeof( data ) ) );
	}
};


#endif // DB_FILE_INCLUDED
