#include "wsjtx_lib.h"
#include "wsjtx_decode.h"
#include "wsjtx_encode.h"
#include "constants.h"
#include <memory>
#include <fftw3.h>

extern DataQueue<WsjtxMessage> WsjtxMessageQueue;
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

wsjtx_lib::wsjtx_lib()
{
	fftwf_init_threads();
}

bool wsjtx_lib::pullMessage(WsjtxMessage &msg)
{
	
	return WsjtxMessageQueue.pull(msg);
}

void wsjtx_lib::decode(wsjtxMode mode, WsjTxVector &audiosamples, int freq, int thread)
{
	std::unique_ptr<wstjx_decode> ptr;

	ptr = std::make_unique<wstjx_decode>();
	ptr->decode(mode, audiosamples, freq, thread);
}

void wsjtx_lib::decode(wsjtxMode mode, IntWsjTxVector &audiosamples, int freq, int thread)
{
	std::unique_ptr<wstjx_decode> ptr;

	ptr = std::make_unique<wstjx_decode>();
	ptr->decode(mode, audiosamples, freq, thread);
}

int wspr_decode(std::vector<std::complex<float>> &iqdat,
				int samples,
				decoder_options options,
				std::vector<struct decoder_results> &decodes,
				int threads);

std::vector<struct decoder_results> wsjtx_lib::wspr_decode(WsjtxIQSampleVector &iqsignal, decoder_options options)
{
	std::vector<decoder_results> results;

	::wspr_decode(iqsignal, iqsignal.size(), options, results, 4);
	return results;
}

std::vector<float> wsjtx_lib::encode(wsjtxMode mode, int frequency, std::string message, std::string &messagesend)
{
	std::vector<float> ret;
	switch (mode)
	{
		case FT8:
		{
			std::unique_ptr<wsjtx_encode> ptr;

			ptr = std::make_unique<wsjtx_encode>();
			return ptr->encode_ft8(mode, frequency, message, messagesend);
		}
		case FT4:
		{
			std::unique_ptr<wsjtx_encode> ptr;

			ptr = std::make_unique<wsjtx_encode>();
			return ptr->encode_ft4(mode, frequency, message, messagesend);
		}
	}
	// unsuported modes return empty vector
	return ret;
}