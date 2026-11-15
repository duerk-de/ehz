#ifndef CALCARG_INCLUDED
#define CALCARG_INCLUDED

	struct arg_t
	{
		ptr_arr_t<maxops, op_t> const& sops;
		ptr_arr_t<maxoplvls, ptr_arr_t<maxops, op_t>> const& ops;
		subtext_t const text;
		op_t const* op_assign;

		op_t const* op;
		bool invariant;
		calc_t val;
		var_t * var;
		ptr_arr_t<maxargs, arg_t> args;
		stream<var_t> & vars;
		stream<error_t> & errors;
		
		static constexpr bool dbg = false;
		
		bool valid;
		uint lvl;

		arg_t( subtext_t _text,
		       ptr_arr_t<maxops, op_t> const& _sops,
		       ptr_arr_t<maxoplvls, ptr_arr_t<maxops, op_t>> const& _ops,
		       op_t const* _op_assign,
		       stream<var_t> & _vars,
		       stream<error_t> & _errors, uint _lvl = 0 ) :
			sops( _sops ),
			ops( _ops ),
			text( trim( _text ) ),
			op_assign( _op_assign ),
			op( nullptr ),
			invariant( false ),
			val( 0 ),
			var( nullptr ),
			vars( _vars ),
			errors( _errors ),
			valid( false ),
			lvl( _lvl )
		{
			if( dbg && !lvl )
				d_fprint( ":\n" );
		}
		
		void print( uint lvl = 0, uint i = 0 ) const
		{
			if( i )
				d_fprint( "%0,*s", lvl + 1, "" );
			if( !op )
			{
				d_fprint( "error: %*s\n", text.len, text.str );
				return;
			}
			uint len;
			if( op == sops[op_val] )
				len = d_fprint( "%*s ", text.len, text.str );
			else if( op == sops[op_var] )
				len = d_fprint( ":%*s ", text.len, text.str );
			else
				len = d_fprint( "%*s: ", op->text.len, op->text.str );
			for( uint i = 0; i < args.num; i++ )
			{
				if( i )
					d_fprint( "\n" );
				args[i]->print( lvl + len, i );
			}
			if( !lvl )
				d_fprint( "\n" );
		}
		
		calc_t calc()
		{
			calc_t result;
			if( !invariant )
			{
				invariant = op->invariant;
				calc_t argvals[maxargs];
				if( op == sops[op_val] )
				{
					argvals[0] = val;
				} else
				if( op == sops[op_var] )
				{
					argvals[0] = var->val;
				} else {
					for( uint i = 0; i < args.num; i++ )
					{
						argvals[i] = args[i]->calc();
						invariant = invariant && args[i]->invariant;
					}
				}
				result = op->calc( argvals );
				if( invariant )
					val = result;
				if( op == op_assign )
					args[0]->var->val = result;
			} else {
				if( op == op_assign )
					args[0]->var->val = val;
				result = val;
			}
			return( result );
		}
		
		bool parse()
		{
			valid = parse_internal();
			if( !valid )
				return( false );
			addvars();
			setvars();
			return( true );
		}
			
		bool check_brackets()
		{
			uint lnpos = 0;
			uint npos = 0;
			uint blvl = 0;
			for( uint i = 0; i < text.len; i++ )
			{
				if( !blvl )
					npos = i;
				switch( text.str[i] )
				{
					case op_t::bbeg:
						if( !blvl )
							lnpos = i;
						blvl++;
						break;
					case op_t::bend:
						if( !blvl )
						{
							add_error( err_nbracket, text, lnpos, i + 1 );
							return( false );
						}
						blvl--;
						break;
				}
			}
			if( blvl )
			{
				add_error( err_pbracket, text, npos, text.len );
				return( false );
			}
			return( true );
		}

		void addvars()
		{
			if( op == sops[op_var] )
			{
				if( var )
					var = nullptr;
				if( !getvarptr( text ) )
					vars.add( var_t::from_str( 0, text.str, text.len ) );
			}
			for( uint i = 0; i < args.num; i++ )
				args[i]->addvars();
		}
		
		void setvars()
		{
			if( op == sops[op_var] )
			{
				if( !var )
					var = getvarptr( text );
				ENSURE( var, "variable set\n" );
			}
			for( uint i = 0; i < args.num; i++ )
				args[i]->setvars();
		}
		
		void add_error( err_t e, subtext_t text, uint opbeg, uint opend )
		{
			error_t error = error_t( e, text, opbeg, opend );
			bool add = true;
			for( uint i = 0; i < errors.num(); i++ )
				if( error == errors.getval( i ) )
					add = false;
			if( add )
				errors.add( error );
		}

		bool get_opargs( op_t * op, uint textpos )
		{
			arg_t * larg = new arg_t( subtext_t( text, 0, textpos ),
			                          sops, ops, op_assign, vars, errors, lvl + 1 );
			arg_t * rarg = new arg_t( subtext_t( text, textpos + op->text.len, text.len ),
			                          sops, ops, op_assign, vars, errors, lvl + 1 );
			ENSURE( larg, "oom\n" );
			ENSURE( rarg, "oom\n" );
			if( op == op_assign && isvarname( larg->text ) )
			{
				larg->getvar( larg->text );
				larg->op = sops[op_var];
				larg->valid = true;
			} else {
				larg->parse_internal();
			}
			rarg->parse_internal();
			bool valid = true;
			switch( op->type )
			{
				case op->right:
					if( larg->text.len )
					{
						valid = false;
						add_error( err_anyleft, text, textpos, op->text.len );
					}
					if( !larg->valid && rarg->valid )
					{
						args.move( rarg );
					} else {
						valid = false;
					}
					break;
				case op->left:
					if( rarg->text.len )
					{
						valid = false;
						add_error( err_anyright, text, textpos, op->text.len );
					}
					if( !rarg->valid && larg->valid )
					{
						args.move( larg );
					} else {
						valid = false;
					}
					break;
				case op->middle:
					if( !larg->text.len )
					{
						valid = false;
						add_error( err_noleft, text, textpos, op->text.len );
					}
					if( !rarg->text.len )
					{
						valid = false;
						add_error( err_noright, text, textpos, op->text.len );
					}
					if( valid && larg->valid && rarg->valid )
					{
						args.move( larg );
						args.move( rarg );
					} else {
						valid = false;
					}
					break;
				default:
					valid = false;
					ENSURE( false, "unsupported argument type: %i\n",
							op->type );
					break;
			}
			delete rarg;
			delete larg;
			return( valid );
		}

		bool getlvlop_atpos( ptr_arr_t<maxops, op_t> const& lvlops, uint pos, bool & found )
		{
			uint plen = text.len - pos;
			for( uint iop = 0; iop < lvlops.num; iop++ )
			{
				op_t * op = lvlops[iop];
				if( op->text.len <= plen &&
					d_memcmp( text.str + pos, op->text.str, op->text.len ) )
				{
					found = true;
					if( get_opargs( op, pos ) )
					{
						this->op = op;
						return( true );
					}
				}
			}
			return( false );
		}

		bool getlvlop( ptr_arr_t<maxops, op_t> const& lvlops, bool & found )
		{
			uint blvl = 0;
			for( uint in = text.len; in; in-- )
			{
				uint i = in - 1;
				switch( text.str[i] )
				{
					case op_t::bbeg:
						ENSURE( blvl, "bracket level underflow in @ %i\n%*s\n%0,*s^\n",
									  i + 1, text.len, text.str, i, "" );
						blvl--;
						break;
					case op_t::bend:
						blvl++;
						break;
					default:
						if( blvl == 0 )
							if( getlvlop_atpos( lvlops, i, found ) )
								return( true );
						break;
				}
			}
			return( false );
		}

		bool getop()
		{
			bool found = false;
			for( uint ioplvl = 0; ioplvl < ops.num; ioplvl++ )
			{
				if( getlvlop( *ops[ioplvl], found ) )
				{
					if( dbg )
						d_fprint( "%0,*s:op_%i @ %i\n", lvl, "", ioplvl, args[0] ? args[0]->text.len : -1 );
					return( true );
				}
			}
			if( dbg )
				d_fprint( "%0,*s:no_op (found=%b)\n", lvl, "", found );

			if( !found )
			{
				uint num;
				str2doublel( text.str, 0, text.len, num );
				if( num > 0 )
				{
					op_t * op = sops[op_mul];
					found = true;
					arg_t * larg = new arg_t( subtext_t( text, 0, num ),
					                          sops, ops, op_assign, vars, errors, lvl + 1 );
					arg_t * rarg = new arg_t( subtext_t( text, num, text.len ),
					                          sops, ops, op_assign, vars, errors, lvl + 1 );
					ENSURE( larg, "oom\n" );
					ENSURE( rarg, "oom\n" );
					ENSURE( op->type == op->middle, "op_mul type" );
					larg->parse_internal();
					rarg->parse_internal();
					if( larg->valid && rarg->valid )
					{
						args.move( larg );
						args.move( rarg );
						this->op = op;
						return( true );
					}
					delete rarg;
					delete larg;
				}
			}

			if( !found )
				add_error( err_notfound, text, 0, text.len );
			return( false );
		}

		bool getsop()
		{
			if( getval( text, val ) )
			{
				if( dbg )
					d_fprint( "%0,*s:op_val\n", lvl + 1, "" );
				op = sops[op_val];
				return( true );
			}
			if( getvar( text ) )
			{
				if( dbg )
					d_fprint( "%0,*s:op_var\n", lvl + 1, "" );
				op = sops[op_var];
				return( true );
			}
			return( false );
		}

		bool getanyop()
		{
			if( !text.len )
				return( false );
			if( dbg )
				d_fprint( "%0,*s%*s\n", lvl + 1, "", text.len, text.str );
			if( getsop() )
				return( true );
			return( getop() );
		}

		bool parse_internal()
		{
			valid = getanyop();
			if( !valid )
				return( false );
			ENSURE( op, "operator nullptr\n" );
			return( true );
		}
			
		
		static bool getval( text_t text, calc_t & val )
		{
			uint num;
			calc_t v = str2doublel( text.str, 0, text.len, num );
			if( (uint)num != text.len )
				return( false );
			val = v;
			return( true );
		}
		
		static bool isvarname( text_t text )
		{
			if( !text.len )
				return( false );
			uint i = 0;
			if( !op_t::varbegchar( text.str[i++] ) )
				return( false );
			while( i < text.len )
				if( !op_t::varchar( text.str[i++] ) )
					return( false );
			return( true );
		}
		
		var_t * getvarptr( text_t text )
		{
			for( uint i = 0; i < vars.num(); i++ )
			{
				var_t * ivar = vars.get( i );
				if( ( ivar->len == text.len ) &&
				    d_memcmp( ivar->name, text.str, text.len ) )
				    return( ivar );
			}
			return( nullptr );
		}
		
		bool getvar( text_t text )
		{
			if( !isvarname( text ) )
				return( false );
			var_t * ivar = getvarptr( text );
			if( !ivar )
				return( false );
			var = ivar;
			return( true );
		}
		
		static void trim_raw( char const* str, uint & beg, uint & end )
		{
			for( bool finished = false; !finished; )
			{
				finished = true;
				while( beg < end && str[beg] == op_t::space )
				{
					beg++;
					finished = false;
				}
				while( beg < end && str[end-1] == op_t::space )
				{
					end--;
					finished = false;
				}
				if( beg + 1 < end && str[beg] == op_t::bbeg && str[end-1] == op_t::bend )
				{
					bool c_b_valid = true;
					for( uint i = beg + 1, blvl = 0; i < end - 1 && c_b_valid; i++ )
					{
						switch( str[i] )
						{
							case op_t::bbeg:
								blvl++;
								break;
							case op_t::bend:
								if( blvl )
									blvl--;
								else
									c_b_valid = false;
								break;
						}
					}
					if( c_b_valid )
					{
						beg++;
						end--;
						finished = false;
					}
				}
			}
		}
		
		static subtext_t trim( subtext_t text )
		{
			uint beg = 0;
			uint end = text.len;
			trim_raw( text.str, beg, end );
			if( beg > end )
				beg = end;
			return( subtext_t( text, beg, end ) );
		}
		
		static bool isempty( text_t text )
		{
			uint beg = 0;
			uint end = text.len;
			trim_raw( text.str, beg, end );
			return( beg == end );
		}
	};

#endif // CALCARG_INCLUDED

