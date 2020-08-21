#include "stdafx.h"
#include "Gcode.h"
#include "TheMachine.h"

float GCodeParser::Last_Feedrate	=  NAN;
GcodeStatus GCodeParser::Last_State = { 0 };

float GCodeParser::Last_Position[N_AXIS]		  = { 0 };
float GCodeParser::Global_Offset[N_AXIS]	      = { 0 };
float GCodeParser::Predefine_Position_G28[N_AXIS] = { 0 };
float GCodeParser::Predefine_Position_G30[N_AXIS] = { 0 };
float GCodeParser::Coord_System_Offset[N_AXIS][NUMBER_COORDINATE_OFFSET - 1] = { 0 };

size_t GCodeParser::Proccess_Gcode(char* &LINE)
{
	if (*LINE == 0) return STATUS_GCODE_LINE_EMPTY;

	size_t words_activated = 0;
	size_t word = 0;
	memset(&state, 0, sizeof(GcodeStatus));
	memset(&value, 0, sizeof(GcodeValues));

	state   = Last_State;
	value.f = Last_Feedrate;

	while (*LINE)
	{
		char CH;
		Number NUM;

		if (*LINE == ';' || *LINE == '/') break; //Despues del comentario se ignora toda la linea.
		if (Read_Letter_And_Number(LINE, CH, NUM))
		{
			switch (CH) {
			case 'R': word = WORD_R; value.r = NUM.float_num; break;
			case 'I': word = WORD_I; value.ijk[X_AXIS] = NUM.float_num; break;
			case 'J': word = WORD_J; value.ijk[Y_AXIS] = NUM.float_num; break;
			case 'K': word = WORD_K; value.ijk[Z_AXIS] = NUM.float_num; break;
			case 'X': word = WORD_X; value.xyz[X_AXIS] = NUM.float_num; break;
			case 'Y': word = WORD_Y; value.xyz[Y_AXIS] = NUM.float_num; break;
			case 'Z': word = WORD_Z; value.xyz[Z_AXIS] = NUM.float_num; break;

			case 'F': if (NUM.have_a_negative_sign) return STATUS_GCODE_FEEDRATE_IS_NEGATIVE;
				word = WORD_F; value.f = NUM.float_num; break;
			case 'P': if (NUM.have_a_negative_sign) return STATUS_BAD_P_IS_NEGATIVE;
				word = WORD_P; value.p = NUM.float_num; break;
			case 'S': if (NUM.have_a_negative_sign) return STATUS_GCODE_SPINDLE_IS_NEGATIVE;
				word = WORD_S; value.s = NUM.float_num; break;
			case 'L':
				if (NUM.have_a_negative_sign)						 return STATUS_BAD_L_NUMBER_NEGATIVE;
				if (NUM.have_a_decimal_part)						 return STATUS_BAD_L_NUMBER_NOT_INTEGER;
				if (NUM.integer_part != 2 || NUM.integer_part != 20) return STATUS_BAD_L_NUMBER;
				word = WORD_L; value.l = NUM.integer_part; break;
				break;
			case 'T':
				if (NUM.have_a_negative_sign)			return STATUS_BAD_TOOL_NUMBER_BEGATIVE;
				if (NUM.have_a_decimal_part)			return STATUS_BAD_TOOL_NUMBER_NOT_INTEGER;
				if (NUM.integer_part > MAX_TOOL_NUMBER) return STATUS_BAD_TOOL_NUMBER_EXCEEDED;
				word = WORD_T; value.s = NUM.float_num; break;
			case 'N':
				if (NUM.have_a_positive_sign)			return STATUS_BAD_LINE_NUMBER_FORMAT;
				if (NUM.have_a_negative_sign)			return STATUS_GCODE_LINE_NUMBER_IS_NEGATIVE;
				if (NUM.have_a_decimal_part)			return STATUS_GCODE_INVALID_LINE_NUMBER;
				if (NUM.integer_part > MAX_LINE_NUMBER) return STATUS_LINE_LENGTH_EXCEEDED;
				word = WORD_N; value.n = NUM.integer_part; break;

			case 'M':
				if (NUM.have_a_decimal_part) return STATUS_GCODE_UNSUPPORTED_COMMAND;// [No Mxx.x commands]
				switch (NUM.integer_part) {
				case 0:  word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_PAUSED; break;
				case 1:  word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_OPTIONAL_STOP; break;
				case 2:  word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_COMPLETED_M2; break;
				case 30: word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_COMPLETED_M30; break;
				case 3:  word = MODAL_GROUP_M7; state.spindle = SPINDLE_ENABLE_CW; break;
				case 4:  word = MODAL_GROUP_M7; state.spindle = SPINDLE_ENABLE_CCW; break;
				case 5:  word = MODAL_GROUP_M7; state.spindle = SPINDLE_DISABLE; break;
				case 7:  word = MODAL_GROUP_M8; state.coolant = COOLANT_MIST_ENABLE; break;
				case 8:  word = MODAL_GROUP_M8; state.coolant = COOLANT_FLOOD_ENABLE; break;
				case 9:  word = MODAL_GROUP_M8; state.coolant = COOLANT_DISABLE; break;
				case 56: word = MODAL_GROUP_M9; state.override = OVERRIDE_PARKING_MOTION; break;
				default: return STATUS_GCODE_UNSUPPORTED_COMMAND;
				}

			case 'G':
				if (NUM.have_a_decimal_part) {
					if (NUM.integer_part == 28 && NUM.decimal_part == 1) { //G28.1
						word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_SET_HOME_0;
					}
					if (NUM.integer_part == 30 && NUM.decimal_part == 1) { //G30.1
						word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_SET_HOME_1;
					}
					if (NUM.integer_part == 92 && NUM.decimal_part == 1) { //G92.1
						word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_RESET_COORDINATE_OFFSET;
					}
					if (NUM.integer_part == 43 && NUM.decimal_part == 1) { //G43.1
						word = MODAL_GROUP_G8; state.tool_length = TOOL_LENGTH_OFFSET_ENABLE_DYNAMIC;
					}
					else return STATUS_GCODE_UNSUPPORTED_COMMAND;// [Invalid Gxx.x commands]
				}
				else {
					switch (NUM.integer_part) {
					case 4:  word = (MODAL_GROUP_G0 | NEED_PARAMETERS); state.non_modal = NON_MODAL_DWELL; break;
					case 10: word = (MODAL_GROUP_G0 | NEED_PARAMETERS); state.non_modal = NON_MODAL_SET_COORDINATE_DATA; break;
					case 28: word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_GO_HOME_0; break;
					case 30: word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_GO_HOME_1; break;
					case 53: word = (MODAL_GROUP_G0 | NEED_PARAMETERS);	state.non_modal = NON_MODAL_ABSOLUTE_COORDINATE; break;
					case 92: word = (MODAL_GROUP_G0 | NEED_PARAMETERS); state.non_modal = NON_MODAL_SET_COORDINATE_OFFSET; break;
					case 0:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_SEEK; break;
					case 1:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_LINEAR; break;
					case 2:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_CW_ARC; break;
					case 3:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_CCW_ARC; break;
					case 80: word = MODAL_GROUP_G1; state.motion = MOTION_MODE_NONE; break;
					case 17: word = MODAL_GROUP_G2;	state.plane_select = PLANE_SELECT_XY; break;
					case 18: word = MODAL_GROUP_G2;	state.plane_select = PLANE_SELECT_ZX; break;
					case 19: word = MODAL_GROUP_G2;	state.plane_select = PLANE_SELECT_YZ; break;
					case 90: word = MODAL_GROUP_G3;	state.distance = DISTANCE_MODE_ABSOLUTE; break;
					case 91: word = MODAL_GROUP_G3;	state.distance = DISTANCE_MODE_INCREMENTAL;	break;
					case 93: word = MODAL_GROUP_G5;	state.feed_rate = FEED_RATE_MODE_INVERSE_TIME; break;
					case 94: word = MODAL_GROUP_G5;	state.feed_rate = FEED_RATE_MODE_UNITS_PER_MIN;	break;
					case 20: word = MODAL_GROUP_G6;	state.units = UNITS_MODE_INCHES; break;
					case 21: word = MODAL_GROUP_G6;	state.units = UNITS_MODE_MM; break;
					case 40: word = MODAL_GROUP_G7; state.radius_compensation = CUTTER_COMP_DISABLE; break;
					case 49: word = MODAL_GROUP_G8; state.tool_length = TOOL_LENGTH_OFFSET_CANCEL; break;
					case 54: word = MODAL_GROUP_G12; state.coord_select = G54; break;
					case 55: word = MODAL_GROUP_G12; state.coord_select = G55; break;
					case 56: word = MODAL_GROUP_G12; state.coord_select = G56; break;
					case 57: word = MODAL_GROUP_G12; state.coord_select = G57; break;
					case 58: word = MODAL_GROUP_G12; state.coord_select = G58; break;
					case 59: word = MODAL_GROUP_G12; state.coord_select = G59; break;
					default: return STATUS_GCODE_UNSUPPORTED_COMMAND;
					}
				}
				break;
				// Other Letters
			default:
				return STATUS_GCODE_UNSUPPORTED_COMMAND;
			}
		}
		else return STATUS_INVALID_WORD_FORMAT;

		if (bit_istrue(words_activated, word)) {
			if (bit_istrue(words_activated, NEED_PARAMETERS))
				// Hay mas de una instruccion que usa parametros.
				return STATUS_GCODE_AXIS_COMMAND_CONFLICT;
			else
				// Hay colisiones intra-grupos.
				return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		words_activated |= word;
		word = 0;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	// CORRECCIONES DE FOMRATO Y ERRORES //////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------------
	// <Group 0> Non-modal [G4,G10,G28,G28.1,G30,G30.1,G53,G92,G92.1]
	//-------------------------------------------------------------------------------------
	if (bit_istrue(words_activated, MODAL_GROUP_G0)) {
		size_t idx = size_t(value.p);
		switch (state.non_modal) {
		case NON_MODAL_DWELL:				// [G4 ERRORS]: P value missing. P is negative (done.)
			if (bit_isfalse(words_activated, WORD_P))	return STATUS_GCODE_DWELL_PARAMETER_IS_MISSING;
			break;
		case NON_MODAL_SET_COORDINATE_DATA: // [G10 ERRORS]: L missing, L is not 2 or 20 (done.). P word missing. P is negative (done.)
			if (bit_isfalse(words_activated, WORD_L) || bit_isfalse(words_activated, WORD_P))
				return STATUS_GCODE_G10_PARAMETER_IS_MISSING;
			// [G10 L2/L20 Errors]: P must be 0 to nCoordSys(max 6).
			if (value.p - idx)	return STATUS_BAD_P_NUMBER_NOT_INTEGER;
			if (idx < 0 || idx > NUMBER_COORDINATE_OFFSET)	return STATUS_BAD_NUMBER_COORDINATE_OFFSET_OUT_OF_RANGE;

			if (idx == 0) {
				state.coord_select = Last_State.coord_select;
				idx = Last_State.coord_select;
			}
			// Change reference Word Coordinate that is ACTIVE (Def: G54).

			switch (value.l) {
			case 2: // [G10 L2 Px]
				for (size_t n = 0; n < N_AXIS; n++) {
					if (bit_istrue(words_activated, bit(n))) {
						GCodeParser::Coord_System_Offset[n][idx] = value.xyz[n];
						//Guardar en EEPROM
					}
				}
				break;
			case 20:  // [G10 L20 Px]
				for (size_t n = 0; n < N_AXIS; n++) {
					if (bit_istrue(words_activated, bit(n))) {
						GCodeParser::Coord_System_Offset[n][idx] = Last_Position[n] - Global_Offset[n] - value.xyz[n];
						//Guardar en EEPROM
					}
				}
				break;
			}
			break;
		case NON_MODAL_SET_HOME_0: // [G28.1]
			memcpy(&Predefine_Position_G28[0], &Last_Position[0], sizeof(Last_Position[N_AXIS]));
			//Guardar en EEPROM
			break;
		case NON_MODAL_SET_HOME_1: // [G30.1]
			memcpy(&Predefine_Position_G30[0], &Last_Position[0], sizeof(Last_Position[N_AXIS]));
			//Guardar en EEPROM
			break;
		case NON_MODAL_GO_HOME_0: // [G28]
			//....................................
			// Completar accion de la instruccion
			//....................................
			break;
		case NON_MODAL_GO_HOME_1: // [G30]
			//....................................
			// Completar accion de la instruccion
			//....................................
			break;
		case NON_MODAL_ABSOLUTE_COORDINATE: // [G53]
			if (state.motion != MOTION_MODE_SEEK || state.motion != MOTION_MODE_LINEAR)
				return STATUS_GCODE_G53_INVALID_MOTION_MODE;
			break;
		}
	}
	//-------------------------------------------------------------------------------------


	// Cuando FEED_RATE_MODE_INVERSE_TIME esta activo y existe el comando G1 G2 o G3 el
	// Feedrate tiene que estar definido (explicita o implicitamente).
	if (state.feed_rate == FEED_RATE_MODE_INVERSE_TIME &&
		(
			state.motion == MOTION_MODE_LINEAR ||
			state.motion == MOTION_MODE_CW_ARC ||
			state.motion == MOTION_MODE_CCW_ARC
		)
		&& value.f == NAN)
		return STATUS_GCODE_UNDEFINED_FEED_RATE;


	// Pre-convert XYZ coordinate values to millimeters, if applicable.
	if (bit_istrue(words_activated, MODAL_GROUP_G6) && state.units == UNITS_MODE_INCHES) {
		state.units = UNITS_MODE_MM;
		if (bit_istrue(words_activated, WORD_X)) value.xyz[X_AXIS] *= MM_PER_INCH;
		if (bit_istrue(words_activated, WORD_Y)) value.xyz[Y_AXIS] *= MM_PER_INCH;
		if (bit_istrue(words_activated, WORD_Z)) value.xyz[Z_AXIS] *= MM_PER_INCH;
	}

	// [G43.1 Errors]: Motion command in same line.
	if (bit_istrue(words_activated, MODAL_GROUP_G8) && bit_istrue(words_activated, MODAL_GROUP_G1))
		if (state.tool_length == TOOL_LENGTH_OFFSET_ENABLE_DYNAMIC) 
			return  STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR;

	// [G92 Errors]: No axis words.

    // [G28/30 Errors]: Cutter compensation is enabled.
	// [G28.1/30.1 Errors]: Cutter compensation is enabled.

	// [G80 Errors]: Axis word are programmed while G80 is active.
	// [G0 Errors]: Axis letter not configured or without real value (done.)
	// All remaining motion modes (all but G0 and G80), require a valid feed rate value. In units per mm mode,
	// the value must be positive. In inverse time mode, a positive value must be passed with each block.
	
	// [G2/3 Errors All-Modes]: Feed rate undefined.
	// [G2/3 Radius-Mode Errors]: No axis words in selected plane. Target point is same as current.
	// [G2/3 Offset-Mode Errors]: No axis words and/or offsets in selected plane. The radius to the current
	//   point and the radius to the target point differs more than 0.002mm (EMC def. 0.5mm OR 0.005mm and 0.1% radius).
	// [G2/3 Full-Circle-Mode Errors]: NOT SUPPORTED. Axis words exist. No offsets programmed. P must be an integer.
	// NOTE: Both radius and offsets are required for arc tracing and are pre-computed with the error-checking.

	return STATUS_OK;
}
