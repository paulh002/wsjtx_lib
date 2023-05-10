#include "wsjtx_encode.h"
#include "DataBuffer.h"
#include "commons.h"
#include "fortran_interface.h"



std::vector<float> wsjtx_encode::encode_ft8(wsjtxMode mode, int frequency, std::string message)
{
	bool is_ft4{mode == wsjtxMode::FT4};
	std::vector<float> signal;
	int nsym = FT8_NN;

	int i3 = 0;
	int n3 = 0;
	char ft8msgbits[77];
	std::string msgsent;

	message.resize(38);
	msgsent.resize(38);
	genft8_((char *)message.c_str(), &i3, &n3, (char *)msgsent.c_str(), const_cast<char *>(ft8msgbits),
			const_cast<int *>(itone), 37, 37);

	printf("FSK tones: ");
	for (int j = 0; j < nsym; ++j)
	{
		printf("%d", itone[j]);
	}
	printf("\n");

	float fsample = 48000.0;
	int nsps = 4 * 1920;
	float bt = 2.0;
	int icmplx = 0;
	int nwave = nsym * nsps;
	float f0 = frequency;

	float symbol_period = FT8_SYMBOL_PERIOD;
	float symbol_bt = FT8_SYMBOL_BT;
	float slot_time = FT8_SLOT_TIME;
	int num_samples = (int)(0.5f + nsym * symbol_period * fsample);

	signal.clear();
	signal.resize(num_samples);

	gen_ft8wave_(const_cast<int *>(itone), &nsym, &nsps, &bt, &fsample, &f0, signal.data(),
				 signal.data(), &icmplx, &nwave);
	signal.resize(nwave);
	printf("frequency %d number of tones %d, samplerate %d no samples %d\n", frequency, nsym, FT8_SAMPLERATE, nwave);
	//save_wav(signal.data(), signal.size(), FT8_SAMPLERATE, "./wave.wav");
	return signal;
}

//void genft4_(char *msg, int *ichk, char *msgsent, char ft4msgbits[], int itone[],
//			 fortran_charlen_t, fortran_charlen_t);


std::vector<float> wsjtx_encode::encode_ft4(wsjtxMode mode, int frequency, std::string message)
{
	int ichk = 0;
	char ft4msgbits[77];
	std::string msgsent;
	std::vector<float> signal;
	
	message.resize(38);
	msgsent.resize(38);
	genft4_((char *)message.c_str(), &ichk, (char *)msgsent.c_str(), const_cast<char *>(ft4msgbits),
			const_cast<int *>(itone), 37, 37);
	int nsym = 103;
	int nsps = 4 * 576;
	float fsample = 48000.0;
	float f0 = frequency; //ui->TxFreqSpinBox->value() - m_XIT;
	int nwave = (nsym + 2) * nsps;
	int icmplx = 0;
	float symbol_period = FT4_SYMBOL_PERIOD;
	int num_samples = (int)(0.5f + nsym * symbol_period * fsample);
	
	printf("FSK tones: ");
	for (int j = 0; j < nsym; ++j)
	{
		printf("%d", itone[j]);
	}
	printf("\n");

	signal.clear();
	signal.resize(num_samples);
	gen_ft4wave_(const_cast<int *>(itone), &nsym, &nsps, &fsample, &f0, signal.data(),
				 foxcom_.wave, &icmplx, &nwave);
	return signal;
}