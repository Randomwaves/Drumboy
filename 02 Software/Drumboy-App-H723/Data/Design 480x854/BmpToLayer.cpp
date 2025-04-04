#include <iostream>
#include <fstream>

using namespace std;

const char sourceName[] = "Image_Layer.bmp";
const char targetName[] = "Image_Layer.rwi";
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

const uint8_t kLayerColorRef[][3] = {
	{2, 2, 0},	// l 00
	{1, 2, 0},	// l 01
	{0, 2, 0},	// l 02
	{0, 2, 1},	// l 03
	{0, 2, 2},	// l 04
	{0, 1, 2},	// l 05
	{0, 0, 2},	// l 06
	{1, 0, 2},	// l 07
	{2, 0, 2},	// l 08
	{2, 0, 1},	// l 09
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
			
			uint8_t colorRef[bmp.paletteSize];
			
			uint8_t* pixelData = new uint8_t[bmp.width * bmp.height];
			
			for (int i = 0; i < bmp.paletteSize; i++) {
				colorRef[i] = buffer[bmp.paletteStartByte + (i * 4)];
				// printf("%d\n", colorRef[i]);
			}
			
			for (int i = 0; i < bmp.height; i++) {
				for (int j = 0; j < bmp.widthOffset; j++) {
					if (j < bmp.width) {
						pixelData[((bmp.height - 1 - i) * bmp.width) + j] = buffer[bmp.pixelStartByte + (i * bmp.widthOffset) + j];
					}
				}
			}
			
			source.close();
			
			// generate layer color palette
			
			uint16_t paletteSize;
			
			if ((bmp.paletteSize == 64) || (bmp.paletteSize == 128) || (bmp.paletteSize == 256)) {
				paletteSize = bmp.paletteSize;
			} else {
				if (bmp.paletteSize < 64) {
					paletteSize = 64;
				} else if (paletteSize < 128) {
					paletteSize = 128;
				} else if (paletteSize < 256) {
					paletteSize = 256;
				}
				for (int i = bmp.paletteSize; i < paletteSize; i++) {
					colorRef[i] = 0;
				}
			}
			
			RGB24Color colorPalette24[10][paletteSize];
			RGB16Color colorPalette16[10][paletteSize];
			
			// palette 24
			
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < paletteSize; j++) {
					uint8_t data;
					
					// red
					switch (kLayerColorRef[i][0]) {
						case 0:
							data = 0;
							break;
						
						case 1:
							data = colorRef[j] / 2;
							break;
						
						case 2:
							data = colorRef[j];
							break;
					}
					colorPalette24[i][j].red = data;
					// printf("%03d, ", data);
					
					// green
					switch (kLayerColorRef[i][1]) {
						case 0:
							data = 0;
							break;
						
						case 1:
							data = colorRef[j] / 2;
							break;
						
						case 2:
							data = colorRef[j];
							break;
					}
					colorPalette24[i][j].green= data;
					// printf("%03d, ", data);
					
					// blue
					switch (kLayerColorRef[i][2]) {
						case 0:
							data = 0;
							break;
						
						case 1:
							data = colorRef[j] / 2;
							break;
						
						case 2:
							data = colorRef[j];
							break;
					}
					colorPalette24[i][j].blue = data;
					// printf("%03d\n", data);
				}
				// printf("\n");
			}
			
			// palette 16
			
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < paletteSize; j++) {
					uint16_t red   = colorPalette24[i][j].red >> 3;
					uint16_t green = colorPalette24[i][j].green >> 2;
					uint16_t blue  = colorPalette24[i][j].blue >> 3;
					colorPalette16[i][j].color = (red << 11) | (green << 5) | (blue);
				}
			}
			
			// write to file
			
			ofstream target;
			target.open (targetName);
			
			char lo;
			char hi;
			
			
			target << "RW_IMAGE  ";

			lo = (paletteSize * 10) & 0xFF;
			hi = (paletteSize * 10) >> 8;
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
			
			switch (paletteType) {
				case 16:
					for (int i = 0; i < 10; i++) {
						for (int j = 0; j < paletteSize; j++) {
							uint16_t data = colorPalette16[i][j].color;
							lo = data & 0xFF;
							hi = data >> 8;
							target << lo;
							target << hi;
						}
					}
					break;
				
				case 24:
					for (int i = 0; i < 10; i++) {
						for (int j = 0; j < paletteSize; j++) {
							target << colorPalette24[i][j].blue;
							target << colorPalette24[i][j].green;
							target << colorPalette24[i][j].red;
						}
					}
					break;
			};
			
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
