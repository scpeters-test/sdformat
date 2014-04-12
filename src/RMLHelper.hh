/*
 * Copyright 2013 Open Source Robotics Foundation
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
#ifndef _RML_HELPER_HH_
#define _RML_HELPER_HH_

#include <sdf/sdf.hh>

namespace rml
{
  class RMLHelper : public sdf::SDFHelper
  {
    /// \brief Get the name of the root XML element, "rml". 
    /// \return "rml", the name of the root XML element.
    public: virtual std::string GetRootName() const;

    /// \brief Convert a URI to a filename. This allows special processing of
    /// URI strings.
    /// \param[in] _uri The URI string.
    /// \return A string containing the fully-qualified path to an SDF file.
    public: virtual std::string UriToFilename(const std::string &_uri);

    /// \brief Allow special processing of included files.
    /// \param[in,out] _sdf The SDF that will be included..
    /// \param[in] _elemXml Raw XML for the <include>.
    /// \return True if successful.
    public: bool ProcessInclude(sdf::SDFPtr _sdf, TiXmlElement *_elemXml);
  };
}
#endif
