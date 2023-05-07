#include "wsjtx_decode.h"
#include <cstring>
#include <string>

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

		printf("nutc %d, snr %d, dt %f, freq %d message %s \n", *nutc, *snr, *dt, *freq, message);
		WsjtxMessageQueue.push(WsjtxMessage(*nutc / 1000 ,*nutc % 100, *nutc, *snr, *dt ,*freq, std::string(message)));
	}

}

void wstjx_decode::push_samples(SampleVector &audiosamples)
{
	samplebuffer.push(move(audiosamples));
}

void wstjx_decode::decode_ft8(SampleVector &audiosamples, int freq)
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
	params.nzhsym = 50;
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

	int nfsample = 12000;
	for (int i = 0; i < audiosamples.size(); i++)
	{
		dec_data.d2[i] = (short int) (audiosamples[i] * 32768.0f);
	}
	multimode_decoder_(dec_data.ss, dec_data.d2 , &params, &nfsample);
}

void wstjx_decode::decode_ft8(IntSampleVector &audiosamples, int freq)
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

	int nfsample = 12000;
	for (int i = 0; i < audiosamples.size(); i++)
	{
		dec_data.d2[i] = (short int)audiosamples[i];
	}
	multimode_decoder_(dec_data.ss, dec_data.d2, &params, &nfsample);
}