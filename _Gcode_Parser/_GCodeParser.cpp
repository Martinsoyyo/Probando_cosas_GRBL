#include "_GCodeParser.h"

GcodeStatus GCodeParser::Last_State = { 0 };
float GCodeParser::Last_Feedrate = NAN;
float GCodeParser::Last_Position[N_AXIS] = { 0 };
float GCodeParser::Global_Offset[N_AXIS] = { 0 };
float GCodeParser::Predefine_Position_G28[N_AXIS] = { 0 };
float GCodeParser::Predefine_Position_G30[N_AXIS] = { 0 };
float GCodeParser::Coord_System_Offset[N_AXIS][NUMBER_COORDINATE_OFFSET - 1] = { 0 };

bool GCodeParser::_remove_space_and_comments(std::string& STR) {
	size_t idx = 0;
	bool inside_comment = false;

	for (auto CH : STR) {
		if (CH == '(')		inside_comment = true;
		else if (CH == ')') inside_comment = false;
		else if (CH == ';' || CH == '/') break;
		else if (CH == ' ') continue;
		else if (!inside_comment) STR[idx++] = CH;
	}
	STR.resize(idx);
	return (inside_comment);
}

void GCodeParser::_proccess_gcode(void* MSG) {
	memset(&state, 0, sizeof(GcodeStatus));
	memset(&value, 0, sizeof(GcodeValues));
	state = Last_State;
	value.f = Last_Feedrate;

	std::string LINE = std::move(*static_cast<std::string*>(MSG));
	if (LINE.empty() || _remove_space_and_comments(LINE)) 	return;

	std::cout << "< " << LINE << " >" << std::endl;

	// ---------------------------------------------------------------------------------------------
	// ETAPA I
	// Regla 1: Verifico que los comandos parseados existan realmente.
	// Regla 2: Solo puede haber UN COMANDO que use los parametros.
	// Regla 3: NO PUEDEN existir dos comandos pertenecientes al mismo grupo.
	// ---------------------------------------------------------------------------------------------
	size_t words_activated = 0;
	_Number NUM;
	while (LINE.length()) {
		size_t word = 0;
		char CH = std::toupper(LINE[0]);
		std::cout << "< " << CH << " >" << std::endl;

		LINE = std::move(LINE.substr(1, LINE.length()));

		if (NUM.Read(LINE)) {
			std::cout << "< " << NUM.GetIntegerPart() << "." << NUM.GetFractionalPart() << " >" << std::endl;
			if (NUM.IsFractional()) { //<Xxx.n>
				switch (CH) {
				case 'G':
					if (NUM.GetFractionalPart() == 1) {//<Gxx.1>
						switch (NUM.GetIntegerPart()) {
						case 28:word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_SET_HOME_0; break;//G28.1
						case 30:word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_SET_HOME_1; break;//G30.1
						case 92:word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_RESET_COORDINATE_OFFSET; break;//G92.1
						case 43:word = MODAL_GROUP_G8; state.tool_length = TOOL_LENGTH_OFFSET_ENABLE_DYNAMIC; break; //G43.1
						default:
							real_time_status = STATUS_GCODE_UNSUPPORTED_G_COMMAND;
							return;
						};
					}
					else { //<Gxx.n>
						real_time_status = STATUS_GCODE_UNSUPPORTED_G_COMMAND;
						return;
					}
					break;

				case 'R': word = WORD_R; value.r = NUM.GetFloat(); break;
				case 'I': word = WORD_I; value.ijk[X_AXIS] = NUM.GetFloat(); break;
				case 'J': word = WORD_J; value.ijk[Y_AXIS] = NUM.GetFloat(); break;
				case 'K': word = WORD_K; value.ijk[Z_AXIS] = NUM.GetFloat(); break;
				case 'X': word = WORD_X; value.xyz[X_AXIS] = NUM.GetFloat(); break;
				case 'Y': word = WORD_Y; value.xyz[Y_AXIS] = NUM.GetFloat(); break;
				case 'Z': word = WORD_Z; value.xyz[Z_AXIS] = NUM.GetFloat(); break;
				case 'F': word = WORD_F; value.f = NUM.GetFloat(); break;
				case 'P': word = WORD_P; value.p = NUM.GetFloat(); break;
				case 'S': word = WORD_S; value.s = NUM.GetFloat(); break;
				default:
					real_time_status = STATUS_GCODE_UNSUPPORTED_COMMAND;
					return;
				}
			} 
			else { //<Xxx>
				switch (CH) {
				case 'R': word = WORD_R; value.r = NUM.GetFloat(); break;
				case 'I': word = WORD_I; value.ijk[X_AXIS] = NUM.GetFloat(); break;
				case 'J': word = WORD_J; value.ijk[Y_AXIS] = NUM.GetFloat(); break;
				case 'K': word = WORD_K; value.ijk[Z_AXIS] = NUM.GetFloat(); break;
				case 'X': word = WORD_X; value.xyz[X_AXIS] = NUM.GetFloat(); break;
				case 'Y': word = WORD_Y; value.xyz[Y_AXIS] = NUM.GetFloat(); break;
				case 'Z': word = WORD_Z; value.xyz[Z_AXIS] = NUM.GetFloat(); break;
				case 'F': word = WORD_F; value.f = NUM.GetFloat(); break;
				case 'P': word = WORD_P; value.p = NUM.GetFloat(); break;
				case 'S': word = WORD_S; value.s = NUM.GetFloat(); break;
				case 'L': word = WORD_L; value.l = (int8_t)NUM.GetFloat(); break;
				case 'T': word = WORD_T; value.s = (int8_t)NUM.GetFloat(); break;
				case 'N': word = WORD_N; value.n = (int8_t)NUM.GetFloat(); break;
				case 'G':
					switch (NUM.GetIntegerPart()) { //<Gxx>
					case 4:  word = (MODAL_GROUP_G0 | NEED_PARAMETERS); state.non_modal = NON_MODAL_DWELL; break;//G4
					case 10: word = (MODAL_GROUP_G0 | NEED_PARAMETERS); state.non_modal = NON_MODAL_SET_COORDINATE_DATA; break;//G10
					case 28: word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_GO_HOME_0; break;//G28
					case 30: word = MODAL_GROUP_G0; state.non_modal = NON_MODAL_GO_HOME_1; break;//G30
					case 53: word = (MODAL_GROUP_G0 | NEED_PARAMETERS);	state.non_modal = NON_MODAL_ABSOLUTE_COORDINATE; break;//G53
					case 92: word = (MODAL_GROUP_G0 | NEED_PARAMETERS); state.non_modal = NON_MODAL_SET_COORDINATE_OFFSET; break;//G92
					case 0:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_SEEK; break;//G0
					case 1:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_LINEAR; break;//G1
					case 2:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_CW_ARC; break;//G2
					case 3:  word = (MODAL_GROUP_G1 | NEED_PARAMETERS); state.motion = MOTION_MODE_CCW_ARC; break;//G3
					case 80: word = MODAL_GROUP_G1; state.motion = MOTION_MODE_NONE; break;//G80
					case 17: word = MODAL_GROUP_G2;	state.plane_select = PLANE_SELECT_XY; break;//G17
					case 18: word = MODAL_GROUP_G2;	state.plane_select = PLANE_SELECT_ZX; break;//G18
					case 19: word = MODAL_GROUP_G2;	state.plane_select = PLANE_SELECT_YZ; break;//G19
					case 90: word = MODAL_GROUP_G3;	state.distance = DISTANCE_MODE_ABSOLUTE; break;//G20
					case 91: word = MODAL_GROUP_G3;	state.distance = DISTANCE_MODE_INCREMENTAL;	break;//G91
					case 93: word = MODAL_GROUP_G5;	state.feed_rate = FEED_RATE_MODE_INVERSE_TIME; break;//G93
					case 94: word = MODAL_GROUP_G5;	state.feed_rate = FEED_RATE_MODE_UNITS_PER_MIN;	break;//G94
					case 20: word = MODAL_GROUP_G6;	state.units = UNITS_MODE_INCHES; break;//G20
					case 21: word = MODAL_GROUP_G6;	state.units = UNITS_MODE_MM; break;//G21
					case 40: word = MODAL_GROUP_G7; state.radius_compensation = CUTTER_COMP_DISABLE; break;//G40
					case 49: word = MODAL_GROUP_G8; state.tool_length = TOOL_LENGTH_OFFSET_CANCEL; break;//G49
					case 54: word = MODAL_GROUP_G12; state.coord_select = G54; break;//G54
					case 55: word = MODAL_GROUP_G12; state.coord_select = G55; break;//G55
					case 56: word = MODAL_GROUP_G12; state.coord_select = G56; break;//G56
					case 57: word = MODAL_GROUP_G12; state.coord_select = G57; break;//G57
					case 58: word = MODAL_GROUP_G12; state.coord_select = G58; break;//G58
					case 59: word = MODAL_GROUP_G12; state.coord_select = G59; break;//G59
					default:
						real_time_status = STATUS_GCODE_UNSUPPORTED_G_COMMAND;
						return;
					}
					break;
				case 'M':
					switch (NUM.GetIntegerPart()) { //<Mxx>
					case 0:  word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_PAUSED; break;//M0
					case 1:  word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_OPTIONAL_STOP; break;//M1
					case 2:  word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_COMPLETED_M2; break;//M2
					case 30: word = MODAL_GROUP_M4; state.program_flow = PROGRAM_FLOW_COMPLETED_M30; break;//M30
					case 3:  word = MODAL_GROUP_M7; state.spindle = SPINDLE_ENABLE_CW; break;//M3
					case 4:  word = MODAL_GROUP_M7; state.spindle = SPINDLE_ENABLE_CCW; break;//M4
					case 5:  word = MODAL_GROUP_M7; state.spindle = SPINDLE_DISABLE; break;//M5
					case 7:  word = MODAL_GROUP_M8; state.coolant = COOLANT_MIST_ENABLE; break;//M7
					case 8:  word = MODAL_GROUP_M8; state.coolant = COOLANT_FLOOD_ENABLE; break;//M8
					case 9:  word = MODAL_GROUP_M8; state.coolant = COOLANT_DISABLE; break;//M9
					case 56: word = MODAL_GROUP_M9; state.override = OVERRIDE_PARKING_MOTION; break;//M56
					default:
						real_time_status = STATUS_GCODE_UNSUPPORTED_M_COMMAND;
						return;
					}
					break;
				default:
					real_time_status = STATUS_GCODE_UNSUPPORTED_COMMAND;
					return;
				}
			}
		}
		else {
			real_time_status = STATUS_GCODE_ERROR_NUMBER_EXPECTED;
			return;
		}

		// Regla 2: Solo puede haber UN COMANDO que use los parametros.
		// Regla 3: NO PUEDEN existir dos comandos pertenecientes al mismo grupo.
		if (bit_istrue(words_activated, word)) {
			switch (word) {
			case MODAL_GROUP_M4:case MODAL_GROUP_M7:case MODAL_GROUP_M8:case MODAL_GROUP_M9:
				real_time_status = STATUS_GCODE_MODAL_M_GROUP_VIOLATION;
				return;
			case NEED_PARAMETERS:
				real_time_status = STATUS_GCODE_MULTIPLE_PARAMETERS_VIOLATION;
				return;
			case WORD_X : case WORD_Y:case WORD_Z:case WORD_I: case WORD_J:case WORD_K:
				real_time_status = STATUS_GCODE_AXIS_COMMAND_CONFLICT;
				return;
			case WORD_R: case WORD_F:case WORD_P:case WORD_S:case WORD_L:case WORD_T:case WORD_N:
				real_time_status = STATUS_GCODE_WORD_COMMAND_CONFLICT;
				return;
			default:
				real_time_status = STATUS_GCODE_MODAL_G_GROUP_VIOLATION;
				return;
			}
		};

		words_activated |= word;
	}


	// ---------------------------------------------------------------------------------------------
	// ETAPA II
	// Regla 1: Verifico que los comandos que necesitan parametros los tengan.
	// Regla 2: Verifico que los comandos y parametros tengas valores validos.
	// ---------------------------------------------------------------------------------------------
	if (bit_istrue(words_activated, WORD_S) && value.s < 0) {
		real_time_status = STATUS_GCODE_SPINDLE_IS_NEGATIVE;
		return;
	}
	if (bit_istrue(words_activated, WORD_F) && value.f < 0) {
		real_time_status = STATUS_GCODE_FEEDRATE_IS_NEGATIVE;
		return;
	}
	if (bit_istrue(words_activated, WORD_N)) {
		if (value.n < 0) {
			real_time_status = STATUS_GCODE_LINE_NUMBER_IS_NEGATIVE; 
			return;
		}
		if (value.n > MAX_LINE_NUMBER) {
			real_time_status = STATUS_LINE_LENGTH_EXCEEDED;
			return;
		}
	};
	if (bit_istrue(words_activated, WORD_T)) {
		if (value.t < 0) {
			real_time_status = STATUS_BAD_TOOL_NUMBER_NEGATIVE;
			return;
		}
		if (value.t > MAX_TOOL_NUMBER) {
			real_time_status = STATUS_BAD_TOOL_NUMBER_EXCEEDED;
			return;
		}
	};

	//---------------------------------------------------------------
	// <Group 0> Non-modal [G4,G10,G28,G28.1,G30,G30.1,G53,G92,G92.1]
	//---------------------------------------------------------------
	if (bit_istrue(words_activated, MODAL_GROUP_G0)) {
		size_t idx = size_t(value.p);
		switch (state.non_modal) {
		case NON_MODAL_DWELL:				
			// [G4 Errors]: P value missing. P is negative.
			if (bit_isfalse(words_activated, WORD_P)) {
				real_time_status = STATUS_GCODE_DWELL_PARAMETER_IS_MISSING;
				return;
			}
			if (value.p < 0) {
				real_time_status = STATUS_BAD_P_DWELL_IS_NEGATIVE;
				return;
			}
			break;
		case NON_MODAL_SET_COORDINATE_DATA: 
			// [G10 Errors]: L missing or P word missing.
			if (bit_isfalse(words_activated, WORD_L) || bit_isfalse(words_activated, WORD_P)) {
				real_time_status = STATUS_GCODE_G10_PARAMETER_IS_MISSING;
				return;
			}
			// [G10 Errors]: P must be integer (in a range) and non zero. 
			
			if (idx < 0 || idx > NUMBER_COORDINATE_OFFSET) {
				real_time_status = STATUS_BAD_P_COORDINATE_OUT_OF_RANGE;
				return;
			}
			// Change reference Word Coordinate that is ACTIVE (Def: G54).
			if (idx == 0) {
				state.coord_select = Last_State.coord_select;
				idx = Last_State.coord_select;
			}

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
			default:
				real_time_status = STATUS_BAD_L_NUMBER;
				return;
			}
			break;
		case NON_MODAL_SET_HOME_0: // [G28.1]
			memcpy(&Predefine_Position_G28[0], &Last_Position[0], N_AXIS * sizeof(Last_Position[0]));
			//Guardar en EEPROM
			break;
		case NON_MODAL_SET_HOME_1: // [G30.1]
			memcpy(&Predefine_Position_G30[0], &Last_Position[0], N_AXIS * sizeof(Last_Position[0]));
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
			if (state.motion != MOTION_MODE_SEEK || state.motion != MOTION_MODE_LINEAR) {
				real_time_status = STATUS_GCODE_G53_INVALID_MOTION_MODE;
				return;
			}
			break;
		}
	}

	// Cuando FEED_RATE_MODE_INVERSE_TIME esta activo y existe el comando G1 G2 o G3 el
	// Feedrate tiene que estar definido (explicita o implicitamente).
	if (state.feed_rate == FEED_RATE_MODE_INVERSE_TIME &&
		(
			state.motion == MOTION_MODE_LINEAR ||
			state.motion == MOTION_MODE_CW_ARC ||
			state.motion == MOTION_MODE_CCW_ARC
			)
		&& value.f == NAN) {
		real_time_status = STATUS_GCODE_UNDEFINED_FEED_RATE;
		return;
	}

	// Pre-convert XYZ coordinate values to millimeters, if applicable.
	if (bit_istrue(words_activated, MODAL_GROUP_G6) && state.units == UNITS_MODE_INCHES) {
		state.units = UNITS_MODE_MM;
		if (bit_istrue(words_activated, WORD_X)) value.xyz[X_AXIS] *= (float)MM_PER_INCH;
		if (bit_istrue(words_activated, WORD_Y)) value.xyz[Y_AXIS] *= (float)MM_PER_INCH;
		if (bit_istrue(words_activated, WORD_Z)) value.xyz[Z_AXIS] *= (float)MM_PER_INCH;
	}

	// [G43.1 Errors]: Motion command in same line.
	if (bit_istrue(words_activated, MODAL_GROUP_G8) && bit_istrue(words_activated, MODAL_GROUP_G1))
		if (state.tool_length == TOOL_LENGTH_OFFSET_ENABLE_DYNAMIC) {
			real_time_status = STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR;
			return;
		}

	// Check for G10/28/30/92 being called with G0/1/2/3/38 on same block.
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
	// point and the radius to the target point differs more than 0.002mm (EMC def. 0.5mm OR 0.005mm and 0.1% radius).
	// [G2/3 Full-Circle-Mode Errors]: NOT SUPPORTED. Axis words exist. No offsets programmed. P must be an integer.
	// NOTE: Both radius and offsets are required for arc tracing and are pre-computed with the error-checking.

	real_time_status = STATUS_OK;
}


ostream& operator<<(ostream& STREAM, const GCodeParser& GCODE) {
	switch (GCODE._get_Status()) {
	case STATUS_GCODE_DWELL_PARAMETER_IS_MISSING: STREAM << "STATUS_GCODE_DWELL_PARAMETER_IS_MISSING" << endl; break;
	case STATUS_BAD_P_DWELL_IS_NEGATIVE: STREAM << "STATUS_BAD_P_DWELL_IS_NEGATIVE" << endl; break;
	case STATUS_BAD_P_COORDINATE_OUT_OF_RANGE: STREAM << "STATUS_BAD_P_COORDINATE_OUT_OF_RANGE" << endl; break;
	case STATUS_GCODE_UNSUPPORTED_M_COMMAND: STREAM << "STATUS_GCODE_UNSUPPORTED_M_COMMAND" << endl; break;
	case STATUS_GCODE_UNSUPPORTED_G_COMMAND: STREAM << "STATUS_GCODE_UNSUPPORTED_G_COMMAND" << endl; break;
	case STATUS_GCODE_FEEDRATE_IS_NEGATIVE: STREAM << "STATUS_GCODE_FEEDRATE_IS_NEGATIVE" << endl; break;
	case STATUS_GCODE_SPINDLE_IS_NEGATIVE: STREAM << "STATUS_GCODE_SPINDLE_IS_NEGATIVE" << endl; break;
	case STATUS_GCODE_LINE_NUMBER_IS_NEGATIVE: STREAM << "STATUS_GCODE_LINE_NUMBER_IS_NEGATIVE" << endl; break;
	case STATUS_LINE_LENGTH_EXCEEDED: STREAM << "STATUS_LINE_LENGTH_EXCEEDED" << endl; break;
	case STATUS_GCODE_LINE_EMPTY: STREAM << "STATUS_GCODE_LINE_EMPTY" << endl; break;
	case STATUS_GCODE_UNSUPPORTED_COMMAND: STREAM << "STATUS_GCODE_UNSUPPORTED_COMMAND" << endl; break;
	case STATUS_GCODE_COMMENT: STREAM << "STATUS_GCODE_COMMENT" << endl; break;
	case STATUS_EXPECTED_COMMAND_LETTER: STREAM << "STATUS_EXPECTED_COMMAND_LETTER" << endl; break;
	case STATUS_EXPECTED_A_VALID_NUMBER: STREAM << "STATUS_EXPECTED_A_VALID_NUMBER" << endl; break;
	//case STATUS_GCODE_MODAL_GROUP_VIOLATION: STREAM << "STATUS_GCODE_MODAL_GROUP_VIOLATION" << endl; break;
	case STATUS_GCODE_UNDEFINED_FEED_RATE: STREAM << "STATUS_GCODE_UNDEFINED_FEED_RATE" << endl; break;
	case STATUS_GCODE_INVALID_COMMENT: STREAM << "STATUS_GCODE_INVALID_COMMENT" << endl; break;
	case STATUS_GCODE_G10_PARAMETER_IS_MISSING: STREAM << "STATUS_GCODE_G10_PARAMETER_IS_MISSING" << endl; break;
	case STATUS_INVALID_GCODE_NUMBER_FORMAT: STREAM << "STATUS_INVALID_GCODE_NUMBER_FORMAT" << endl; break;
	case STATUS_VALID_WORD_FORMAT: STREAM << "STATUS_VALID_WORD_FORMAT" << endl; break;
	case STATUS_BAD_TOOL_NUMBER_NEGATIVE: STREAM << "STATUS_BAD_TOOL_NUMBER_NEGATIVE" << endl; break;
	case STATUS_BAD_TOOL_NUMBER_EXCEEDED: STREAM << "STATUS_BAD_TOOL_NUMBER_EXCEEDED" << endl; break;
	case STATUS_BAD_NUMBER_COORDINATE_OFFSET_OUT_OF_RANGE: STREAM << "STATUS_BAD_NUMBER_COORDINATE_OFFSET_OUT_OF_RANGE" << endl; break;
	case STATUS_BAD_L_NUMBER: STREAM << "STATUS_BAD_L_NUMBER" << endl; break;
	case STATUS_GCODE_G53_INVALID_MOTION_MODE: STREAM << "STATUS_GCODE_G53_INVALID_MOTION_MODE" << endl; break;
	case STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR: STREAM << "STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR" << endl; break;
	case STATUS_GCODE_ERROR_NUMBER_EXPECTED: STREAM << "STATUS_GCODE_ERROR_NUMBER_EXPECTED" << endl; break;

	case STATUS_GCODE_MODAL_M_GROUP_VIOLATION: STREAM << "STATUS_GCODE_MODAL_M_GROUP_VIOLATION" << endl; break;
	case STATUS_GCODE_MODAL_G_GROUP_VIOLATION: STREAM << "STATUS_GCODE_MODAL_G_GROUP_VIOLATION" << endl; break;
	case STATUS_GCODE_MULTIPLE_PARAMETERS_VIOLATION: STREAM << "STATUS_GCODE_MULTIPLE_PARAMETERS_VIOLATION" << endl; break;
	case STATUS_GCODE_AXIS_COMMAND_CONFLICT: STREAM << "STATUS_GCODE_AXIS_COMMAND_CONFLICT" << endl; break;
	case STATUS_GCODE_WORD_COMMAND_CONFLICT: STREAM << "STATUS_GCODE_WORD_COMMAND_CONFLICT" << endl; break;

	case STATUS_OK:
		// <GROUP 0> [G4,G10,G28,G28.1,G30,G30.1,G53,G92,G92.1] Non-modal
		STREAM << "STATUS_OK" << endl;
		switch (GCODE.state.non_modal) {
		case NON_MODAL_DWELL:					//  G4
			STREAM << "<G4> NON_MODAL_DWELL" << " P=" << GCODE.value.p << endl;
			break;
		case NON_MODAL_SET_COORDINATE_DATA:		// G10
			STREAM << "<G10> NON_MODAL_SET_COORDINATE_DATA" << " P=" << GCODE.value.p << endl;
			break;
		case NON_MODAL_GO_HOME_0:				// G28
			STREAM << "<G28> NON_MODAL_GO_HOME_0" <<
				" X=" << GCODE.Predefine_Position_G28[0] <<
				" Y=" << GCODE.Predefine_Position_G28[1] <<
				" Z=" << GCODE.Predefine_Position_G28[2] << endl;
			break;
		case NON_MODAL_SET_HOME_0:				// G28.1
			STREAM << "<G28.1> NON_MODAL_SET_HOME_0" <<
				" X=" << GCODE.Predefine_Position_G28[0] <<
				" Y=" << GCODE.Predefine_Position_G28[1] <<
				" Z=" << GCODE.Predefine_Position_G28[2] << endl;
			break;
		case NON_MODAL_GO_HOME_1:				// G30
			STREAM << "<G30> NON_MODAL_GO_HOME_1" <<
				" X=" << GCODE.Predefine_Position_G30[0] <<
				" Y=" << GCODE.Predefine_Position_G30[1] <<
				" Z=" << GCODE.Predefine_Position_G30[2] << endl;
			break;
		case NON_MODAL_SET_HOME_1:				// G30.1
			STREAM << "<G30.1> NON_MODAL_SET_HOME_1" <<
				" X=" << GCODE.Predefine_Position_G30[0] <<
				" Y=" << GCODE.Predefine_Position_G30[1] <<
				" Z=" << GCODE.Predefine_Position_G30[2] << endl;
			break;
		case NON_MODAL_ABSOLUTE_COORDINATE:		// G53
			STREAM << "<G53> NON_MODAL_ABSOLUTE_COORDINATE" << endl;
			break;
		case NON_MODAL_SET_COORDINATE_OFFSET:   // G92
			STREAM << "<G92> NON_MODAL_SET_COORDINATE_OFFSET" << endl;
			break;
		case NON_MODAL_RESET_COORDINATE_OFFSET: // G92.1
			STREAM << "<G92.1> NON_MODAL_RESET_COORDINATE_OFFSET" << endl;
			break;
		case NON_MODAL_NO_ACTION:
			break;
		}		
		//switch (GCODE.state.motion)
		//switch (GCODE.state.plane_select)
		//switch (GCODE.state.distance)
		//switch (GCODE.state.feed_rate)
		//switch (GCODE.state.units)
		//switch (GCODE.state.radius_compensation)
		//switch (GCODE.state.tool_length)
		//switch (GCODE.state.coord_select)
		//switch (GCODE.state.program_flow)
		//switch (GCODE.state.coolant)
		//switch (GCODE.state.spindle)
		//switch (GCODE.state.override)
		break;
	default:
		STREAM << "UNESPECIFIED ERROR" << endl;
		break;
	};

	return STREAM;
}
