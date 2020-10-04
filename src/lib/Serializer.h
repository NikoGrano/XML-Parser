#pragma once

#include "Engine.hpp"
#include <string>
#include <cstdint>

namespace BWPack
{
	static const int BW_MATRIX_NROWS = 4;
	static const int BW_MATRIX_NCOLS = 3;
	static const int BW_MATRIX_SIZE = BW_MATRIX_NROWS * BW_MATRIX_NCOLS;

    rawDataBlock PackBuffer(const std::string& strVal);
	struct rawDataBlock {
		Engine::PackedDataType type;
		std::string data;
		rawDataBlock(const Engine::PackedDataType _type, const std::string _data): type(_type), data(_data) {};
	};

	struct dataBlock {
		uint16_t stringId;
		rawDataBlock data;
		dataBlock(const uint16_t _stringId, rawDataBlock _data): stringId(_stringId), data(_data) {};
	};
}