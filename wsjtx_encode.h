#pragma once
#include "wsjtx_lib.h"
#include "constants.h"
#include <string>
#include <vector>

#define FT8_SYMBOL_BT 2.0f ///< symbol smoothing filter bandwidth factor (BT)
#define FT4_SYMBOL_BT 1.0f ///< symbol smoothing filter bandwidth factor (BT)
#define MAX_NUM_SYMBOLS 250
#define GFSK_CONST_K 5.336446f ///< == pi * sqrt(2 / log(2))

class wsjtx_encode
{
	public:
	std::vector<float> encode_ft84(wsjtxMode mode, int frequency, std::string message);
	
  private:
	int itone[MAX_NUM_SYMBOLS] = {0};
};