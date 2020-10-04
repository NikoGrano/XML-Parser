#pragma once

#include <cassert>
#include <cstdint>

namespace Engine {
	static const uint32_t PACKED_SECTION_MAGIC = 0x62a14e45;
    enum PackedDataType {
		TYPE_Section  = 0x0,
		TYPE_String   = 0x1,
		TYPE_Int      = 0x2,
		TYPE_Float    = 0x3,
		TYPE_Bool     = 0x4,
		TYPE_Blob     = 0x5,
		TYPE_Enc_blob = 0x6,
		TYPE_Reserved = 0x7
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