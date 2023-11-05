#include "WavLoader.h"

#define SwitchEndianness(x) (x << 24) | (((x >> 16) << 24) >> 16) | (((x << 16) >> 24) << 16) | (x >> 24)

float** LoadWav(void* fileData, char* filename, Wave* waveData) {
	WaveFileH* wav = (WaveFileH*) fileData;
	assert(wav->riffNum[0] == 'R' && wav->riffNum[1] == 'I' && wav->riffNum[2] == 'F' && wav->riffNum[3] == 'F');
	assert(wav->waveNum[0] == 'W' && wav->waveNum[1] == 'A' && wav->waveNum[2] == 'V' && wav->waveNum[3] == 'E');
	assert(
		(wav->firstChunkHeader[0] == 'f' && wav->firstChunkHeader[1] == 'm' && wav->firstChunkHeader[2] == 't' && wav->firstChunkHeader[3] == ' ') ||
		(wav->firstChunkHeader[0] == 'J' && wav->firstChunkHeader[1] == 'U' && wav->firstChunkHeader[2] == 'N' && wav->firstChunkHeader[3] == 'K')
	);

	if (wav->firstChunkHeader[0] == 'J' && wav->firstChunkHeader[1] == 'U' && wav->firstChunkHeader[2] == 'N' && wav->firstChunkHeader[3] == 'K') {
		wav = (WaveFileH*)((uintptr_t)fileData + (uintptr_t)sizeof(wav->firstChunkHeader) + (uintptr_t)wav->firstChunkLen + (uintptr_t)sizeof(wav->firstChunkLen));
		assert(wav->firstChunkHeader[0] == 'f' && wav->firstChunkHeader[1] == 'm' && wav->firstChunkHeader[2] == 't' && wav->firstChunkHeader[3] == ' ');
	}

	waveData->channels = wav->channels;
	waveData->sampleRate = wav->sampleRate;
	waveData->bitsPerSample = wav->bitsPerSample;
	if (wav->bitsPerSample == 16 && wav->formatType == 1) {
		waveData->format = FMT_INT16;
	} if (wav->bitsPerSample == 32 && wav->formatType == 3) {
		waveData->format = FMT_FLOAT32;
	} else {
		waveData->format = wav->formatType + 1;
	}

	while (!(wav->dataChunkHeader[0] == 'd' && wav->dataChunkHeader[1] == 'a' && wav->dataChunkHeader[2] == 't' && wav->dataChunkHeader[3] == 'a')) {
		wav = (WaveFileH*)((uintptr_t)wav + (uintptr_t)sizeof(wav->dataChunkHeader) + (uintptr_t)wav->dataSize + (uintptr_t)sizeof(wav->dataSize));
	}

	assert(wav->dataChunkHeader[0] == 'd' && wav->dataChunkHeader[1] == 'a' && wav->dataChunkHeader[2] == 't' && wav->dataChunkHeader[3] == 'a');


	int16_t max = 32767;
	waveData->samplesPerChannel = wav->dataSize / waveData->format; // huh? I've seen many implementations of this but not this one.
	float** buffer = (float**)malloc(waveData->channels * sizeof(float*));
	for (int i = 0; i < waveData->channels; i++) {
		buffer[i] = (float*)malloc(waveData->samplesPerChannel * sizeof(float));
	}
	char* audioStart = (char*)&wav->dataSize + sizeof(wav->dataSize);

	for (int channel = 0; channel < waveData->channels; channel++) {
		for (uint32_t sample = (waveData->channels - 1 - channel); sample < waveData->samplesPerChannel + (waveData->channels - 1 - channel); sample += waveData->channels) { // .wav is interleaved so I have to read like this
			if (waveData->format == FMT_INT16) {
				buffer[channel][sample] = (float)((int16_t*)audioStart)[channel * waveData->channels + sample] / max;
			} if (waveData->format == FMT_FLOAT32) {
				buffer[channel][sample] = ((float*)audioStart)[channel * waveData->channels + sample];
			}
		}
	}

	assert(buffer);
	return (float**)buffer;
}

float** LoadWavFromFile(char* filename, Wave* wav) {
	FILE* f = fopen(filename, "rb");
	assert(f);

	fseek(f, 0l, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0l, SEEK_SET);

	char* fileBuf = malloc(sizeof(char) * fileSize);
	fread(fileBuf, sizeof(char), fileSize, f);

	fclose(f);
	float** buffer = NULL;
	buffer = LoadWav(fileBuf, filename, wav);

	free(fileBuf);
	fileBuf = NULL;

	assert(buffer);
	return buffer;
}

void FreeAudio(float** audio, Wave* wav) {
	for (int i = 0; i < wav->channels; i++) {
		free(audio[i]);
		audio[i] = NULL;
	}

	free(audio);
	audio = NULL;
}
