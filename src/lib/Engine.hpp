#pragma once

#include <cassert>
#include <cstdint>

namespace Engine {
    enum PackedDataType {
		BW_Section  = 0x0,
		BW_String   = 0x1,
		BW_Int      = 0x2,
		BW_Float    = 0x3,
		BW_Bool     = 0x4,
		BW_Blob     = 0x5,
		BW_Enc_blob = 0x6,
		BW_Reserved = 0x7
	};

#pragma pack(push, 1)
	class DataDescriptor {
        uint32_t rawData;
        public:
		    DataDescriptor(): rawData(0) {};
		    DataDescriptor(PackedDataType type, uint32_t offset) {
			    assert(offset < 0x0FFFFFFF);
			    rawData = ((static_cast<uint32_t>(type) << 28) | offset);
		    }

		    PackedDataType typeId() const { 
                return static_cast<PackedDataType>(rawData >> 28); 
            };
		    uint32_t offset() const { 
                return rawData & 0x0FFFFFFF; 
            };
	};

    struct DataNode {
		uint16_t nameIdx;
		DataDescriptor data;
	};
	
    static_assert(sizeof(DataDescriptor) == 4, "Wrong sizeof DataDescriptor.");
    static_assert(sizeof(DataNode) == 6, "Wrong sizeof DataNode.");

#pragma pack(pop)
}