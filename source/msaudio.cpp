#include "msaudio.hpp"
#pragma comment(lib, "xmaencoder.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

int msaudioGetSamplePerFrame(int cSamplePerSec, unsigned int dwBitPerSec, int iVersion, unsigned __int16 wEncOpt)
{
	unsigned int dwBytesPerFrame; // [esp+4h] [ebp-8h]
	int cSamplePerFrame; // [esp+8h] [ebp-4h]

	if (!dwBitPerSec && iVersion < 3 || iVersion > 3 || cSamplePerSec <= 0)
		return 0;
	if (cSamplePerSec > 8000)
	{
		if (cSamplePerSec > 11025)
		{
			if (cSamplePerSec > 16000)
			{
				if (cSamplePerSec > 22050)
				{
					if (cSamplePerSec > 32000)
					{
						if (cSamplePerSec > 44100)
						{
							if (cSamplePerSec > 48000)
							{
								if (cSamplePerSec > 96000)
									cSamplePerFrame = 0x2000;
								else
									cSamplePerFrame = 4096;
							}
							else
							{
								cSamplePerFrame = 2048;
							}
						}
						else
						{
							cSamplePerFrame = 2048;
						}
					}
					else if (iVersion == 1)
					{
						cSamplePerFrame = 1024;
					}
					else
					{
						cSamplePerFrame = 2048;
					}
				}
				else
				{
					cSamplePerFrame = 1024;
				}
			}
			else
			{
				cSamplePerFrame = 512;
			}
		}
		else
		{
			cSamplePerFrame = 512;
		}
	}
	else
	{
		cSamplePerFrame = 512;
	}
	if (iVersion == 3)
	{
		switch (wEncOpt & 6)
		{
		case 2:
			cSamplePerFrame *= 2;
			break;
		case 4:
			cSamplePerFrame /= 2;
			break;
		case 6:
			cSamplePerFrame /= 4;
			break;
		}
	}
	if (iVersion < 3)
	{
		dwBytesPerFrame = ((dwBitPerSec * cSamplePerFrame + cSamplePerSec / 2) / cSamplePerSec + 7) >> 3;
		if (!dwBytesPerFrame && !(dwBitPerSec * cSamplePerFrame))
		{
			dwBitPerSec = cSamplePerSec;
			dwBytesPerFrame = ((cSamplePerSec * cSamplePerFrame + cSamplePerSec / 2) / (unsigned int)cSamplePerSec + 7) >> 3;
		}
		if (dwBytesPerFrame <= 1)
		{
			while (!dwBytesPerFrame)
			{
				cSamplePerFrame *= 2;
				dwBytesPerFrame = ((dwBitPerSec * cSamplePerFrame + cSamplePerSec / 2) / cSamplePerSec + 7) >> 3;
			}
		}
	}
	return cSamplePerFrame;
}
