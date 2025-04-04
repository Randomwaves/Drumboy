#include <iostream>
#include <fstream>
#include <stdint.h>

using namespace std;

const char sourceName[] = "Drumboy-App-H723.bin";
const char targetName[] = "Drumboy_Fw_01_01.bin";

const uint16_t versionMajor = 0x01;
const uint16_t versionMinor = 0x01;

static void crc_generateTable(uint32_t(&table)[256]) {
	uint32_t polynomial = 0xEDB88320;
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t c = i;
		for (uint8_t j = 0; j < 8; j++) {
			if (c & 1) {
				c = polynomial ^ (c >> 1);
			}
			else {
				c >>= 1;
			}
		}
		table[i] = c;
	}
}

uint32_t crc_update(uint32_t (&table)[256], uint32_t initial, const void* buf, uint32_t len) {
	uint32_t c = initial ^ 0xFFFFFFFF;
	const uint8_t* u = static_cast<const uint8_t*>(buf);
	for (uint32_t i = 0; i < len; ++i) {
		c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
	}
	return c ^ 0xFFFFFFFF;
}

int main(int argc, char *argv[]) {
	uint32_t crcTable[256];
	uint32_t crcValue = 0;
	
	crc_generateTable(crcTable);	
	
	ifstream source(sourceName, ifstream::binary);
	
	if (source) {
		source.seekg(0, source.end);
		uint fileSize = source.tellg();
		source.seekg(0, source.beg);
		
		char* buffer = new char[fileSize];
		source.read(buffer, fileSize);
		
		char *ptr = buffer;
		
		for (uint32_t i = 0; i < fileSize; i++) {
			crcValue = crc_update(crcTable, crcValue, ptr, 1);
			ptr += 1;
		}
		
		printf("%08x\n", crcValue);
		printf("%d\n", crcValue);
		
		ofstream target;
		target.open(targetName);
		
		target << "RW_DRUMBOY_FIRMWARE     ";
		
		target << (uint8_t)(versionMajor & 0xFF);
		target << (uint8_t)((versionMajor >> 8) & 0xFF);
		target << (uint8_t)(versionMinor & 0xFF);
		target << (uint8_t)((versionMinor >> 8) & 0xFF);		
		
		target << (uint8_t)(fileSize & 0xFF);
		target << (uint8_t)((fileSize >> 8) & 0xFF);
		target << (uint8_t)((fileSize >> 16) & 0xFF);		
		target << (uint8_t)((fileSize >> 24) & 0xFF);		
		
		target << (uint8_t)(crcValue & 0xFF);
		target << (uint8_t)((crcValue >> 8) & 0xFF);
		target << (uint8_t)((crcValue >> 16) & 0xFF);
		target << (uint8_t)((crcValue >> 24) & 0xFF);
		
		for (uint32_t i = 0; i < fileSize; i++) {
			target << buffer[i];
		}
		
		target.close();
	}
}