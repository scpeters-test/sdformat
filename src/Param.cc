/*
 * Copyright 2012 Open Source Robotics Foundation
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

#include <math.h>
#include <locale.h>
#include "sdf/Param.hh"

using namespace sdf;

class string_set : public boost::static_visitor<>
{
  public: string_set(const std::string &_value)
          {this->value = _value;}

  public: template <typename T>
          void operator()(T & _operand) const
          {
            _operand = boost::lexical_cast<T>(this->value);
          }
  public: std::string value;
};

class any_set : public boost::static_visitor<>
{
  public: any_set(const boost::any &_value)
          {this->value = _value;}

  public: template <typename T>
          void operator()(T & _operand) const
          {
            _operand = boost::any_cast<T>(this->value);
          }
  public: boost::any value;
};

//////////////////////////////////////////////////
Param::Param(const std::string &_key, const std::string &_typeName,
             const std::string &_default, bool _required,
             const std::string &_description)
  : dataPtr(new ParamPrivate)
{
  this->dataPtr->key = _key;
  this->dataPtr->required = _required;
  this->dataPtr->typeName = _typeName;
  this->dataPtr->description = _description;
  this->dataPtr->set = false;

  if (this->dataPtr->typeName == "bool")
    this->Init<bool>(_default);
  else if (this->dataPtr->typeName == "int")
    this->Init<int>(_default);
  else if (this->dataPtr->typeName == "unsigned int")
    this->Init<unsigned int>(_default);
  else if (this->dataPtr->typeName == "double")
    this->Init<double>(_default);
  else if (this->dataPtr->typeName == "float")
    this->Init<float>(_default);
  else if (this->dataPtr->typeName == "char")
    this->Init<char>(_default);
  else if (this->dataPtr->typeName == "std::string" ||
      this->dataPtr->typeName == "string")
    this->Init<std::string>(_default);
  else if (this->dataPtr->typeName == "sdf::Vector2i" ||
      this->dataPtr->typeName == "vector2i")
    this->Init<sdf::Vector2i>(_default);
  else if (this->dataPtr->typeName == "sdf::Vector2d" ||
      this->dataPtr->typeName == "vector2d")
    this->Init<sdf::Vector2d>(_default);
  else if (this->dataPtr->typeName == "sdf::Vector3" ||
       this->dataPtr->typeName == "vector3")
    this->Init<sdf::Vector3>(_default);
  else if (this->dataPtr->typeName == "sdf::Pose" ||
      this->dataPtr->typeName == "pose" || this->dataPtr->typeName == "Pose")
    this->Init<sdf::Pose>(_default);
  else if (this->dataPtr->typeName == "sdf::Quaternion" ||
      this->dataPtr->typeName == "quaternion")
    this->Init<sdf::Quaternion>(_default);
  else if (this->dataPtr->typeName == "sdf::Time" ||
      this->dataPtr->typeName == "time")
    this->Init<sdf::Time>(_default);
  else if (this->dataPtr->typeName == "sdf::Color" ||
      this->dataPtr->typeName == "color")
    this->Init<sdf::Color>(_default);
  else
    sdferr << "Unknown parameter type[" << this->dataPtr->typeName << "]\n";
}

//////////////////////////////////////////////////
Param::~Param()
{
  delete this->dataPtr;
  this->dataPtr = NULL;
}

//////////////////////////////////////////////////
bool Param::GetAny(boost::any &_anyVal) const
{
  if (typeid(int) == this->GetType())
  {
    int ret = 0;
    if (!this->Get<int>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(double) == this->GetType())
  {
    double ret = 0;
    if (!this->Get<double>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(float) == this->GetType())
  {
    float ret = 0;
    if (!this->Get<float>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(bool) == this->GetType())
  {
    bool ret = false;
    if (!this->Get<bool>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(std::string) == this->GetType())
  {
    std::string ret;
    if (!this->Get<std::string>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Vector3) == this->GetType())
  {
    sdf::Vector3 ret;
    if (!this->Get<sdf::Vector3>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(unsigned int) == this->GetType())
  {
    unsigned int ret = 0;
    if (!this->Get<unsigned int>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(char) == this->GetType())
  {
    char ret = 0;
    if (!this->Get<char>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Vector2i) == this->GetType())
  {
    sdf::Vector2i ret;
    if (!this->Get<sdf::Vector2i>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Vector2d) == this->GetType())
  {
    sdf::Vector2d ret;
    if (!this->Get<sdf::Vector2d>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Pose) == this->GetType())
  {
    sdf::Pose ret;
    if (!this->Get<sdf::Pose>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Quaternion) == this->GetType())
  {
    sdf::Quaternion ret;
    if (!this->Get<sdf::Quaternion>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Time) == this->GetType())
  {
    sdf::Time ret;
    if (!this->Get<sdf::Time>(ret))
      return false;
    _anyVal = ret;
  }
  else if (typeid(sdf::Color) == this->GetType())
  {
    sdf::Color ret;
    if (!this->Get<sdf::Color>(ret))
      return false;
    _anyVal = ret;
  }
  else
  {
    sdferr << "Type of parameter not known: [" << this->GetTypeName() << "]\n";
    return false;
  }
  return true;
}

//////////////////////////////////////////////////
void Param::Update()
{
  if (this->dataPtr->updateFunc)
  {
    try
    {
      boost::apply_visitor(any_set(this->dataPtr->updateFunc()),
      this->dataPtr->value);
    }
    catch(boost::bad_lexical_cast &/*e*/)
    {
      sdferr << "Unable to set value using Update for key["
        << this->dataPtr->key << "]\n";
    }
  }
}

//////////////////////////////////////////////////
std::string Param::GetAsString() const
{
  return boost::lexical_cast<std::string>(this->dataPtr->value);
}

//////////////////////////////////////////////////
std::string Param::GetDefaultAsString() const
{
  return boost::lexical_cast<std::string>(this->dataPtr->defaultValue);
}

//////////////////////////////////////////////////
bool Param::SetFromString(const std::string &_value)
{
  // Under some circumstances, latin locales (es_ES or pt_BR) will return a
  // comma for decimal position instead of a dot, making the conversion
  // to fail. See bug #60 for more information. Force to use always C
  setlocale(LC_NUMERIC, "C");

  std::string str = _value;
  boost::trim(str);

  if (str.empty() && this->dataPtr->required)
  {
    sdferr << "Empty string used when setting a required parameter. Key["
      << this->GetKey() << "]\n";
    return false;
  }
  else if (str.empty())
  {
    this->dataPtr->value = this->dataPtr->defaultValue;
    return true;
  }

  std::string tmp(str);
  std::string lowerTmp(str);
  boost::to_lower(lowerTmp);

  // "true" and "false" doesn't work properly
  if (lowerTmp == "true")
    tmp = "1";
  else if (lowerTmp == "false")
    tmp = "0";

  bool isHex = tmp.compare(0, 2, "0x") == 0;

  try
  {
    // If the string is hex, try to use stoi and stoul, and then
    // lexical cast as a last resort.
    if (isHex)
    {
      if (this->dataPtr->typeName == "int")
        this->dataPtr->value = std::stoi(tmp, NULL, 16);
      else if (this->dataPtr->typeName == "unsigned int")
      {
        this->dataPtr->value = static_cast<unsigned int>(
            std::stoul(tmp, NULL, 16));
      }
      else
      {
        boost::apply_visitor(string_set(tmp), this->dataPtr->value);
      }
    }
    // Otherwise use stod, stof, and lexical cast
    else
    {
      if (this->dataPtr->typeName == "int")
        this->dataPtr->value = std::stoi(tmp, NULL, 10);
      else if (this->dataPtr->typeName == "unsigned int")
      {
        this->dataPtr->value = static_cast<unsigned int>(
            std::stoul(tmp, NULL, 10));
      }
      else if (this->dataPtr->typeName == "double")
        this->dataPtr->value = std::stod(tmp);
      else if (this->dataPtr->typeName == "float")
        this->dataPtr->value = std::stof(tmp);
      else
        boost::apply_visitor(string_set(tmp), this->dataPtr->value);
    }
  }
  // Catch invalid argument exception from std::stoi/stoul/stod/stof
  catch(std::invalid_argument &)
  {
    sdferr << "Invalid argument. Unable to set value ["
      << str << " ] for key["
      << this->dataPtr->key << "].\n";
    return false;
  }
  // Catch out of range exception from std::stoi/stoul/stod/stof
  catch(std::out_of_range &)
  {
    sdferr << "Out of range. Unable to set value ["
      << str << " ] for key["
      << this->dataPtr->key << "].\n";
    return false;
  }
  // Catch boost lexical cast exceptions
  catch(boost::bad_lexical_cast &)
  {
    if (str == "inf" || str == "-inf")
    {
      // in this case, the parser complains, but seems to assign the
      // right values
      sdfmsg << "INFO [sdf::Param]: boost throws when lexical casting "
        << "inf's, but the values are usually passed through correctly\n";
    }
    else
    {
      sdferr << "Unable to set value [" <<  str
        << "] for key[" << this->dataPtr->key << "]\n";
      return false;
    }
  }

  this->dataPtr->set = true;
  return this->dataPtr->set;
}

//////////////////////////////////////////////////
void Param::Reset()
{
  this->dataPtr->value = this->dataPtr->defaultValue;
  this->dataPtr->set = false;
}

//////////////////////////////////////////////////
boost::shared_ptr<Param> Param::Clone() const
{
  return boost::shared_ptr<Param>(
  new Param(this->dataPtr->key, this->dataPtr->typeName,
      this->GetAsString(), this->dataPtr->required,
      this->dataPtr->description));
}

//////////////////////////////////////////////////
const std::type_info &Param::GetType() const
{
  return this->dataPtr->value.type();
}

//////////////////////////////////////////////////
const std::string &Param::GetTypeName() const
{
  return this->dataPtr->typeName;
}

/////////////////////////////////////////////////
void Param::SetDescription(const std::string &_desc)
{
  this->dataPtr->description = _desc;
}

/////////////////////////////////////////////////
std::string Param::GetDescription() const
{
  return this->dataPtr->description;
}

/////////////////////////////////////////////////
const std::string &Param::GetKey() const
{
  return this->dataPtr->key;
}

/////////////////////////////////////////////////
bool Param::GetRequired() const
{
  return this->dataPtr->required;
}

/////////////////////////////////////////////////
Param &Param::operator=(const Param &_param)
{
  this->dataPtr->value = _param.dataPtr->value;
  this->dataPtr->defaultValue  = _param.dataPtr->defaultValue;
  return *this;
}

/////////////////////////////////////////////////
bool Param::GetSet() const
{
  return this->dataPtr->set;
}
