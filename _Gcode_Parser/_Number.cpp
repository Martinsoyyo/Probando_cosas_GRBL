#include "_Number.h"
#include <iostream>

bool _Number::Read(std::string& LINE) {
	memset(this, 0, sizeof(_Number));
	size_t count = 0;

	for (auto CH : LINE) {
		if (isdigit(CH)) {
			if (_have_fractional) {
				_decimal_part = _decimal_part * 10 + (CH - '0');
				_digit_in_decimal_part++;
			}
			else {
				_integer_part = _integer_part * 10 + (CH - '0');
				_digit_in_integer_part++;
			}
		}
		// La puntuacion solo puede aparecer UNA VEZ, si no es asi, esta mal el formato.
		else if (CH == '.' && !_have_fractional) {
			_have_fractional = true;
		}
		// El signo tiene que estar ANTES que cualquier numero, si no es asi, esta mal el formato.
		else if (CH == '-' && _digit_in_integer_part == 0 && _digit_in_decimal_part == 0) {
			_have_negative_sign = true;
		}
		else if (CH == '+' && _digit_in_integer_part == 0 && _digit_in_decimal_part == 0) {
			_have_positive_sign = true;
		}
		// Formato de numero invalido.
		else break;
		count++;
	}

	//Verifico que haya recogido algun numero., si no es asi, esta mal el formato.
	if (_digit_in_integer_part || _digit_in_decimal_part) {
		_float_num = stof(LINE.substr(0, count));
		LINE = std::move(LINE.substr(count, LINE.length()));
		return true;
	}
	return false;
};
