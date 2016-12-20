/*
 * Copyright (c) 2015-2016 Daniel Shihoon Lee <daniel@nansae.im>
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include <iostream>

namespace NSL {
  template <typename T>
    inline T StreamBinaryRead(std::istream &is) {
      T value;
      is.read(reinterpret_cast<char*>(&value), sizeof(T));
      return value;
    }

  template<>
    inline bool StreamBinaryRead<bool>(std::istream &is) {
      char c = StreamBinaryRead<char>(is);
      bool value = (bool)c;
      return value;
    }

  template <typename T>
    inline void StreamBinaryWrite(std::ostream& os, T value) {
      os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

  template <>
    inline void StreamBinaryWrite<bool>(std::ostream& os, bool value) {
      char c = (char)value;
      StreamBinaryWrite<char>(os, c);
    }
}
