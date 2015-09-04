/*
 * Copyright 2015 Nate Koenig
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
#include <string>
#include <algorithm>
#include <functional>
#include <locale>

#include "sdf/system_util.hh"

namespace sdf
{
  /// \brief Windows equivalent of getEnv.
  /// Should only be called when using windows.
  /// \param[in] _name Name of the environment variable to get.
  /// \return Environment variable contents, or NULL on error.
  SDFORMAT_VISIBLE
  const char *winGetEnv(const char *_name);

  /// \brief Convert a variable to the specified type.
  ///
  /// This is a replacement to sdf::lexicalCast.
  ///
  /// \param[in] _var Variable to convert.
  /// \return The value contained in the variable in the specified type.
  /// \throws std::runtime_error When conversion results in an error.
  template<typename T, typename U>
  T lexicalCast(const U &_var)
  {
    // Send the variable into a stream.
    std::ostringstream out;
    out << _var;

    // Check for error
    /*if (out.bad())
      throw std::runtime_error("Read/write error in SDF lexicalCast output");
      */

    // Send the string version of the variable into the return type
    T var;
    std::istringstream stream;
    stream.str(out.str());
    stream >> var;

    // Check for error
    /*if (stream.bad())
      throw std::runtime_error("Read/write error in SDF lexicalCast input");
      */

    return var;
  }

  /// \brief Convert a string to an int.
  ///
  /// \param[in] _var String to convert.
  /// \return The value contained in the string.
  template<>
  inline int lexicalCast<int, std::string>(const std::string &_var)
  {
    return std::stoi(_var, NULL, _var.compare(0, 2, "0x") ? 10 : 16);
  }

  /// \brief Convert a string to an unsigned int.
  ///
  /// \param[in] _var String to convert.
  /// \return The value contained in the string.
  template<>
  inline unsigned int lexicalCast<unsigned int, std::string>(
      const std::string &_var)
  {
    return std::stoul(_var, NULL, _var.compare(0, 2, "0x") ? 10 : 16);
  }

  /// \brief Convert a string to a uint64_t.
  ///
  /// \param[in] _var String to convert.
  /// \return The value contained in the string.
  template<>
  inline uint64_t lexicalCast<uint64_t, std::string>(
      const std::string &_var)
  {
    return std::stoul(_var, NULL, _var.compare(0, 2, "0x") ? 10 : 16);
  }

  /// \brief Convert a string to a float.
  ///
  /// \param[in] _var String to convert.
  /// \return The value contained in the string.
  template<>
  inline float lexicalCast<float, std::string>(const std::string &_var)
  {
    return std::stof(_var);
  }

  /// \brief Convert a string to a double.
  ///
  /// \param[in] _var String to convert.
  /// \return The value contained in the string.
  template<>
  inline double lexicalCast<double, std::string>(const std::string &_var)
  {
    return std::stod(_var);
  }

  /// \brief Trim the left side of a string
  /// \param[in,out] _s String to trim.
  /// \return The string that was trimmed.
  inline std::string &ltrim(std::string &_s)
  {
    _s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(),
          std::not1(std::ptr_fun<int, int>(std::isspace))));
    return _s;
  }

  /// \brief Trim the right side of a string
  /// \param[in,out] _s String to trim.
  /// \return The string that was trimmed.
  inline std::string &rtrim(std::string &_s)
  {
    _s.erase(std::find_if(_s.rbegin(), _s.rend(),
          std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _s.end());
    return _s;
  }

  /// \brief Trim the both sides of a string
  /// \param[in,out] _s String to trim.
  /// \return The string that was trimmed.
  inline std::string &trim(std::string &_s)
  {
      return ltrim(rtrim(_s));
  }
}
#endif
