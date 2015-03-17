#pragma once
#include <vector>
#include "Grid.h"

#define MAGIC_NUMBER_1 0x76615769 // = "iWav"
#define MAGIC_NUMBER_2 0x11111111
#define BLOCK_HEADER   0x4B4F4C42 // = "BLOK"

class ExternalFile
{
	std::wstring _filename;

public:
	ExternalFile(std::wstring filename);

	/**
	 * Returns the number of frames in the file, or -1 if the file is invalid.
	 */
	int CheckValidity() const;
	bool Read(int* startFrameOut, std::vector<Grid*>* gridsOut) const;
	bool Write(int startFrame, const std::vector<Grid*>& gridsIn) const;
};

