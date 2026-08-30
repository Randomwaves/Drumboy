#include <iostream>
#include <fstream>
#include <cmath>

char fileName[] = "44100_08_M.wav";

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
	infile.open(fileName, ios::in);
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
	
	int16_t sdramData[2000];
	uint32_t sdramDataCounter = 0;
	
	uint32_t rawDataSize = header.subChunk2Size;
	uint32_t rawArraySize = 96;
	uint32_t rawArrayCounter = 0;
	uint32_t rawArrayCounterMax = rawDataSize / rawArraySize;
	uint32_t rawArrayRemainder = rawDataSize % rawArraySize;
	
	uint8_t rawArray[rawArraySize];
	
	uint8_t coefSamplerate;
	uint8_t coefChannel;
	uint8_t coefBps;
	
	switch (header.sampleRate) {
		case 8000:
		case 16000:
		case 11025:
		case 22050:
		case 44100:
		case 48000:
			coefSamplerate = 1;
			break;
		
		case 88200:
		case 96000:
			coefSamplerate = 2;
			break;
		
		case 176400:
		case 192000:
			coefSamplerate = 4;
			break;
	}
	
	switch (header.channels) {
		case 1:
			coefChannel = 1;
			break;
		case 2:
			coefChannel = 2;
			break;
	}
	
	switch (header.bitPerSample) {
		case 8:
			coefBps = 1;
			break;
		
		case 16:
			coefBps = 2;
			break;
		
		case 24:
			coefBps = 3;
			break;
		
		case 32:
			coefBps = 4;
			break;
	}
	
	uint32_t offsetSize = coefSamplerate * coefChannel * coefBps;
	uint32_t readArraySize;
	uint32_t readSampleSize;
	uint32_t tempSampleSize;
	
	if (header.sampleRate < 44100) {
		uint16_t writeCount;
		switch (header.sampleRate) {
			case 8000:
				writeCount = 6;
				break;
			
			case 16000:
				writeCount = 3;
				break;
			
			case 11025:
				writeCount = 4;
				break;
			
			case 22050:
				writeCount = 2;
				break;
		}
		
		for (int i = 0; i <= rawArrayCounterMax; i++) {	
			// file to raw
			(i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemainder;
			infile.read((char*)&rawArray, readArraySize);
			
			// raw to temp
			readSampleSize = readArraySize / offsetSize;
			
			switch (header.sampleRate) {
				case 8000:
					tempSampleSize = readSampleSize * 6;
					break;
				
				case 16000:
					tempSampleSize = readSampleSize * 3;
					break;
				
				case 11025:
					tempSampleSize = readSampleSize * 4;
					break;
				
				case 22050:
					tempSampleSize = readSampleSize * 2;
					break;
			}
			
			int16_t tempArray[tempSampleSize];
			
			if (header.bitPerSample == 8) {
				for (int j = 0; j < readSampleSize; j++) {
					int8_t* dataPtr = (int8_t*)&rawArray[j * offsetSize];
					int16_t data = (int16_t)((*dataPtr - 128) << 8);
					
					for (int k = 0; k < writeCount; k++) {
						tempArray[(j * writeCount) + k] = data;
					}
				}
			} else if (header.bitPerSample == 16) {
				for (int j = 0; j < readSampleSize; j++) {
					int16_t* dataPtr = (int16_t*)&rawArray[j * offsetSize];
					int16_t data = *dataPtr;
					
					for (int k = 0; k < writeCount; k++) {
						tempArray[(j * writeCount) + k] = data;
					}
				}
			} else if (header.bitPerSample == 24) {
				for (int j = 0; j < readSampleSize; j++) {
					int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
					int16_t data = (int16_t)(*dataPtr >> 8);
					
					for (int k = 0; k < writeCount; k++) {
						tempArray[(j * writeCount) + k] = data;
					}
				}
			} else if (header.bitPerSample == 32) {
				for (int j = 0; j < readSampleSize; j++) {
					int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
					int16_t data = (int16_t)(*dataPtr >> 16);
					
					for (int k = 0; k < writeCount; k++) {
						tempArray[(j * writeCount) + k] = data;
					}
				}
			}
			
			// temp to sdram
			for (int l = 0; l < tempSampleSize; l++) {
				sdramData[sdramDataCounter] = tempArray[l];
				sdramDataCounter += 1;
			}
		}
	} else if (header.sampleRate >= 44100) {
		for (int i = 0; i <= rawArrayCounterMax; i++) {	
			
			// file to raw
			(i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemainder;
			infile.read((char*)&rawArray, readArraySize);
			
			// raw to temp
			readSampleSize = readArraySize / offsetSize;
			tempSampleSize = readSampleSize;
			int16_t tempArray[tempSampleSize];
			
			if (header.bitPerSample == 8) {
				for (int j = 0; j < readSampleSize; j++) {
					int8_t* dataPtr = (int8_t*)&rawArray[j * offsetSize];
					tempArray[j] = (int16_t)((*dataPtr - 128) << 8);
				}
			} else if (header.bitPerSample == 16) {
				for (int j = 0; j < readSampleSize; j++) {
					int16_t* dataPtr = (int16_t*)&rawArray[j * offsetSize];
					tempArray[j] = *dataPtr;
				}
			} else if (header.bitPerSample == 24) {
				for (int j = 0; j < readSampleSize; j++) {
					int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
					tempArray[j] = (int16_t)(*dataPtr >> 8);
				}
			} else if (header.bitPerSample == 32) {
				for (int j = 0; j < readSampleSize; j++) {
					int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
					tempArray[j] = (int16_t)(*dataPtr >> 16);
				}
			}
			
			// temp to sdram
			for (int k = 0; k < tempSampleSize; k++) {
				sdramData[sdramDataCounter] = tempArray[k];
				sdramDataCounter += 1;
			}
		}
	}
	infile.close();
	
	for (int l = 0; l < sdramDataCounter; l++) {
		cout << sdramData[l] << endl;
	}
	
};






/*
if (header.sampleRate == 11025) {

for (int i = 0; i <= rawArrayCounterMax; i++) {	

// file to raw
(i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemainder;
infile.read((char*)&rawArray, readArraySize);

// raw to temp
readSampleSize = readArraySize / offsetSize;
tempSampleSize = readSampleSize * 4;
int16_t tempArray[tempSampleSize];

if (header.bitPerSample == 8) {
for (int j = 0; j < readSampleSize; j++) {
int8_t* dataPtr = (int8_t*)&rawArray[j * offsetSize];
int16_t data = (int16_t)((*dataPtr - 128) << 8);
uint16_t inc = j * 4;
tempArray[inc] = data;
tempArray[inc + 1] = data;
tempArray[inc + 2] = data;
tempArray[inc + 3] = data;
}
} else if (header.bitPerSample == 16) {
for (int j = 0; j < readSampleSize; j++) {
int16_t* dataPtr = (int16_t*)&rawArray[j * offsetSize];
int16_t data = *dataPtr;
uint16_t inc = j * 4;
tempArray[inc] = data;
tempArray[inc + 1] = data;
tempArray[inc + 2] = data;
tempArray[inc + 3] = data;
}			
} else if (header.bitPerSample == 24) {
for (int j = 0; j < readSampleSize; j++) {
int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
int16_t data = (int16_t)(*dataPtr >> 8);
uint16_t inc = j * 4;
tempArray[inc] = data;
tempArray[inc + 1] = data;
tempArray[inc + 2] = data;
tempArray[inc + 3] = data;
}
} else if (header.bitPerSample == 32) {
for (int j = 0; j < readSampleSize; j++) {
int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
int16_t data = (int16_t)(*dataPtr >> 16);
uint16_t inc = j * 4;
tempArray[inc] = data;
tempArray[inc + 1] = data;
tempArray[inc + 2] = data;
tempArray[inc + 3] = data;
}
}

// temp to sdram
for (int k = 0; k < tempSampleSize; k++) {
sdramData[sdramDataCounter] = tempArray[k];
sdramDataCounter += 1;
}
}


} else if (header.sampleRate == 22050) {

for (int i = 0; i <= rawArrayCounterMax; i++) {	

// file to raw
(i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemainder;
infile.read((char*)&rawArray, readArraySize);

// raw to temp
readSampleSize = readArraySize / offsetSize;
tempSampleSize = readSampleSize * 2;
int16_t tempArray[tempSampleSize];

if (header.bitPerSample == 8) {
for (int j = 0; j < readSampleSize; j++) {
int8_t* dataPtr = (int8_t*)&rawArray[j * offsetSize];
int16_t data = (int16_t)((*dataPtr - 128) << 8);
uint16_t inc = j * 2;
tempArray[inc] = data;
tempArray[inc + 1] = data;
}
} else if (header.bitPerSample == 16) {
for (int j = 0; j < readSampleSize; j++) {
int16_t* dataPtr = (int16_t*)&rawArray[j * offsetSize];
int16_t data = *dataPtr;
uint16_t inc = j * 2;
tempArray[inc] = data;
tempArray[inc + 1] = data;
}
} else if (header.bitPerSample == 24) {
for (int j = 0; j < readSampleSize; j++) {
int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
int16_t data = (int16_t)(*dataPtr >> 8);
uint16_t inc = j * 2;
tempArray[inc] = data;
tempArray[inc + 1] = data;
}
} else if (header.bitPerSample == 32) {
for (int j = 0; j < readSampleSize; j++) {
int32_t* dataPtr = (int32_t*)&rawArray[j * offsetSize];
int16_t data = (int16_t)(*dataPtr >> 16);
uint16_t inc = j * 2;
tempArray[inc] = data;
tempArray[inc + 1] = data;
}
}

// temp to sdram
for (int k = 0; k < tempSampleSize; k++) {
sdramData[sdramDataCounter] = tempArray[k];
sdramDataCounter += 1;
}
}
*/
