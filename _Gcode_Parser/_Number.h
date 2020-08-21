#ifndef NUMBER_H
#define NUMBER_H

#include <string> // memset

#include "_Configuration.h"

using namespace std;

class _Number 
{
private:
	struct {
		uint32_t _digit_in_integer_part : 8;
		uint32_t _digit_in_decimal_part : 8;
		uint32_t _have_positive_sign : 1;
		uint32_t _have_negative_sign : 1;
		uint32_t _have_fractional : 1;
	};

	float  _float_num;
	size_t _decimal_part;
	size_t _integer_part;

public:
	_Number() { memset(this, 0, sizeof(_Number)); }
	bool Read(std::string& LINE);

	float GetFloat()		   const { return _float_num; }
	size_t GetFractionalPart() const { return _decimal_part; }
	size_t GetIntegerPart()    const { return _integer_part; }
	bool IsNegative()          const { return _have_negative_sign; }
	bool IsFractional()		   const { return _have_fractional; }
	bool IsInteger()		   const { return !_have_fractional; }
	bool HaveSign()			   const { return (_have_positive_sign || _have_negative_sign); }
};

#endif
