/*
 * Copyright 2018 Open Source Robotics Foundation
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
#include "sdf/Light.hh"
#include "Utils.hh"

using namespace sdf;

class sdf::LightPrivate
{
  /// \brief Name of the light.
  public: std::string name = "";
};

/////////////////////////////////////////////////
Light::Light()
  : dataPtr(new LightPrivate)
{
}

/////////////////////////////////////////////////
Light::~Light()
{
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

/////////////////////////////////////////////////
Errors Light::Load(ElementPtr _sdf)
{
  Errors errors;

  // Check that the provided SDF element is a <light>
  // This is an error that cannot be recovered, so return an error.
  if (_sdf->GetName() != "light")
  {
    errors.push_back({ErrorCode::ELEMENT_INCORRECT_TYPE,
        "Attempting to load a Light, but the provided SDF element is not a "
        "<light>."});
    return errors;
  }

  // Read the lights's name
  if (!loadName(_sdf, this->dataPtr->name))
  {
    errors.push_back({ErrorCode::ATTRIBUTE_MISSING,
                     "A light name is required, but the name is not set."});
  }

  return errors;
}

/////////////////////////////////////////////////
std::string Light::Name() const
{
  return this->dataPtr->name;
}

/////////////////////////////////////////////////
void Light::SetName(const std::string &_name) const
{
  this->dataPtr->name = _name;
}
