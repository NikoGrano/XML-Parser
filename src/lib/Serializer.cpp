#include "Serializer.h"

#include "common/Stream.h"
#include "common/Base64.h"

#include <vector>
#include <sstream>
#include <algorithm>

namespace Serializer
{
	using namespace Engine;
	using namespace Stream::IO;

	static std::string serializeF(const std::vector<double>& floatVals)
	{
		std::stringstream _ret;
		StreamBufWriter ret(_ret.rdbuf());
		std::for_each(floatVals.begin(), floatVals.end(), [&](float v){ ret.put<float>(static_cast<float>(v)); });

		return _ret.str();
	}

	static std::string serializeI(const int64_t& intVal)
	{
		std::stringstream _ret;
		StreamBufWriter ret(_ret.rdbuf());

		uint64_t absVal = abs(intVal);
		if (absVal > std::numeric_limits<int32_t>::max()) {
			ret.put<int64_t>(intVal);

        } else if (absVal > std::numeric_limits<int16_t>::max()) {
			ret.put<int32_t>(static_cast<int>(intVal));
        } else if (absVal > std::numeric_limits<int8_t>::max()) {
			ret.put<int16_t>(static_cast<int16_t>(intVal));
        } else if (absVal != 0) {
			ret.put<int8_t>(static_cast<int8_t>(intVal));
        }

		return _ret.str();
	}

	static std::string serializeB(const bool boolVal)
	{
		std::stringstream _ret;
		StreamBufWriter ret(_ret.rdbuf());
		if (boolVal) {
			ret.put<uint8_t>(1);
        }

		return _ret.str();
	}


	rawDataBlock PackBuffer(const std::string& strVal) {
		if (strVal.empty()) {
			return rawDataBlock(TYPE_String, "");
		}

		// Trying to determine floats by dots. If ends with f, its string.
		if ((strVal.find('.') != std::string::npos) && (strVal.find('f') == std::string::npos)) {
			std::vector<double> values;
			double tmp;
			std::stringstream ss;

			ss << strVal;
			ss >> tmp;
			if (!ss.fail()) { // Float
				values.push_back(tmp);
				while (!ss.eof() && !ss.fail()) {
					ss >> tmp;
					if (!ss.fail()) {
						values.push_back(tmp);
                    }
				}

				return rawDataBlock(TYPE_Float, serializeF(values));
			}
		}

		{
			std::stringstream ss;
			ss << strVal;
			uint64_t i;
			ss >> i;
			if (!ss.fail() && ss.eof()) {
				return rawDataBlock(TYPE_Int, serializeI(i));
            }
		}

		{
			if (!strVal.compare("true")) {
				return rawDataBlock(TYPE_Bool, serializeB(true));
            } else if (!strVal.compare("false")) {
				return rawDataBlock(TYPE_Bool, serializeB(false));
            }
		}

		if (Base64::Is(strVal)) {
			return rawDataBlock(TYPE_Blob, Base64::Decode(strVal));
		}

		return rawDataBlock(TYPE_String, strVal);
	}}
