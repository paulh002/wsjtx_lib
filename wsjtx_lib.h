#pragma once
#include <vector>
#include <string>

typedef std::vector<float> SampleVector;
typedef std::vector<short int> IntSampleVector;

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
	JT65JT9 // Dual
};

class wsjtx_lib
{
  public:
	wsjtx_lib();

	void decode(wsjtxMode mode, SampleVector &audiosamples, int freq, int thread = 1);
	void decode(wsjtxMode mode, IntSampleVector &audiosamples, int freq, int thread = 1);
	std::vector<float> encode(wsjtxMode mode, int frequency, std::string message);
};