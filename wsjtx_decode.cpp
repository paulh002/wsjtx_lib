#include "wsjtx_lib.h"
#include "wsjtx_decode.h"
#include <cstring>
#include <string>
#include <fftw3.h>
#include <ctime>
#include <time.h>

DataQueue<WsjtxMessage> WsjtxMessageQueue;

extern "C" {

void wsjtx_decoded_(int *nutc, int *snr, float *dt, int *freq, char *decoded, int len)
	{
		char message[23];

		std::strncpy(message, decoded, 22);
		message[22] = '\0';
		for (int i = 22; i != 0; i--)
		{
			if (message[i] == ' ' || message[i] == '\0')
				message[i] = '\0';
			else
				break;
		}
		if (!strstr(message, "DecodeFinished"))
		{
			//printf("nutc %d, snr %d, dt %f, freq %d message %s \n", *nutc, *snr, *dt, *freq, message);
			WsjtxMessageQueue.push(WsjtxMessage(*nutc / 10000, (*nutc / 100) % 100, *nutc % 100, *snr, *dt, *freq, std::string(message)));
		}
	}

	void wsjtx_decoded_fst4_(int *nutc, float *sync, int *snr, float *dt, float *freq, char *decoded, int len)
	{
		char message[38];

		std::strncpy(message, decoded, 37);
		message[37] = '\0';
		for (int i = 37; i != 0; i--)
		{
			if (message[i] == ' ' || message[i] == '\0')
				message[i] = '\0';
			else
				break;
		}
		if (!strstr(message, "DecodeFinished"))
		{
			//printf("nutc %d, snr %d, dt %f, freq %d message %s \n", *nutc, *snr, *dt, *freq, message);
			WsjtxMessageQueue.push(WsjtxMessage(*nutc / 10000, (*nutc / 100) % 100, *nutc % 100, *snr, *sync, *dt, *freq, std::string(message)));
		}
	}
}

void wstjx_decode::decode(wsjtxMode mode, WsjTxVector &audiosamples, int freq, int threads)
{
	samplebuffer.push(move(audiosamples));

	std::memset(&params, 0, sizeof(params));
	params.nmode = 8;
	params.ntrperiod = 60.0;
	params.nfqso = freq;
	params.newdat = true;
	params.npts8 = 74736;
	params.nfa = 200;
	params.nfSplit = 2700;
	params.nfb = 4000;
	params.ntol = 20;
	params.kin = 64800;
	params.nzhsym = 79;
	params.nsubmode = 0;
	params.nagain = false;
	params.ndepth = 1;
	params.lft8apon = true;
	params.lapcqonly = false;
	params.ljt65apon = true;
	params.napwid = 75;
	params.ntxmode = 65;
	params.nmode = 8;
	params.minw = 0;
	params.nclearave = false;
	params.minSync = 0;
	params.emedelay = 0.0;
	params.dttol = 3;
	params.nlist = 0;
	params.listutc[0] = '\0';
	params.n2pass = 2;
	params.nranera = 6;
	params.naggressive = 0;
	params.nrobust = false;
	params.nexp_decode = 0;
	//params.max_drift;
	//params.datetime;
	//params.mycall;
	//params.mygrid;
	//params.hiscall;
	//params.hisgrid;
	switch (mode)
	{
	case FT8:
		params.nmode = 8;
		break;
	case FT4:
		params.nmode = 5;
		break;
	case JT4:
		params.nmode = 8;
		return; // not yet implememted
	case JT65:
		params.nmode = 65;
		return; // not yet implememted
	case JT9:
		params.nmode = 9;
		return; // not yet implememted
	case FST4:
		params.nmode = 240;
		return; // not yet implememted
	case FST4W:
		params.nmode = 241;
		return; // not yet implememted
	case Q65:
		params.nmode = 66;
		return; // not yet implememted
	case JT65JT9:
		params.nmode = 65 + 9;
		return; // not yet implememted
	case WSPR:
		return; // in this class
	}

	int nfsample = 12000;
	for (int i = 0; i < audiosamples.size(); i++)
	{
		dec_data.d2[i] = (short int) (audiosamples[i] * 32768.0f);
	}
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&tt);
	printf("start decode %02d:%02d:%02d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
	params.nutc = local_tm.tm_hour * 10000 + local_tm.tm_min * 100 + local_tm.tm_sec;
	
	fftwf_plan_with_nthreads(threads);
	multimode_decoder_(dec_data.ss, dec_data.d2 , &params, &nfsample);
}

void wstjx_decode::decode(wsjtxMode mode, IntWsjTxVector &audiosamples, int freq, int threads)
{
	std::memset(&params, 0, sizeof(params));
	params.nmode = 8;
	params.ntrperiod = 60.0;
	params.nfqso = freq;
	params.newdat = true;
	params.npts8 = 74736;
	params.nfa = 200;
	params.nfSplit = 2700;
	params.nfb = 4000;
	params.ntol = 20;
	params.kin = 64800;
	params.nzhsym = 50;
	params.nsubmode = 0;
	params.nagain = false;
	params.ndepth = 1;
	params.lft8apon = true;
	params.lapcqonly = false;
	params.ljt65apon = true;
	params.napwid = 75;
	params.ntxmode = 65;

	switch (mode)
	{
	case FT8:
		params.nmode = 8;
		break;
	case FT4:
		params.nmode = 5;
		break;
	case JT4:
		params.nmode = 8;
		return; // not yet implememted
	case JT65:
		params.nmode = 65;
		return; // not yet implememted
	case JT9:
		params.nmode = 9;
		return; // not yet implememted
	case FST4:
		params.nmode = 240;
		return; // not yet implememted
	case FST4W:
		params.nmode = 241;
		return; // not yet implememted
	case Q65:
		params.nmode = 66;
		return; // not yet implememted
	case JT65JT9:
		params.nmode = 65+9;
		return; // not yet implememted
	case WSPR:
		return; // not in this class
	}

	params.minw = 0;
	params.nclearave = false;
	params.minSync = 0;
	params.emedelay = 0.0;
	params.dttol = 3;
	params.nlist = 0;
	params.listutc[0] = '\0';
	params.n2pass = 2;
	params.nranera = 6;
	params.naggressive = 0;
	params.nrobust = false;
	params.nexp_decode = 0;
	//params.max_drift;
	//params.datetime;
	//params.mycall;
	//params.mygrid;
	//params.hiscall;
	//params.hisgrid;

	int nfsample = 12000;
	for (int i = 0; i < audiosamples.size(); i++)
	{
		dec_data.d2[i] = (short int)audiosamples[i];
	}
	
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&tt);
	printf("start decode %02d:%02d:%02d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
	params.nutc = local_tm.tm_hour * 10000 + local_tm.tm_min * 100 + local_tm.tm_sec;

	fftwf_plan_with_nthreads(threads);
	multimode_decoder_(dec_data.ss, dec_data.d2, &params, &nfsample);
}