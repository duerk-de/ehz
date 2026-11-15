#ifndef CALC_INCLUDED
#define CALC_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "ctrl.cpp"

#include "db_time.cpp"
#include "db_math.cpp"
#include "math.cpp"

struct calculator_t
{
	typedef double calc_t;
	static constexpr uint maxargs = 2;
	static constexpr uint maxops = 9;
	static constexpr uint maxoplvls = 8;
	static constexpr uint varnamelen = 32;
	
	struct var_t
	{
		calc_t val;
		char name[varnamelen];
		uint len;
		var_t() :
			val( 0 ),
			name{ 0 },
			len( 0 )
		{ }
		
		template <uint _len>
		static var_t from_const( calc_t _val, char const(& _name)[_len] )
		{
			static_assert( _len > 0 );
			static_assert( _len <= varnamelen );
			var_t self;
			self.val = _val;
			cpy( self.name, _name, _len );
			if( varnamelen > _len )
				zro( self.name + _len, varnamelen - _len );
			self.len = _len - 1;
			return( self );
		}
		
		static var_t from_str( calc_t _val, char const* _name, uint _len )
		{
			ENSURE( _len < varnamelen, "varible name too long (>%i: %*s)", varnamelen, _len, _name )
			var_t self;
			self.val = _val;
			cpy( self.name, _name, _len );
			if( varnamelen > _len )
				zro( self.name + _len, varnamelen - _len );
			self.len = d_strlen( _name, _len );
			return( self );
		}
	};
	
	typedef stream<var_t> varlist_t;
	
	#include "calctypes.cxx"

	struct op_val_t : public op_t
	{
		op_val_t() : op_t( "", none ) { }
		virtual calc_t calc( calc_t const(& args)[maxargs] ) const
		{
			return( args[0] );
		}
	};
	
	struct op_var_t : public op_t
	{
		op_var_t() : op_t( "", none, false ) { }
		virtual calc_t calc( calc_t const(& args)[maxargs] ) const
		{
			return( args[0] );
		}
	};
	
	struct op_smul_t : public op_t
	{
		op_smul_t() : op_t( "" ) { }
		virtual calc_t calc( calc_t const(& args)[maxargs] ) const
		{
			return( args[0] * args[1] );
		}
	};
	
	#include "calcops.cxx"

	enum err_t { err_none, err_nbracket, err_pbracket, err_notfound,
		         err_noleft, err_noright, err_anyleft, err_anyright };
	struct error_t
	{
		err_t err;
		subtext_t text;
		uint opbeg;
		uint opend;
		error_t() :
			err( err_none )
		{ }
		error_t( err_t _err, subtext_t _text, uint _opbeg, uint _opend ) :
			err( _err ),
			text( _text ),
			opbeg( _opbeg ),
			opend( _opend )
		{ }
		bool operator ==( error_t const e )
		{
			return( err == e.err &&
			        text == e.text &&
			        opbeg == e.opbeg &&
			        opend == e.opend );
		}
	};
	
	enum { op_val, op_var, op_mul };
	static constexpr uint sop_max = (uint)op_mul + 1;
	op_t * op_assign;
	class arg_t;

	text_t const text;
	ptr_arr_t<maxops, op_t> sops;
	ptr_arr_t<maxoplvls, ptr_arr_t<maxops, op_t>> ops;
	arg_t * arg;
	varlist_t vars;
	stream<error_t> errors;
	
	calculator_t( char const* _text, uint _textlen, varlist_t _vars = varlist_t() ) :
		op_assign( nullptr ),
		text( _text, _textlen ),
		arg( nullptr ),
		vars( _vars )
	{
		sops.add( new op_val_t );
		sops.add( new op_var_t );
		sops.add( new op_smul_t );
		ENSURE( sops.num == sop_max, "sops error\n" );
		mkops( ops, op_assign );
	}
	
	~calculator_t()
	{
		if( arg )
			delete arg;
	}
	
	bool parse()
	{
		arg = new arg_t( text, sops, ops, op_assign, vars, errors );
		ENSURE( arg, "oom\n" );
		if( !arg->check_brackets() || !arg->parse() )
		{
			delete arg;
			arg = nullptr;
		}
		return( arg );
	}
	
	void print() const
	{
		if( !arg )
			return;
		arg->print();
	}
	
	calc_t calc()
	{
		if( !arg )
			return( 0 );
		return( arg->calc() );
	}

	void print_errors()
	{
		if( !arg )
		{
			d_fprint_err( "\n"
			              "Could not parse input. The follow-\n"
						  "ing list of errors might help you\n"
						  "finding your desired expression:\n\n" );
		}
		for( uint i = 0; i < errors.num(); i++ )
		{
			error_t e = errors.getval( i );
			text_t tot = e.text.total();
			switch( e.err )
			{
				case err_none:
					break;
				case err_nbracket:
					d_fprint_err( "excessive bracket:\n" );
					break;
				case err_pbracket:
					d_fprint_err( "unclosed bracket:\n" );
					break;
				case err_notfound:
					d_fprint_err( "no operator found:\n" );
					break;
				case err_noleft:
					d_fprint_err( "left operand not given:\n" );
					break;
				case err_noright:
					d_fprint_err( "right operand not given:\n" );
					break;
				case err_anyleft:
					d_fprint_err( "left operand given:\n" );
					break;
				case err_anyright:
					d_fprint_err( "right operand given:\n" );
					break;
			}
			d_fprint_err( "%*s\n", tot.len, tot.str );
			
			d_fprint_err( "%0,*s", e.text.off + e.opbeg, "" );
			d_fprint_err( "%c", '^' );
			uint j;
			for( j = e.opbeg + 1; j < e.opend - 1; j++ )
				d_fprint_err( "%c", '~' );
			if( j <= e.opend - 1 )
				d_fprint_err( "%c", '^' );
			d_fprint_err( "%0,*s", e.text.tot - ( e.text.off + j ), "" );
			d_fprint_err( "\n" );
			
			d_fprint_err( "%0,*s", e.text.off, "" );
			d_fprint_err( "%c", '^' );
			for( j = 1; j < e.text.len - 1; j++ )
				d_fprint_err( "%c", '~' );
			if( j <= e.text.len - 1 )
				d_fprint_err( "%c", '^' );
			d_fprint_err( "%0,*s", e.text.tot - ( e.text.off + j ), "" );
			d_fprint_err( "\n" );
			
			d_fprint_err( "\n" );
		}
	}
	
	varlist_t const& getvars() const
	{
		return( vars );
	}

	#include "calcarg.cxx"

	static bool eval( calc_t & result, char const* str, uint len = -1,
	                  varlist_t vars = varlist_t() )
	{
		calculator_t calc( str, d_strlen( str, len ), vars );
		if( !calc.parse() )
		{
			calc.print_errors();
			d_fprint_err( "calculation failed\n\n" );
			return( false );
		}
		result = calc.calc();
		return( true );
	}
};

#endif // CALC_INCLUDED

