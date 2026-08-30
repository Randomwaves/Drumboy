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

using namespace std;
int main(int argc, char *argv[]) {
	ifstream infile;
	infile.open("44100_32_M.wav", ios::in);
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
	
	cout << endl;
	
	int16_t finalData[800];
	uint32_t finalDataCounter = 0;
	
	uint32_t rawDataSize = header.subChunk2Size;
	uint32_t rawArraySize = 32;
	uint32_t rawArrayCounter = 0;
	uint32_t rawArrayCounterMax = rawDataSize / rawArraySize;
	uint32_t rawArrayRemaining = rawDataSize % rawArraySize;
	
	uint8_t rawArray[rawArraySize];
	
	for (int i = 0; i <= rawArrayCounterMax; i++) {
		uint32_t readArraySize;
		uint16_t tempArraySize;
		// file to raw
		(i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemaining;
		infile.read((char*)&rawArray, readArraySize);
		// function
		
		// 16 bit
		
		// --> mono / stereo
		
		// ----> 44100 - 48000
		
		// ----> 88200 - 96000
		
		// ----> 176400 - 192000
		
		/*
		// 11.025 - 16bit - mono
		if ((header.sampleRate == 11025) && (header.bitPerSample == 16) && (header.channels == 1)) {
			// raw to temp
			tempArraySize = readArraySize * 2;
			int16_t tempArray[tempArraySize];
			for (int j = 0; j < readArraySize; j++) {
				int16_t* dataPtr = (int16_t*)&rawArray[j * 2];
				tempArray[j * 4] = *dataPtr;
				tempArray[(j * 4) + 1] = *dataPtr;
				tempArray[(j * 4) + 2] = *dataPtr;
				tempArray[(j * 4) + 3] = *dataPtr;
			}
			// temp to data
			for (int k = 0; k < tempArraySize; k++) {
				finalData[finalDataCounter] = tempArray[k];
				finalDataCounter += 1;
			}
		}
		
		// 22.050 - 16bit - mono
		if ((header.sampleRate == 22050) && (header.bitPerSample == 16) && (header.channels == 1)) {
			// raw to temp
			tempArraySize = readArraySize;
			int16_t tempArray[tempArraySize];
			for (int j = 0; j < readArraySize; j++) {
				int16_t* dataPtr = (int16_t*)&rawArray[j * 2];
				tempArray[j * 2] = *dataPtr;
				tempArray[(j * 2) + 1] = *dataPtr;
			}
			// temp to data
			for (int k = 0; k < tempArraySize; k++) {
				finalData[finalDataCounter] = tempArray[k];
				finalDataCounter += 1;
			}
		}
		
		// 44.1kHz - 16bit - mono
		if ((header.sampleRate == 44100) && (header.bitPerSample == 16) && (header.channels == 1)) {
			// raw to temp
			tempArraySize = readArraySize / 2;
			int16_t tempArray[tempArraySize];
			for (int j = 0; j < tempArraySize; j++) {
				int16_t* dataPtr = (int16_t*)&rawArray[j * 2];
				tempArray[j] = *dataPtr;
			}
			// temp to data
			for (int k = 0; k < tempArraySize; k++) {
				finalData[finalDataCounter] = tempArray[k];
				finalDataCounter += 1;
			}
		}
		
		
		// 88.2kHz - 16bit - mono
		if ((header.sampleRate == 88200) && (header.bitPerSample == 16) && (header.channels == 1)) {
			// raw to temp
			tempArraySize = readArraySize / 4;
			int16_t tempArray[tempArraySize];
			for (int j = 0; j < tempArraySize; j++) {
				int16_t* dataPtr = (int16_t*)&rawArray[j * 4];
				tempArray[j] = *dataPtr;
			}
			// temp to data
			for (int k = 0; k < tempArraySize; k++) {
				finalData[finalDataCounter] = tempArray[k];
				finalDataCounter += 1;
			}
		}
		
		// 176.4kHz - 16bit - mono
		if ((header.sampleRate == 176400) && (header.bitPerSample == 16) && (header.channels == 1)) {
			// raw to temp
			tempArraySize = readArraySize / 8;
			int16_t tempArray[tempArraySize];
			for (int j = 0; j < tempArraySize; j++) {
				int16_t* dataPtr = (int16_t*)&rawArray[j * 8];
				tempArray[j] = *dataPtr;
			}
			// temp to data
			for (int k = 0; k < tempArraySize; k++) {
				finalData[finalDataCounter] = tempArray[k];
				finalDataCounter += 1;
			}
		}
	*/	
	}
	
	infile.close();
	/*
	for (int l = 0; l < finalDataCounter; l++) {
		cout << finalData[l] << endl;
	}	
	*/
}