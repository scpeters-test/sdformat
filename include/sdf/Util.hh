/*
 * Copyright 2011 Nate Koenig
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef _SDF_UTIL_HH_
#define _SDF_UTIL_HH_

#include <stdexcept>
#include <sstream>

#include "sdf/system_util.hh"

namespace sdf
{
  /// \brief Windows equivalent of getEnv.
  /// Should only be called when using windows.
  /// \param[in] _name Name of the environment variable to get.
  /// \return Environment variable contents, or NULL on error.
  SDFORMAT_VISIBLE
  const char *winGetEnv(const char *_name);

  /// \brief Convert a string to the specified type.
  ///
  /// This is a replacement to sdf::lexicalCast.
  //
  /// \param[in] _str String to convert.
  /// \return The value contained in the string.
  /// \throws std::runtime_error When conversion results in an error.
  template<typename T>
  T lexicalCast(const std::string &_str)
  {
    T var;
    std::istringstream stream;
    stream.str(_str);
    stream >> var;
    if (stream.bad())
    {
      throw std::runtime_error("Read/write error in SDF lexicalCast");
    }
    else if (stream.fail())
    {
      throw std::runtime_error("Logical error in SDF lexicalCast");
    }
    return var;
  }
}
#endif
