#pragma once
#include "wsjtx_lib.h"

typedef std::vector<float> SampleVector;
typedef std::vector<short int> IntSampleVector;


class WsjtxMessage
{
  public:
	WsjtxMessage(int chh, int cmm, int css, int csnr, float cdt, int cfreq, std::string cmsg)
		: hh{chh}, min{cmm}, sec{css}, snr{csnr}, dt{cdt}, freq{cfreq}, msg{cmsg} {};

	WsjtxMessage()
		: hh{0}, min{0}, sec{0}, snr{0}, freq{0}, dt{0}, msg{""} {};

	//void display();

  private:
	int hh;
	int min;
	int sec;
	int snr;
	float dt;
	int freq;
	std::string msg;
};


class wstjx_decode
{
  public:
	void push_samples(SampleVector &audiosamples);
	void decode_ft8(SampleVector &audiosamples, int freq);
	void decode_ft8(IntSampleVector &audiosamples, int freq);
  private:
	params_t params;
	DataBuffer<float> samplebuffer;
	dec_data_t dec_data;
};