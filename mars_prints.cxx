#ifndef MARSPRINTS_INCLUDED
#define MARSPRINTS_INCLUDED

		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> mars\n"
		                       "out             :%3,6f W\n"
		                       "external        :%3,6f W\n"
		                       "offset          :%3,6f W\n"
		                       "error           :%3,6f W\n"
		                       "setpoint        :%3,6f W\n"
		                       "full power      :%3,6f W\n"
		                       "full power off  :%3,6f W\n"
		                       "empty power     :%3,6f W\n"
		                       "low power       :%3,6f W\n"
		                       "limited         :     %b\n"
		                       "state           :      %*s\n"
		                       "pack power diff :%3,6f W\n"
		                       "pack power      :%3,6f W\n"
		                       "pack voltage    :%3,6f V\n"
		                       "pack current    :%3,6f A\n"
		                       "cell volt max   :%6,6f V\n"
		                       "cell volt avg   :%6,6f V\n"
		                       "cell volt min   :%6,6f V\n"
		                       "cell volt diff  :%6,6f V\n"
		                       "cell temp       :%3,6f °C\n"
		                       "level           :%3,6f %%\n"
		                       "voltlevel       :%3,6f %%\n"
		                       "voltlevel now   :%3,6f %%\n"
		                       "gen voltage     :%3,6f V\n"
		                       "cell volt max   :%6,6F V\n" 
		                       "cell volt       :%6,6F V\n" 
		                       "cell volt min   :%6,6F V\n" 
		                       "cell volt diff  :%6,6F V\n",
		                       data.out,
		                       data.external,
		                       data.offset,
		                       data.error,
		                       data.setpoint,
		                       data.full_power,
		                       data.full_power_offset,
		                       data.empty_power,
		                       data.low_power,
		                       data.limited,
		                       (int)sizeof(state.state_names[state.state]),
		                       state.state_names[state.state],
		                       data.powerdiff,
		                       data.pack_power,
		                       data.pack_voltage,
		                       data.pack_current,
		                       data.cell_voltage_max,
		                       data.cell_voltage,
		                       data.cell_voltage_min,
		                       data.balance.cell_voltage_max - data.balance.cell_voltage_min,
		                       data.temp,
		                       data.level,
		                       data.voltlevel,
		                       data.voltlevel_current,
		                       data.gen_voltage,
		                       stat.cell_voltage_max,
		                       stat.cell_voltage,
		                       stat.cell_voltage_min,
		                       stat.cell_voltage_max - stat.cell_voltage_min );
/*
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "dc_voltage      : %9,1i dV\n"
		                       "power           : %9,1i W\n"
		                       "temp_tot        : %9,1i d°C\n"
		                       "temp_mos        : %9,1i d°C\n"
		                       "ac_voltage      : %9,1i dV\n",
		                       main.data.name, 
		                       (int)fe( main.data.data().dc_voltage ),
		                       -(int)(short)fe( main.data.data().power ),
		                       (int)fe( main.data.data().temp_tot ),
		                       (int)fe( main.data.data().temp_mos ),
		                       (int)fe( main.data.data().ac_voltage ) );
*/
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "state           : %9,1i (%s)\n",
		                       imode.data.name, 
		                       (int)fe<ushort>( imode.data.data().state ),
		                       imode.data.data().state_str( imode.data.data().state ) );
/*
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "voltage         : %9,1i cV\n"
		                       "current         : %9,1i A\n"
		                       "voltage_max     : %9,1i mV\n"
		                       "voltage_min     : %9,1i mV\n"
		                       "temp_max        : %9,1i c°C\n"
		                       "temp_min        : %9,1i c°C\n"
		                       "reserved1       : %9,1i\n"
		                       "reserved2       : %9,1i\n"
		                       "reserved3       : %9,1i\n"
		                       "reserved4       : %9,1i\n"
		                       "reserved5       : %9,1i\n",
		                       cell_deprecated.data.name, 
		                       (int)fe( cell_deprecated.data.data().voltage ),
		                       (int)(short)fe( cell_deprecated.data.data().current ),
		                       (int)fe( cell_deprecated.data.data().voltage_max ),
		                       (int)fe( cell_deprecated.data.data().voltage_min ),
		                       (int)fe( cell_deprecated.data.data().temp_max ),
		                       (int)fe( cell_deprecated.data.data().temp_min ),
		                       (int)fe( cell_deprecated.data.data().reserved1 ),
		                       (int)fe( cell_deprecated.data.data().reserved2 ),
		                       (int)fe( cell_deprecated.data.data().reserved3 ),
		                       (int)fe( cell_deprecated.data.data().reserved4 ),
		                       (int)fe( cell_deprecated.data.data().reserved5 ) );
*/
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       /*
		                       "voltage         : %9,1i cV\n"
		                       "current         : %9,1i\n"
		                       "power           : %9,1i\n"
		                       "soc             : %9,1i %%\n"
		                       "max_energy      : %9,1i kWh\n"
		                       */
		                       "charge_limit    : %9,1i e-3\n"
		                       "discharge_limit : %9,1i e-3\n"
		                       /*
		                       "temp3           : %9,1i\n"
		                       "reserved1       : %9,1i\n"
		                       "reserved2       : %9,1i\n"
		                       "reserved3       : %9,1i\n"
		                       */
		                       ,
		                       pack.data.name,
		                       /*
		                       (int)fe( pack.data.data().voltage ),
		                       (int)(short)fe( pack.data.data().current ),
		                       (int)fe( pack.data.data().power ),
		                       (int)fe( pack.data.data().soc ),
		                       (int)fe( pack.data.data().max_energy ),
		                       */
		                       (int)fe( pack.data.data().charge_limit ),
		                       (int)fe( pack.data.data().discharge_limit )
		                       /*
		                       ,
		                       (int)fe( pack.data.data().temp3 ),
		                       (int)fe( pack.data.data().reserved1 ),
		                       (int)fe( pack.data.data().reserved2 ),
		                       (int)fe( pack.data.data().reserved3 )
		                       */
		                       );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       /*
		                       "voltage         : %9,1i cV\n"
		                       "current         : %9,1i A\n"
		                       */
		                       "soc             : %9,1i e-3\n"
		                       "cycles          : %9,1i\n"
		                       "full            : %9,1i (2=full)\n"
		                       /*
		                       "voltage_max     : %9,1i mV\n"
		                       "voltage_min     : %9,1i mV\n"
		                       "reserved1       : %9,1i\n"
		                       "reserved2       : %9,1i\n"
		                       */
		                       "hivolt          : %9,1i (4=hi)\n"
		                       /*
		                       "reserved3       : %9,1i\n"
		                       */
		                       "temp1           : %9,1i d°C\n"
		                       "temp2           : %9,1i d°C\n"
		                       "temp3           : %9,1i d°C\n"
		                       "temp4           : %9,1i d°C\n"
		                       "temp5           : %9,1i d°C\n"
		                       "temp6           : %9,1i d°C\n"
		                       "reserved4       : %9,1i\n"
		                       /*
		                       "reserved5       : %9,1i mV\n"
		                       */
		                       ,
		                       cell.data.name, 
		                       /*
		                       (int)fe( cell.data.data().voltage ),
		                       (int)(short)fe( cell.data.data().current ),
		                       */
		                       (int)fe( cell.data.data().soc ),
		                       (int)fe( cell.data.data().cycles ),
		                       (int)fe( cell.data.data().full ),
		                       /*
		                       (int)fe( cell.data.data().voltage_max ),
		                       (int)fe( cell.data.data().voltage_min ),
		                       (int)fe( cell.data.data().reserved1 ),
		                       (int)fe( cell.data.data().reserved2 ),
		                       */
		                       (int)fe( cell.data.data().hivolt ),
		                       /*
		                       (int)fe( cell.data.data().reserved3 ),
		                       */
		                       (int)fes( cell.data.data().temp1 ),
		                       (int)fes( cell.data.data().temp2 ),
		                       (int)fes( cell.data.data().temp3 ),
		                       (int)fes( cell.data.data().temp4 ),
		                       (int)fes( cell.data.data().temp5 ),
		                       (int)fes( cell.data.data().temp6 ),
		                       (int)fes( cell.data.data().reserved4 )
		                       /*
		                       ,
		                       (int)fe( cell.data.data().reserved5 )
		                       */
		                       );
/*
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "voltage         : %9,1i dV\n"
		                       "voltage2        : %9,1i dV\n"
		                       "power           : %9,1i W\n"
		                       "frequency       : %9,1i dHz\n",
		                       ac.data.name, 
		                       (int)fe( ac.data.data().voltage ),
		                       (int)fe( ac.data.data().voltage2 ),
		                       (int)fe( ac.data.data().power ),
		                       (int)fe( ac.data.data().frequency ) );
*/
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "voltage         : %9,1i V\n"
		                       "power           : %9,1i W\n",
		                       offgrid.data.name, 
		                       (int)fe( offgrid.data.data().voltage ),
		                       (int)(short)fe( offgrid.data.data().power ) );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "charged         : %9,1i\n"
		                       "discharged      : %9,1i\n",
		                       meters.data.name, 
		                       (int)fe( meters.data.data().charged ),
		                       (int)fe( meters.data.data().discharged ) );
/*
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "tot             : %9,1i d°C\n"
		                       "mos1            : %9,1i d°C\n"
		                       "mos2            : %9,1i d°C\n"
		                       "batt_max        : %9,1i d°C\n"
		                       "batt_min        : %9,1i d°C\n",
		                       temp1.data.name, 
		                       (int)fe( temp1.data.data().tot ),
		                       (int)fe( temp1.data.data().mos1 ),
		                       (int)fe( temp1.data.data().mos2 ),
		                       (int)fe( temp2.data.data().batt_max ),
		                       (int)fe( temp2.data.data().batt_min ) );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "voltage         : %9,1i d°C\n"
		                       "charge_current  : %9,1i d°C\n"
		                       "discharg_current: %9,1i d°C\n",
		                       limits.data.name, 
		                       (int)fe( limits.data.data().voltage ),
		                       (int)fe( limits.data.data().charge_current ),
		                       (int)fe( limits.data.data().discharge_current ) );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> %S\n"
		                       "ctrl_enable     : %9,1,16i\n"
		                       "ctrl_mode       : %9,1i\n"
		                       "ctrl_amount     : %9,1i\n"
		                       "ctrl_amount     : %9,1i\n",
		                       ctrl_enable.data.name, 
		                       (int)fe<ushort>( ctrl_enable.data.data().enabled ),
		                       (int)fe<ushort>( ctrl_mode.data.data().state ),
		                       (int)fe( ctrl_amount.data.data().charge ),
		                       (int)fe( ctrl_amount.data.data().discharge ) );
		rawbuf.len += d_fstrn( rawbuf.buf + rawbuf.len, rawbuf.max - rawbuf.len,
		                       "> algorithm\n"
		                       "generator       : %9,1I mW\n"
		                       "solar           : %9,1I mW\n"
		                       "external        : %9,1I mW\n"
		                       "consumption     : %9,1I mW\n"
		                       "battery         : %9,1I mW\n"
		                       "solar_max       : %9,1I mW\n"
		                       "setpoint_max    : %9,1I mW\n",
		                       generator,
		                       solar,
		                       external,
		                       consumption,
		                       battery,
		                       solar_max,
		                       max_generator - solar_max );
*/


#endif // MARSPRINTS_INCLUDED

