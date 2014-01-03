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

#include "sdf/SDFHelper.hh"

using namespace sdf;

/////////////////////////////////////////////////
SDFHelper::SDFHelper()
{
}

/////////////////////////////////////////////////
SDFHelper::~SDFHelper()
{
}

/////////////////////////////////////////////////
std::string SDFHelper::GetRootName() const
{
  return "sdf";
}

/////////////////////////////////////////////////
std::string SDFHelper::FindFile(const std::string & /*_filename*/,
    bool /*_searchLocalPath*/, bool /*_useCallback*/)
{
  sdferr << "Tried to use SDF Helper in sdf::findFile(), but a SDFHelper "
    "was not specified.  Did you call sdf::setSDFHelper()?";

  return std::string();
}

/////////////////////////////////////////////////
std::string SDFHelper::UriToFilename(const std::string &_uri)
{
  return _uri;
}

/////////////////////////////////////////////////
bool SDFHelper::ProcessInclude(SDFPtr /*_sdf*/, TiXmlElement * /*_elemXml*/)
{
  return true;
}
