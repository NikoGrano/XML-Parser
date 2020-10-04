#include "Stream.h"

#include <vector>
#include <iterator>
#include <stdexcept>

namespace Stream {
	namespace IO {
		StreamReader::StreamReader(const std::string& fname) {
			mInput.open(fname, std::ios::binary);
			if (!mInput.is_open()) {
				throw std::runtime_error("File not found");
            }
		}

		StreamReader::~StreamReader() {
			mInput.close();
		}

		std::string StreamReader::getString(size_t len) {
			std::string ret;
			if (len) {
				ret.reserve(len);
				std::vector<char> buf(len);
				mInput.read(reinterpret_cast<char*>(buf.data()), len);
				std::copy(buf.begin(), buf.end(), std::back_inserter(ret));
			}

			return ret;
		}

		std::string StreamReader::getNullTerminatedString() {
			char buf[MAX_STRING_LEN] = { 0 };
			mInput.getline(buf, MAX_STRING_LEN, '\0');
            
			return std::string(buf);
		}

		void StreamBufWriter::putString(const std::string& str, bool nullTerminate /* =true */ ) {
			mOutput << str;
			if (nullTerminate) {
				mOutput << '\0';
            }
		}
	}
}