#pragma once
#include <vector>
#include "DataBuffer.h"
#include "commons.h"
#include "fortran_interface.h"
#include "wsjtx_lib.h"


class wstjx_decode
{
  public:
	void decode(wsjtxMode mode, WsjTxVector &audiosamples, int freq, int threads = 1);
	void decode(wsjtxMode mode, IntWsjTxVector &audiosamples, int freq, int threads = 1);

  private:
	params_t params;
	DataBuffer<float> samplebuffer;
	dec_data_t dec_data;
};