#pragma once

#include <Windows.h>
#include "wave_format.hpp"

typedef unsigned int audecState;

typedef struct _WMAPlayerInfo
{
	WORD nPlayerOpt;        // Bit fields based on above defines 
	DWORD *rgiMixDownMatrix; // Can be null to invoke defaults 
	DWORD iPeakAmplitudeRef;
	DWORD iRmsAmplitudeRef;
	DWORD iPeakAmplitudeTarget;
	DWORD iRmsAmplitudeTarget;
	DWORD nDRCSetting;       // Dynamic range control setting 
} WMAPlayerInfo;

extern "C"
{
	void * __stdcall audecNew(
		void *pMemBuf,
		const int iMemBufSize);

	HRESULT __stdcall audecInit(
		void *pDecHandle,
		WMAFormat * pWMAFormat,
		PCMFormat * pPCMFormat,
		WMAPlayerInfo * pPlayerInfo,
		audecState * paudecState,
		struct audecInitParams * pParams);

	HRESULT __stdcall audecInput(
		void *pDecHandle,
		byte *pbIn,
		unsigned int cbIn,
		int fNewPacket,
		int fNoMoreInput,
		int fTIme,
		unsigned __int64 rtTime,
		audecState * paudecState,
		struct audecInputParams * pParams);

	HRESULT __stdcall audecDecode(
		void *pDecHandle,
		unsigned int *pcSamplesReady,
		audecState * paudecState,
		struct audecDecodeParams * pParams);

	HRESULT __stdcall audecGetPCM(
		void *pDecHandle,
		unsigned int cSamplesRequested,
		unsigned int *pcSamplesReturned,
		byte *pbDst,
		unsigned int cbDstLength,
		unsigned int *pcbDstUsed,
		__int64 *prtTime,
		audecState * paudecState,
		WMAPlayerInfo * pPI,
		struct _audecGetPCMParams * pParams);

	HRESULT __stdcall audecDelete(void *pDecHandle);
}

int msaudioGetSamplePerFrame(int cSamplePerSec, unsigned int dwBitPerSec, int iVersion, unsigned __int16 wEncOpt);
