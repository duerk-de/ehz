#ifndef MODBUS_INCLUDED
#define MODBUS_INCLUDED

#include "ctrl.cpp"
#include "crc.cpp"
#include "request_response.cpp"

struct modbus_crc_t
{
	crc_t<ushort> crc;
	
	modbus_crc_t() :
		crc( 0x8005, 0xffff, 0x0000, true )
	{ }

	template <typename t>
	ushort calc( t const& v )
	{
		crc.reset();
		for( byte const* p = (byte const*)&v, * pe = p + sizeof(v); p < pe; p++ )
			crc.add( *p );
		return( crc.get() );
	}
};

template <typename t>
static constexpr t flip_endianess_step( t v, uint b )
{
	return( ( ( ( v & ( (t)type_t<byte>::max() << ( b * type_t<byte>::numbits() ) )
				) >> ( b * type_t<byte>::numbits() )
			  ) << ( ( ( sizeof( t ) - 1 ) - b ) * type_t<byte>::numbits() )
			) | ( b ? flip_endianess_step( v, b - 1 ) : 0 ) );
}

template <typename t>
static constexpr t flip_endianess( t v )
{
	return( flip_endianess_step( v, sizeof( t ) - 1 ) );
}

struct modbus_t : public request_response_t
{
	constexpr static ushort write_offset = 40000 * 0;

	struct hdr_base_t
	{
		byte address;
		byte function;
	} __attribute__((packed));

	struct rqst_hdr_t
	{
		hdr_base_t base;
		ushort beg;
		ushort len;
	} __attribute__((packed));
	
	struct rsp_hdr_t
	{
		hdr_base_t base;
	} __attribute__((packed));

	struct frame_crc_t
	{
		ushort crc;
	} __attribute__((packed));
	
	buf_t response_buf;
	modbus_crc_t & crc;

	inline constexpr modbus_t( void * buf, uint len, modbus_crc_t & _crc,
	                           cstr_t _name ) :
		request_response_t( _name ),
		response_buf {
			.buf = static_cast<byte *>( buf ),
			.len = len },
		crc( _crc )
	{ }
};



template <ushort data_pos, typename data_t>
struct modbus_read_t : public modbus_t
{
	constexpr static byte function = 0x03; // Read Multiple Holding Registers, 16 bits

	struct
	{
		rqst_hdr_t hdr;
		frame_crc_t crc;
	} __attribute__((packed)) request;
	
	struct
	{
		rsp_hdr_t hdr;
		byte size;
		data_t data;
		frame_crc_t crc;
	} __attribute__((packed)) response;

	/*
	inline constexpr modbus_read_t( modbus_crc_t & _crc ) :
		modbus_t( static_cast<void *>( &response ), sizeof( response ), _crc ),
		request {
			.hdr {
				.base {
					.address = flip_endianess<byte>( 0 ),
					.function = flip_endianess( function ) },
				.beg = flip_endianess( data_pos ),
				.len = flip_endianess<ushort>( sizeof( response.data ) / sizeof( ushort ) ) },
			.crc {
				.crc = 0 } }
	{ }
	*/

	inline constexpr modbus_read_t( modbus_crc_t & _crc, data_t const& def,
	                                cstr_t _name ) :
		modbus_t( static_cast<void *>( &response ), sizeof( response ), _crc, _name ),
		request {
			.hdr {
				.base {
					.address = flip_endianess<byte>( 0 ),
					.function = flip_endianess( function ) },
				.beg = flip_endianess( data_pos ),
				.len = flip_endianess<ushort>( sizeof( response.data ) / sizeof( ushort ) ) },
			.crc {
				.crc = 0 } },
		response {
			.hdr {
				.base {
					.address = flip_endianess<byte>( 0 ),
					.function = flip_endianess<byte>( 0 ) } },
			.size = 0,
			.data = def,
			.crc {
				.crc = 0 } }
	{ }

	void set_address( byte address )
	{
		request.hdr.base.address = flip_endianess( address );
		request.crc.crc = crc.calc( request.hdr );
	}
	
	inline bool is_response_valid() const
	{
		bool okay = true;
		if( crc.calc( response ) )
		{
			d_fprint_err( "modbus_read of %S failed: crc error"
			              ", response: %*x\n", name,
			              (uint)sizeof(response), &response );
			okay = false;
		}
		if( !cmp( response.hdr.base, request.hdr.base ) )
		{
			d_fprint_err( "modbus_read of %S failed: invalid header (%*x != %*x)"
			              ", response: %*x\n", name,
			              (uint)sizeof(response.hdr.base), &response.hdr.base,
			              (uint)sizeof(request.hdr.base), &request.hdr.base,
			              (uint)sizeof(response), &response );
			okay = false;
		}
		if( response.size != sizeof( response.data ) )
		{
			d_fprint_err( "modbus_read of %S failed: invalid size (%u/%u)"
			              ", response: %*x\n", name,
			              (uint)response.size, (uint)sizeof(response.data),
			              (uint)sizeof(response), &response );
			okay = false;
		}
		return( okay );
	}
	
	inline constexpr data_t const& data() const
	{
		return( response.data );
	}

	virtual buf_t get_request()
	{
		return( buf_t{ .buf = (byte *)&request, .len = sizeof( request ) } );
	}
	
	virtual buf_t & get_response()
	{
		response_buf.len = sizeof( response );
		return( response_buf );
	}
};

template <ushort data_pos, typename data_t>
struct modbus_write_t : public modbus_t
{
	constexpr static byte function = 0x10; // Write Multiple Holding Registers, 16 bits

	struct
	{
		struct {
			rqst_hdr_t hdr;
			byte size;
			data_t data;
		} __attribute__((packed)) load;
		frame_crc_t crc;
	} __attribute__((packed)) request;
	
	struct
	{
		rsp_hdr_t hdr;
		ushort beg;
		ushort len;
		byte size;
		data_t data;
		frame_crc_t crc;
	} __attribute__((packed)) response;

	inline constexpr modbus_write_t( modbus_crc_t & _crc, data_t const& def, cstr_t _name ) :
		modbus_t( static_cast<void *>( &response ), sizeof( response ), _crc, _name ),
		request {
			.load {
				.hdr {
					.base {
						.address = flip_endianess<byte>( 0 ),
						.function = flip_endianess( function ) },
					.beg = flip_endianess( data_pos ),
					.len = flip_endianess<ushort>( sizeof( data_t ) / sizeof( ushort ) ) },
				.size = sizeof( data_t ),
				.data = def },
			.crc {
				.crc = 0 } },
		response{
			.hdr {
				.base {
					.address = flip_endianess<byte>( 0 ),
					.function = flip_endianess<byte>( 0 ) } },
			.beg = flip_endianess<ushort>( 0 ),
			.len = flip_endianess<ushort>( 0 ),
			.size = flip_endianess<ushort>( 0 ),
			.data = def,
			.crc {
				.crc = 0 } }
	{ }

	void set_address( byte address )
	{
		request.load.hdr.base.address = flip_endianess( address );
	}

	inline constexpr data_t & data()
	{
		return( request.load.data );
	}

	void make_request()
	{
		request.crc.crc = crc.calc( request.load );
	}
	
	inline bool is_response_valid() const
	{
		bool okay = true;
		if( crc.calc( response ) )
		{
			d_fprint_err( "modbus_write of %S failed: crc error\n", name );
			okay = false;
		}
		if( !cmp( response.hdr.base, request.load.hdr.base ) )
		{
			d_fprint_err( "modbus_write of %S failed: invalid header (%*x != %*x)"
			              ", response: %*x\n", name,
			              (uint)sizeof(response.hdr.base), &response.hdr.base,
			              (uint)sizeof(request.load.hdr.base), &request.load.hdr.base,
			              (uint)sizeof(response), &response );
			okay = false;
		}
		if( response.beg != request.load.hdr.beg )
		{
			d_fprint_err( "modbus_write of %S failed: invalid data pos (%u!=%u)"
			              ", response: %*x\n",
			              name, (uint)response.beg, (uint)request.load.hdr.beg,
			              (uint)sizeof(response), &response );
			okay = false;
		}
		if( response.len != request.load.hdr.len )
		{
			d_fprint_err( "modbus_write of %S failed: invalid data len (%u!=%u)"
			              ", response: %*x\n",
			              name, (uint)response.len, (uint)request.load.hdr.len,
			              (uint)sizeof(response), &response );
			okay = false;
		}
		return( okay );
	}
		
	virtual buf_t get_request()
	{
		return( buf_t{ .buf = (byte *)&request, .len = sizeof( request ) } );
	}
	
	virtual buf_t & get_response()
	{
		response_buf.len = sizeof( response );
		return( response_buf );
	}
};


template <ushort data_pos>
struct modbus_write_single_t : public modbus_t
{
	constexpr static byte function = 0x06; // Write Single Holding Register, 16 bits

	struct
	{
		rsp_hdr_t hdr;
		ushort beg;
		ushort val;
		frame_crc_t crc;
	} __attribute__((packed)) request;
	
	struct
	{
		rsp_hdr_t hdr;
		ushort beg;
		ushort val;
		frame_crc_t crc;
	} __attribute__((packed)) response;

	inline constexpr modbus_write_single_t( modbus_crc_t & _crc, ushort def ) :
		modbus_t( static_cast<void *>( &response ), sizeof( response ), _crc ),
		request {
			.hdr {
				.base {
					.address = flip_endianess<byte>( 0 ),
					.function = flip_endianess( function ) } },
			.beg = flip_endianess( data_pos ),
			.val = def,
			.crc {
				.crc = 0 } },
		response{
			.hdr {
				.base {
					.address = flip_endianess<byte>( 0 ),
					.function = flip_endianess<byte>( 0 ) } },
			.beg = flip_endianess<ushort>( 0 ),
			.val = flip_endianess<ushort>( 0 ),
			.crc {
				.crc = 0 } }
	{ }

	void set_address( byte address )
	{
		request.hdr.base.address = flip_endianess( address );
	}

	inline void setdata( ushort le_val )
	{
		request.val = le_val;
	}

	void make_request()
	{
		request.crc.crc = crc.calc( request.hdr );
	}
	
	inline bool is_response_valid() const
	{
		if( crc.calc( response ) )
			return( false );
		if( !cmp( response.hdr.base, request.hdr.base ) )
			return( false );
		if( response.beg != request.beg )
			return( false );
		if( response.val != request.val )
			return( false );
		return( true );
	}
	
	virtual buf_t get_request()
	{
		return( buf_t{ .buf = (byte *)&request, .len = sizeof( request ) } );
	}
	
	virtual buf_t & get_response()
	{
		response_buf.len = sizeof( response );
		return( response_buf );
	}
};


#endif // MODBUS_INCLUDED

