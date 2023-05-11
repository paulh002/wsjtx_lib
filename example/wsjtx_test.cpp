#include <iostream>
#include "wave.h"
#include <wsjtx_lib.h>

using namespace std;

IntWsjTxVector audioSignal;

wsjtx_lib decoder;

int main(int argc, char *argv[])
{

	int num_samples = 15 * 48000;
	int sample_rate = 48000;
	WsjtxMessage msg;

	audioSignal.resize(15 * 48000 * 2);
	
	for (int i =0 ; i < 10; i++)
	{
		num_samples = 15 * 48000;
		load_wav_int((short int *)audioSignal.data(), &num_samples, &sample_rate, "samples/FT8/210703_133430.wav");
		decoder.decode(FT8, audioSignal, 1000, 4);
		load_wav_int((short int *)audioSignal.data(), &num_samples, &sample_rate, "samples/FT4/000000_000002.wav");
		decoder.decode(FT4, audioSignal, 1000, 4);
		while (decoder.pullMessage(msg))
		{
			printf("hh %d min %d s %d, snr %d dt %f, freq %d, message %s, \n",msg.hh,
						  msg.min,
						  msg.sec,
						  msg.snr,
						  msg.dt,
						  msg.freq,
						  msg.msg.c_str());
		}
	} 
	return 0;
}