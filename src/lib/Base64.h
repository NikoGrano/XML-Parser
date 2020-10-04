#pragma once
#include <string>

namespace Base64 {
  std::string Encode(std::string src);
  std::string Decode(std::string src);
  bool Is(std::string src);
}