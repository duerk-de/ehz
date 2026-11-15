#ifndef CALCOPS_INCLUDED
#define CALCOPS_INCLUDED

	#define fnc( v ) \
		virtual calc_t calc( calc_t const(& args)[maxargs] ) const \
		{ \
			return( v ); \
		}

	
	struct op_sep_t : public op_t
	{
		op_sep_t() : op_t( "," ) { }
		fnc( args[1] );
	};
	
	struct op_assign_t : public op_t
	{
		op_assign_t() : op_t( "=" ) { }
		fnc( args[1] );
	};
	
	struct op_less_t : public op_t
	{
		op_less_t() : op_t( "<" ) { }
		fnc( args[0] < args[1] ? 1 : 0 );
	};
	
	struct op_more_t : public op_t
	{
		op_more_t() : op_t( ">" ) { }
		fnc( args[0] > args[1] ? 1 : 0 );
	};
	
	struct op_add_t : public op_t
	{
		op_add_t() : op_t( "+" ) { }
		fnc( args[0] + args[1] );
	};

	struct op_sub_t : public op_t
	{
		op_sub_t() : op_t( "-" ) { }
		fnc( args[0] - args[1] );
	};

	struct op_mul_t : public op_t
	{
		op_mul_t() : op_t( "*" ) { }
		fnc( args[0] * args[1] );
	};

	struct op_div_t : public op_t
	{
		op_div_t() : op_t( "/" ) { }
		fnc( args[1] ? args[0] / args[1] : 0 );
	};
	
	struct op_max_t : public op_t
	{
		op_max_t() : op_t( "maximum" ) { }
		fnc( d_max( args[0], args[1] ) );
	};

	struct op_min_t : public op_t
	{
		op_min_t() : op_t( "minimum" ) { }
		fnc( d_min( args[0], args[1] ) );
	};

	struct op_neg_t : public op_t
	{
		op_neg_t() : op_t( "-", right ) { }
		fnc( - args[0] );
	};

	struct op_sqrt_t : public op_t
	{
		op_sqrt_t() : op_t( "squareroot", right ) { }
		fnc( math::sqrt_heron( args[0], args[0] ) );
	};

	struct op_sin_t : public op_t
	{
		op_sin_t() : op_t( "sin", right ) { }
		fnc( d_sin( args[0] ) );
	};

	struct op_cos_t : public op_t
	{
		op_cos_t() : op_t( "cos", right ) { }
		fnc( d_sin( args[0] ) );
	};

	struct op_arctan_t : public op_t
	{
		op_arctan_t() : op_t( "arctan", right ) { }
		fnc( d_arctan( args[0] ) );
	};

	struct op_timezone_t : public op_t
	{
		op_timezone_t() : op_t( "timezoneof", right ) { }
		fnc( timezone( args[0] ) );
		static int timezone( calculator_t::calc_t time )
		{
			timefclass t( time );
			return( t.timezone + t.summertime );
		}
	};

	struct op_percent_t : public op_t
	{
		op_percent_t() : op_t( "%", left ) { }
		fnc( args[0] * 1e-2 );
	};

	static void mkops( ptr_arr_t<maxoplvls, ptr_arr_t<maxops, op_t>> & ops,
	                   op_t *& op_assign )
	{
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_sep_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( op_assign = new op_assign_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_less_t );
		ops.last()->add( new op_more_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_add_t );
		ops.last()->add( new op_sub_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_mul_t );
		ops.last()->add( new op_div_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_max_t );
		ops.last()->add( new op_min_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_neg_t );
		ops.last()->add( new op_sqrt_t );
		ops.last()->add( new op_sin_t );
		ops.last()->add( new op_cos_t );
		ops.last()->add( new op_arctan_t );
		ops.last()->add( new op_timezone_t );
		ops.add( new ptr_arr_t<maxops, op_t> );
		ops.last()->add( new op_percent_t );
	}


#endif // CALCOPS_INCLUDED

