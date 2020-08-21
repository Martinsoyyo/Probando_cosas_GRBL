#include <iostream>

#include "_Configuration.h"
#include "_Number.h"
#include "_GCodeParser.h"

std::string STR = "g4 x23.1  p67m0";

int main()
{
 
    GCodeParser GCODE;

    GCODE._proccess_gcode(&STR);

    std::cout << GCODE;
}
