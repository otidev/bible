#ifndef WAVLOADER_H
#define WAVLOADER_H

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "assert.h"
#include "string.h"

typedef struct WaveFileH {
	uint8_t riffNum[4];
	uint32_t fileSize; // unused
	uint8_t waveNum[4];
	uint8_t firstChunkHeader[4]; // For some reason, some files have "fmt " and others have "JUNK" in this section
	uint32_t firstChunkLen;
	uint16_t formatType;
	uint16_t channels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint8_t dataChunkHeader[4];
	uint32_t dataSize;
} WaveFileH;

typedef struct int24_t {int32_t val : 24;} int24_t;

typedef enum WaveFormat {
	FMT_INT16 = 2,
	FMT_FLOAT32 = 4
} WaveFormat;

typedef struct Wave {
	int16_t channels;
	int32_t sampleRate;
	int16_t bitsPerSample;
	int32_t samplesPerChannel;
	WaveFormat format;
} Wave;

float** LoadWav(void* fileData, char* filename, Wave* waveData);

float** LoadWavFromFile(char* filename, Wave* wav);

void FreeAudio(float** audio, Wave* wav);

#endif
