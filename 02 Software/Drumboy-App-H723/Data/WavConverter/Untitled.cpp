#include <iostream>
#include <fstream>
#include <cmath>

struct Wav_Header {
	uint32_t   chunkId;       		/* 00 */		// 0x46464952
	uint32_t   fileSize;      		/* 04 */		// --
	uint32_t   fileFormat;    		/* 08 */		// 0x45564157
	uint32_t   subChunk1Id;   		/* 12 */		// 0x20746d66
	uint32_t   subChunk1Size; 		/* 16 */		// 0x10
	uint16_t   audioFormat;   		/* 20 */		// 0x01
	uint16_t   channels;      		/* 22 */		// --
	uint32_t   sampleRate;    		/* 24 */		// --
	uint32_t   byteRate;      		/* 28 */		// --
	uint16_t   blockAlign;    		/* 32 */		// --
	uint16_t   bitPerSample;  		/* 34 */		// --
	uint32_t   subChunk2Id;   		/* 36 */		// 0x61746164
	uint32_t   subChunk2Size; 		/* 40 */		// --
};

struct Wav_Header header;

int16_t buffer[4000];

using namespace std;
int main(int argc, char *argv[]) {
	
	ifstream infile("Snare_32_Mono.wav");
	infile.seekg(0, ios::end);
	size_t length = infile.tellg();
	infile.seekg(0, ios::beg);
	infile.read((char*)&header, sizeof(header));
	
	printf("ChunkId     0x%04x\n", header.chunkId);
	printf("FileSize    %d\n", header.fileSize);
	printf("FileFormat  0x%04x\n", header.fileFormat);
	printf("Chunk1Id    0x%04x\n", header.subChunk1Id);
	printf("Chunk1Size  0x%02x\n", header.subChunk1Size);
	printf("AudioFormat 0x%02x\n", header.audioFormat);
	printf("Channels    %d\n", header.channels);
	printf("SampleRate  %d\n", header.sampleRate);
	printf("ByteRate    %d\n", header.byteRate);
	printf("BlockAlign  %d\n", header.blockAlign);
	printf("BPS         %d\n", header.bitPerSample);
	printf("Chunk2Id    0x%04x\n", header.subChunk2Id);
	printf("Chunk2Size  %d\n", header.subChunk2Size);
	
	// 44.100 / 88.200 / 176.400
	
	if ((header.sampleRate == 44100) || (header.sampleRate == 88200) || (header.sampleRate == 176400)) {
		
		uint8_t offset;
		
		switch (header.sampleRate) {
			case 44100:
				offset = 0;
				break;
			
			case 88200:
				offset = 1;
				break;
			
			case 176400:
				offset = 2;
				break;
		}
		
		// 8bit - mono
		if ((header.bitPerSample == 8) && (header.channels == 1)) {
			for (int i = 0; i < header.subChunk2Size; i++) {
				uint8_t offset = 1;
				int16_t data = 0;
				infile.read((char*)&data, 1);
				infile.seekg(mult * offset, infile.cur);
				data -= 128;
				data = data << 8;
				buffer[i] = data;
			}
		}
		
		
		// 8bit - stereo - 44.1
		if ((header.bitPerSample == 8) && (header.channels == 2)) {
			for (int i = 0; i < header.subChunk2Size; i++) {
				uint8_t offset = 2;
				int16_t temp[2] = {};
				int32_t sum;
				int16_t data;
				infile.read((char*)&temp[0], 1);
				infile.read((char*)&temp[1], 1);
				infile.seekg(mult * offset, infile.cur);
				sum = temp[0] + temp[1];
				data = (round)((double)sum / 2);
				data -= 128;
				data = data << 8;
				buffer[i] = data;		
			}
		}
		
		// 16bit - mono - 44.1
		if ((header.bitPerSample == 16) && (header.channels == 1)) {
			for (int i = 0; i < header.subChunk2Size; i++) {
				int16_t data;
				infile.read((char*)&data, 2);
				buffer[i] = data;
			}
		}
		
		// 16bit - stereo - 44.1
		if ((header.bitPerSample == 16) && (header.channels == 2)) {
			for (int i = 0; i < (header.subChunk2Size / 2); i++) {
				int16_t temp[2];
				int32_t sum;
				int16_t data;
				infile.read((char*)&temp[0], 2);
				infile.read((char*)&temp[1], 2);
				sum = temp[0] + temp[1];
				data = (round)((double)sum / 2);
				buffer[i] = data;
			}
		}
		
		// 24bit - mono - 44.1	
		if ((header.bitPerSample == 24) && (header.channels == 1)) {
			for (int i = 0; i < header.subChunk2Size; i++) {
				int32_t data;
				infile.read((char*)&data, 3);
				data = data >> 8;
				buffer[i] = data;
			}
		}
		
		// 24bit - stereo - 44.1
		if ((header.bitPerSample == 24) && (header.channels == 2)) {
			for (int i = 0; i < (header.subChunk2Size / 2); i++) {
				int32_t temp[2];
				int32_t sum;
				int16_t data;
				infile.read((char*)&temp[0], 3);
				infile.read((char*)&temp[1], 3);
				sum = temp[0] + temp[1];
				data = (round)((double)sum / 2);
				data = data >> 8;
				buffer[i] = data;
			}
		}
		
		// 32bit - mono - 44.1	
		if ((header.bitPerSample == 32) && (header.channels == 1)) {
			for (int i = 0; i < header.subChunk2Size; i++) {
				float dataF;
				int16_t data;
				infile.read((char*)&dataF, 4);
				data = floor(dataF * 32767);
				buffer[i] = data;
			}
		}
		
		// 32bit - stereo - 44.1
		if ((header.bitPerSample == 32) && (header.channels == 2)) {
			for (int i = 0; i < (header.subChunk2Size / 2); i++) {
				float tempF[2];
				float sumF;
				int16_t data;
				infile.read((char*)&tempF[0], 4);
				infile.read((char*)&tempF[1], 4);
				sumF = tempF[0] + tempF[1];
				data = floor(sumF * 32767);
				buffer[i] = data;
			}
		}
		
		// print
		for (int i = 0; i < 15; i++)
			printf("%d\n", buffer[i]);
	}
}