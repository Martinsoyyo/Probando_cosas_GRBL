#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <limits>
#include <string>

#include "_Configuration.h"
#include "_Queue.h"
#include "_Number.h"

enum STATUS_PARSER {
	STATUS_GCODE_UNSUPPORTED_G_COMMAND,
	STATUS_GCODE_UNSUPPORTED_M_COMMAND,
	STATUS_GCODE_UNSUPPORTED_COMMAND,
	STATUS_GCODE_FEEDRATE_IS_NEGATIVE,
	STATUS_GCODE_ERROR_NUMBER_EXPECTED,

	//STATUS_GCODE_MODAL_GROUP_VIOLATION,
	STATUS_GCODE_DWELL_PARAMETER_IS_MISSING,
	STATUS_BAD_P_DWELL_IS_NEGATIVE,
	STATUS_BAD_P_COORDINATE_OUT_OF_RANGE,
	STATUS_GCODE_SPINDLE_IS_NEGATIVE,
	STATUS_GCODE_LINE_NUMBER_IS_NEGATIVE,
	STATUS_LINE_LENGTH_EXCEEDED,
	STATUS_GCODE_LINE_EMPTY,
	STATUS_GCODE_COMMENT,
	STATUS_EXPECTED_COMMAND_LETTER,
	STATUS_EXPECTED_A_VALID_NUMBER,
	STATUS_GCODE_UNDEFINED_FEED_RATE,
	STATUS_GCODE_INVALID_COMMENT,
	STATUS_GCODE_G10_PARAMETER_IS_MISSING,
	STATUS_INVALID_GCODE_NUMBER_FORMAT,
	STATUS_VALID_WORD_FORMAT,
	STATUS_BAD_TOOL_NUMBER_NEGATIVE,
	STATUS_BAD_TOOL_NUMBER_EXCEEDED,
	STATUS_BAD_NUMBER_COORDINATE_OFFSET_OUT_OF_RANGE,
	STATUS_BAD_L_NUMBER,
	STATUS_GCODE_G53_INVALID_MOTION_MODE,
	STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR,

	STATUS_GCODE_MODAL_M_GROUP_VIOLATION,
	STATUS_GCODE_MODAL_G_GROUP_VIOLATION,
	STATUS_GCODE_MULTIPLE_PARAMETERS_VIOLATION,
	STATUS_GCODE_AXIS_COMMAND_CONFLICT,
	STATUS_GCODE_WORD_COMMAND_CONFLICT,

	STATUS_OK
};

// Modal Group G0: Non-modal actions
enum NON_MODAL {
	NON_MODAL_NO_ACTION,
	NON_MODAL_DWELL,					//  G4
	NON_MODAL_SET_COORDINATE_DATA,		// G10
	NON_MODAL_GO_HOME_0,				// G28
	NON_MODAL_SET_HOME_0,				// G28.1
	NON_MODAL_GO_HOME_1,				// G30
	NON_MODAL_SET_HOME_1,				// G30.1
	NON_MODAL_ABSOLUTE_COORDINATE,		// G53
	NON_MODAL_SET_COORDINATE_OFFSET,	// G92
	NON_MODAL_RESET_COORDINATE_OFFSET	// G92.1
};

// Modal Group G1: Motion modes
enum MOTION_MODES {
	MOTION_MODE_SEEK,					//G0
	MOTION_MODE_LINEAR,					//G1
	MOTION_MODE_CW_ARC,					//G2
	MOTION_MODE_CCW_ARC,				//G3
	MOTION_MODE_PROBE_TOWARD,
	MOTION_MODE_PROBE_TOWARD_NO_ERROR,
	MOTION_MODE_PROBE_AWAY,
	MOTION_MODE_PROBE_AWAY_NO_ERROR,
	MOTION_MODE_NONE					//G80
};

// Modal Group G2: Plane select
enum PLANE_SELECT {
	PLANE_SELECT_XY, // G17 (Default: Must be zero)
	PLANE_SELECT_ZX, // G18 (Do not alter value)
	PLANE_SELECT_YZ  // G19 (Do not alter value)
};

// Modal Group G3: Distance mode
enum DISTANCE_MODE {
	DISTANCE_MODE_ABSOLUTE,   // G90
	DISTANCE_MODE_INCREMENTAL,// G91
};

// Modal Group G5: Feed rate mode
enum FEED_RATE_MODE {
	FEED_RATE_MODE_UNITS_PER_MIN, // G94
	FEED_RATE_MODE_INVERSE_TIME   // G93
};

// Modal Group G6: Units mode
enum UNITS_MODE {
	UNITS_MODE_MM,     // G21
	UNITS_MODE_INCHES  // G20
};

// Modal Group G7: Cutter radius compensation mode
enum RADIUS_COMPENSATION {
	CUTTER_COMP_DISABLE // G40
};

// Modal Group G8: Tool length offset
enum TOOL_LENGTH_OFFSET {
	TOOL_LENGTH_OFFSET_CANCEL,        // G49
	TOOL_LENGTH_OFFSET_ENABLE_DYNAMIC // G43.1
};

// Modal Group G12: Coord. system Selection
enum COORD_SYSTEM_SELECT {
	G54,  // G54
	G55,  // G55
	G56,  // G56
	G57,  // G57
	G58,  // G58
	G59,  // G59
};

// Modal Group M4: Program flow
enum PROGRAM_FLOW {
	PROGRAM_FLOW_RUNNING,
	PROGRAM_FLOW_PAUSED, 	    // M0
	PROGRAM_FLOW_OPTIONAL_STOP, // M1 NOTE: Not supported, but valid and ignored.
	PROGRAM_FLOW_COMPLETED_M2,  // M2 
	PROGRAM_FLOW_COMPLETED_M30  // M30
};

// Modal Group M7: Spindle control
enum SPINDLE_CONTROL {
	SPINDLE_DISABLE,    // M5
	SPINDLE_ENABLE_CW,  // M3
	SPINDLE_ENABLE_CCW  // M4
};

// Modal Group M8: Coolant control
enum COOLANT_CONTROL {
	COOLANT_DISABLE,      // M9 
	COOLANT_MIST_ENABLE,  // M7
	COOLANT_FLOOD_ENABLE  // M8
};

// Modal Group M9:
enum PARKING {
	OVERRIDE_PARKING_MOTION,
	OVERRIDE_DISABLED
};

// Define parameter word mapping.
#define WORD_X		    bit(0)
#define WORD_Y		    bit(1)
#define WORD_Z		    bit(2)
#define WORD_I		    bit(3)
#define WORD_J		    bit(4)
#define WORD_K		    bit(5)
#define MODAL_GROUP_G0  bit(6)   // [G4,G10,G28,G28.1,G30,G30.1,G53,G92,G92.1] Non-modal
#define MODAL_GROUP_G1  bit(7)   // [G0,G1,G2,G3,G38.2,G38.3,G38.4,G38.5,G80] Motion
#define MODAL_GROUP_G2  bit(8)   // [G17,G18,G19] Plane selection
#define MODAL_GROUP_G3  bit(9)   // [G90,G91] Distance mode
#define MODAL_GROUP_G5  bit(10)  // [G93,G94] Feed rate mode
#define MODAL_GROUP_G6  bit(11)  // [G20,G21] Units
#define MODAL_GROUP_G7  bit(12)  // [G40] Cutter radius compensation mode. G41/42 NOT SUPPORTED.
#define MODAL_GROUP_G8  bit(13)  // [G43.1,G49] Tool length offset
#define MODAL_GROUP_G12 bit(14)  // [G54,G55,G56,G57,G58,G59]
#define MODAL_GROUP_M4  bit(15)  // [M0,M1,M2,M30] Stopping
#define MODAL_GROUP_M7  bit(16)  // [M3,M4,M5] Spindle turning
#define MODAL_GROUP_M8  bit(17)  // [M7,M8,M9] Coolant control
#define MODAL_GROUP_M9  bit(18)  // [M56] Override control
#define WORD_F		    bit(19)
#define WORD_L		    bit(20)
#define WORD_N		    bit(21)
#define WORD_P		    bit(22)
#define WORD_R		    bit(23)
#define WORD_S		    bit(24)
#define WORD_T		    bit(25)
#define NEED_PARAMETERS bit(27)

#define MAX_LINE_NUMBER			10000000
#define MAX_TOOL_NUMBER				 127 // Limited by max unsigned 7-bit value
#define NUMBER_COORDINATE_OFFSET	   6 // [G54,G55,G56,G57,G58,G59]

typedef struct {
	uint32_t non_modal : 4;				// [G4,G10,G28,G28.1,G30,G30.1,G53,G92,G92.1] Non-modal
	uint32_t motion : 4;				// [G0,G1,G2,G3,G38.2,G38.3,G38.4,G38.5,G80] Motion
	uint32_t plane_select : 2;			// [G17,G18,G19] Plane selection
	uint32_t distance : 1;				// [G90,G91] Distance mode
	uint32_t feed_rate : 1;				// [G93,G94] Feed rate mode
	uint32_t units : 1;					// [G20,G21] Units
	uint32_t radius_compensation : 1;	// [G40] Cutter radius compensation mode. G41/42 NOT SUPPORTED.
	uint32_t tool_length : 1;			// [G43.1,G49] Tool length offset
	uint32_t coord_select : 3;			// [G54,G55,G56,G57,G58,G59] Coordinate system selection
	uint32_t program_flow : 3;			// [M0,M1,M2,M30]
	uint32_t coolant : 2;				// [M7,M8,M9]
	uint32_t spindle : 2;				// [M3,M4,M5]
	uint32_t override : 1;				// [M56]
} GcodeStatus;

typedef struct {
	float ijk[3];    // I,J,K Axis arc offsets
	float xyz[3];    // X,Y,Z Translational axes
	float f;         // Feed
	float p;         // [G10] or [Dwell] parameters
	float r;         // Arc radius
	float s;         // Spindle speed
	int32_t n;       // Line number
	int8_t t;        // Tool selection
	int8_t l;        // G10 or canned cycles parameters
} GcodeValues;

class GCodeParser
{
private:
	static GcodeStatus Last_State;
	static float Last_Feedrate;
	static float Last_Position[N_AXIS];// Where the interpreter considers the tool to be at this point in the code
	static float Global_Offset[N_AXIS];// [G92]
	static float Predefine_Position_G28[N_AXIS];// [G28.1]
	static float Predefine_Position_G30[N_AXIS];// [G30.1]
	//......................................................................................................................
	// Referencia a una direccion de la EEPROM, NO es volatil!
	// Todavia no se como se define.
	static float Coord_System_Offset[N_AXIS][NUMBER_COORDINATE_OFFSET - 1];// Stores offset from absolute machine [G54..G59]
	//......................................................................................................................
	GcodeStatus state;
	GcodeValues value;
	size_t real_time_status;

public:
	bool _remove_space_and_comments(std::string&);
	void _proccess_gcode(void*);
	uint32_t _get_Status()const { return real_time_status; };

	friend ostream& operator<< (ostream&, const GCodeParser&);
};

#endif


