#ifndef FILEIO_INCLUDED
#define FILEIO_INCLUDED

#include "signals.cpp"
#include "unixio.cpp"
#include "ctrl.cpp"
#include "db_file.cpp"
#include <sys/ioctl.h>
#include <sys/poll.h>

struct file_io_t : public unix_io_t
{
	virtual void post_setup()
	{ }
	virtual void pre_teardown()
	{ }

	constexpr file_io_t( signals_t::data_t const& _signals, int _oflags, int _sflags ) :
		unix_io_t( _signals, _oflags, _sflags )
	{ }

	void truncate()
	{
		int ret = _db_trunc( fd, 0 );
		ENSURE( ret == 0, "error %i truncating %S: 0x%1,1,16u\n",
		        ret, name, errno );
		
	}
};

#endif // FILEIO_INCLUDED

