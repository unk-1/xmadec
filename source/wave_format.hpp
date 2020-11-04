#pragma once
#include <Windows.h>

// Used in XMAWAVEFORMAT for per-stream data
struct XMASTREAMFORMAT
{
	DWORD PsuedoBytesPerSec; // Used by the XMA encoder (typo preserved for legacy reasons)
	DWORD SampleRate;        // The stream's decoded sample rate (in XMA2 files,
							 // this is the same for all streams in the file).
	DWORD LoopStart;         // Bit offset of the frame containing the loop start
							 // point, relative to the beginning of the stream.
	DWORD LoopEnd;           // Bit offset of the frame containing the loop end.
	BYTE  SubframeData;      // Two 4-bit numbers specifying the exact location of
							 // the loop points within the frames that contain them.
							 //   SubframeEnd: Subframe of the loop end frame where
							 //                the loop ends.  Ranges from 0 to 3.
							 //   SubframeSkip: Subframes to skip in the start frame to
							 //                 reach the loop.  Ranges from 0 to 4.
	BYTE  Channels;          // Number of channels in the stream (1 or 2)
	WORD  ChannelMask;       // Spatial positions of the channels in the stream
};

// Legacy XMA1 format structure
struct XMAWAVEFORMAT
{
	WORD FormatTag;          // Audio format type (always WAVE_FORMAT_XMA)
	WORD BitsPerSample;      // Bit depth (currently required to be 16)
	WORD EncodeOptions;      // Options for XMA encoder/decoder
	WORD LargestSkip;        // Largest skip used in interleaving streams
	WORD NumStreams;         // Number of interleaved audio streams
	BYTE LoopCount;          // Number of loop repetitions; 255 = infinite
	BYTE Version;            // XMA encoder version that generated the file.
							 // Always 3 or higher for XMA2 files.
	XMASTREAMFORMAT XmaStreams[1]; // Per-stream format information; the actual
								   // array length is in the NumStreams field.
};

#pragma pack(push, 1)
struct WMAFormat
{
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD nBlockAlign;
	WORD nValidBitsPerSample;
	DWORD nChannelMask;
	WORD wEncodeOpt;
};
#pragma pack(pop)

enum PCMData
{
	PCMDataPCM = 0x0,
	PCMDataIEEE_FLOAT = 0x1,
};

struct PCMFormat
{
	DWORD nSamplesPerSec;
	DWORD nChannels;
	DWORD nChannelMask;
	DWORD nValidBitsPerSample;
	DWORD cbPCMContainerSize;
	enum PCMData pcmData;
};
