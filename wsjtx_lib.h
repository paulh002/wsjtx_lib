#pragma once
#include <vector>
#include <string>

typedef std::vector<float> WsjTxVector;
typedef std::vector<short int> IntWsjTxVector;

enum wsjtxMode
{
	FT8,
	FT4,
	JT4,
	JT65,
	JT9,
	FST4,
	Q65,
	FST4W,
	JT65JT9, // Dual
	WSPR
};

class WsjtxMessage
{
  public:
	WsjtxMessage(int chh, int cmm, int css, int csnr, float cdt, int cfreq, std::string cmsg)
		: hh{chh}, min{cmm}, sec{css}, snr{csnr}, dt{cdt}, freq{cfreq}, msg{cmsg} {};

	WsjtxMessage()
		: hh{0}, min{0}, sec{0}, snr{0}, freq{0}, dt{0}, msg{""} {};

  	int hh;
	int min;
	int sec;
	int snr;
	float dt;
	int freq;
	std::string msg;
};

class wsjtx_lib
{
  public:
	wsjtx_lib();

	void decode(wsjtxMode mode, WsjTxVector &audiosamples, int freq, int thread = 1);
	void decode(wsjtxMode mode, IntWsjTxVector &audiosamples, int freq, int thread = 1);
	std::vector<float> encode(wsjtxMode mode, int frequency, std::string message, std::string &messagesend);
	bool pullMessage(WsjtxMessage &msg);
};

