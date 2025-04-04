#include <iostream>
#include <math.h>

using namespace std;

#define LOWPASS  	0x00
#define HIGHPASS 	0x01
#define BANDPASS 	0x02
#define NOTCH 	 	0x03
#define PEAK     	0x04
#define LOWSHELF 	0x05
#define HIGHSHELF 	0x06

// https://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
// https://www.earlevel.com/main/2011/01/02/biquad-formulas/

void calculateFilterCoefficients(uint8_t type, uint32_t sampleRate, uint32_t freq, float Q, float gain, float* coef) {
	
	float a0, a1, a2, b1, b2, norm;
	
	float PI = 3.141592653f;
	float SQ = 1.414213562f;
	float V = pow(10, fabs(gain) / 20.0);
	float K = tan(PI * freq / sampleRate);
	
	switch (type) {
		case LOWPASS:
			norm = 1.0f / (1.0f + K / Q + K * K);
			a0 = K * K * norm;
			a1 = 2.0f * a0;
			a2 = a0;
			b1 = 2.0f * (K * K - 1.0f) * norm;
			b2 = (1.0f - K / Q + K * K) * norm;
			break;
		
		case HIGHPASS:
			norm = 1 / (1 + K / Q + K * K);
			a0 = 1 * norm;
			a1 = -2 * a0;
			a2 = a0;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case BANDPASS:
			norm = 1 / (1 + K / Q + K * K);
			a0 = K / Q * norm;
			a1 = 0;
			a2 = -a0;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case NOTCH:
			norm = 1 / (1 + K / Q + K * K);
			a0 = (1 + K * K) * norm;
			a1 = 2 * (K * K - 1) * norm;
			a2 = a0;
			b1 = a1;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case PEAK:
			if (gain >= 0) {    // boost
				norm = 1 / (1 + 1/Q * K + K * K);
				a0 = (1 + V/Q * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - V/Q * K + K * K) * norm;
				b1 = a1;
				b2 = (1 - 1/Q * K + K * K) * norm;
			}
			else {    // cut
				norm = 1 / (1 + V/Q * K + K * K);
				a0 = (1 + 1/Q * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - 1/Q * K + K * K) * norm;
				b1 = a1;
				b2 = (1 - V/Q * K + K * K) * norm;
			}
			break;
		
		case LOWSHELF:
			if (gain >= 0) {    // boost
				norm = 1 / (1 + SQ * K + K * K);
				a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
				a1 = 2 * (V * K * K - 1) * norm;
				a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
				b1 = 2 * (K * K - 1) * norm;
				b2 = (1 - SQ * K + K * K) * norm;
			}
			else {    // cut
				norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
				a0 = (1 + SQ * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - SQ * K + K * K) * norm;
				b1 = 2 * (V * K * K - 1) * norm;
				b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
			}
			break;
		
		case HIGHSHELF:
			if (gain >= 0) {    // boost
				norm = 1 / (1 + SQ * K + K * K);
				a0 = (V + sqrt(2 * V) * K + K * K) * norm;
				a1 = 2 * (K * K - V) * norm;
				a2 = (V - sqrt(2 * V) * K + K * K) * norm;
				b1 = 2 * (K * K - 1) * norm;
				b2 = (1 - SQ * K + K * K) * norm;
			}
			else {    // cut
				norm = 1 / (V + sqrt(2 * V) * K + K * K);
				a0 = (1 + SQ * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - SQ * K + K * K) * norm;
				b1 = 2 * (K * K - V) * norm;
				b2 = (V - sqrt(2 * V) * K + K * K) * norm;
			}
			break;
		
		
		default:
		break;
	}
	
	*(coef + 0) = a0;
	*(coef + 1) = a1;
	*(coef + 2) = a2;
	*(coef + 3) = b1;
	*(coef + 4) = b2;	
}

int main(int argc, char *argv[]) {
	uint32_t cutoff[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000};
	
	for (int i = 0; i < (sizeof(cutoff)/sizeof(cutoff[0])); i++) {
		float coef[5];
		calculateFilterCoefficients(HIGHSHELF, 48000, cutoff[i], 0.7071, 6.0, coef);
		printf("{%f, %f, %f, %f, %f}, // %dHz\n", coef[0], coef[1], coef[2], coef[3], coef[4], cutoff[i]);
	}	
}