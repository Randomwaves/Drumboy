#include <iostream>
#include <fstream>

using namespace std;

const char sourceName[] = "Image_Menu.bmp";
const char targetName[] = "Image_Menu.rwi";
const uint16_t paletteType = 16; 	// 16 | 24

struct RGB24Color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct RGB16Color {
	uint16_t color;
};

struct BMP {
	uint fileSize;
	uint width;
	uint height;
	uint bitsPerPixel;
	uint compression;
	uint pixelSize;
	uint paletteSize;
	uint paletteStartByte;
	uint pixelStartByte;
	uint widthOffset;
};

uint32_t read4Bytes(char input[], uint start, bool endian) {
	uint32_t data;
	if (endian) {
		data = ((uint8_t)input[start + 3] << 24) | ((uint8_t)input[start + 2] << 16) | ((uint8_t)input[start + 1] << 8) | ((uint8_t)input[start]);
	} else {
		data = ((uint8_t)input[start] << 24) | ((uint8_t)input[start + 1] << 16) | ((uint8_t)input[start + 2] << 8) | ((uint8_t)input[start + 3]);
	}
	return data;
}

uint16_t read2Bytes(char input[], uint start, bool endian) {
	uint16_t data;
	if (endian) {
		data = ((uint8_t)input[start + 1] << 8) | ((uint8_t)input[start]);
	} else {
		data = ((uint8_t)input[start] << 8) | ((uint8_t)input[start + 1]);
	}
	return data;
}

int main(int argc, char *argv[]) {
	BMP bmp;
	char buffer[4];
	
	ifstream source(sourceName, ifstream::binary);
	
	if (source) {
		source.seekg(0, source.end);
		uint length = source.tellg();
		source.seekg(0, source.beg);
		
		char* buffer = new char[length];
		source.read(buffer, length);
		
		if ((source) && (read2Bytes(buffer, 0, false) == 0x424D)) {
			bmp.fileSize         = read4Bytes(buffer, 2, true);
			bmp.pixelStartByte   = read4Bytes(buffer, 10, true);
			bmp.paletteStartByte = read4Bytes(buffer, 14, true) + 14;
			bmp.width            = read4Bytes(buffer, 18, true);
			bmp.height           = read4Bytes(buffer, 22, true);
			bmp.bitsPerPixel     = read2Bytes(buffer, 28, true);
			bmp.compression      = read4Bytes(buffer, 30, true);
			bmp.pixelSize        = read4Bytes(buffer, 34, true) - 2;
			bmp.paletteSize      = read4Bytes(buffer, 46, true);
			
			(bmp.width % 4 == 0) ? bmp.widthOffset = bmp.width : bmp.widthOffset = bmp.width + (4 - (bmp.width % 4));
			
			cout << "BMP       " << endl;
			cout << "--------- " << endl;
			cout << "Width   : " << bmp.width << endl;
			cout << "Height  : " << bmp.height << endl;
			cout << "BPP     : " << bmp.bitsPerPixel << endl;
			cout << "Palette : " << bmp.paletteSize << endl;
			cout << "Pixel   : " << bmp.pixelSize << endl;
			
			RGB24Color* colorPalette24 = new RGB24Color[bmp.paletteSize];
			RGB16Color* colorPalette16 = new RGB16Color[bmp.paletteSize];
			uint8_t* pixelData         = new uint8_t[bmp.width * bmp.height];
			
			for (int i = 0; i < bmp.paletteSize; i++) {
				colorPalette24[i].blue  = buffer[bmp.paletteStartByte + (i * 4)];
				colorPalette24[i].green = buffer[bmp.paletteStartByte + (i * 4) + 1];
				colorPalette24[i].red   = buffer[bmp.paletteStartByte + (i * 4) + 2];
			}
			
			for (int j = 0; j < bmp.paletteSize; j++) {
				uint16_t red   = colorPalette24[j].red >> 3;
				uint16_t green = colorPalette24[j].green >> 2;
				uint16_t blue  = colorPalette24[j].blue >> 3;
				colorPalette16[j].color = (red << 11) | (green << 5) | (blue);
				// printf("%d\n", (uint16_t)colorPalette16[j].color);
			}

			
			for (int i = 0; i < bmp.height; i++) {
				for (int j = 0; j < bmp.widthOffset; j++) {
					if (j < bmp.width) {
						pixelData[((bmp.height - 1 - i) * bmp.width) + j] = buffer[bmp.pixelStartByte + (i * bmp.widthOffset) + j];
					}
				}
			}
			
			source.close();
			
			// write to file
			
			ofstream target;
			target.open (targetName);
			char lo;
			char hi;
			uint16_t paletteSize;
			
			if ((bmp.paletteSize == 64) || (bmp.paletteSize == 128) || (bmp.paletteSize == 256)) {
				paletteSize = bmp.paletteSize;
			} else if (bmp.paletteSize < 64) {
				paletteSize = 64;
			} else if (paletteSize < 128) {
				paletteSize = 128;
			} else if (paletteSize < 256) {
				paletteSize = 256;
			}
			
			target << "RW_IMAGE  ";
			
			lo = paletteSize & 0xFF;
			hi = paletteSize >> 8;
			target << lo;
			target << hi;

			lo = paletteType & 0xFF;
			hi = paletteType >> 8;
			target << lo;
			target << hi;

			lo = bmp.width & 0xFF;			
			hi = bmp.width >> 8;
			target << lo;
			target << hi;

			lo = bmp.height & 0xFF;			
			hi = bmp.height >> 8;
			target << lo;
			target << hi;
			
			target << "_PA";
			
			for (int i = 0; i < bmp.paletteSize; i++) {
				switch (paletteType) {
					case 16:
						lo = colorPalette16[i].color & 0xFF;
						hi = colorPalette16[i].color >> 8;
						target << lo;
						target << hi;
						break;
					
					case 24:
					default:
						target << colorPalette24[i].blue;
						target << colorPalette24[i].green;
						target << colorPalette24[i].red;
						break;
				}
			}
			
			if (bmp.paletteSize != paletteSize) {
				uint8_t emptyData = 0;
				switch (paletteType) {
					case 16:
						for (int i = 0; i < (paletteSize - bmp.paletteSize); i++) {
							target << emptyData;
							target << emptyData;
						}
						break;
					case 24:
						for (int i = 0; i < (paletteSize - bmp.paletteSize); i++) {
							target << emptyData;
							target << emptyData;
							target << emptyData;
						}
						break;
				}
			}
			
			target << "_DA";
			
			for (int j = 0; j < (bmp.width * bmp.height); j++) {
				target << pixelData[j];
			}
			
			target.close();
		} else {
			cout << "Read Error" << endl;
		}
	}
}