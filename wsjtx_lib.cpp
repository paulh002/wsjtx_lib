#include "wsjtx_lib.h"
#include "wsjtx_decode.h"
#include "wsjtx_encode.h"
#include "constants.h"
#include <memory>

/*
	To test the library, include "wsjtx_lib.h" from an application project
	and call wsjtx_libTest().
	
	Do not forget to add the library to Project Dependencies in Visual Studio.
*/

static int s_Test = 0;

int wsjtx_libTest()
{
	return ++s_Test;
}

void wsjtx_lib::decode(wsjtxMode mode, SampleVector &audiosamples, int freq)
{
	std::unique_ptr<wstjx_decode> ptr;

	ptr = std::make_unique<wstjx_decode>();
	ptr->decode(mode, audiosamples, freq);
}

void wsjtx_lib::decode(wsjtxMode mode, IntSampleVector &audiosamples, int freq)
{
	std::unique_ptr<wstjx_decode> ptr;

	ptr = std::make_unique<wstjx_decode>();
	ptr->decode(mode, audiosamples, freq);
}

std::vector<float> wsjtx_lib::encode(wsjtxMode mode,int frequency, std::string message)
{
	std::vector<float> ret;
	switch (mode)
	{
		case FT8:
		case FT4:
		{
			std::unique_ptr<wsjtx_encode> ptr;

			ptr = std::make_unique<wsjtx_encode>();
			return ptr->encode_ft84(mode, frequency, message);
		}
	}
	return ret;
}