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
#ifndef _SDF_HELPER_HH_
#define _SDF_HELPER_HH_

#include <string>
#include "sdf/SDFImpl.hh"

class TiXmlElement;

namespace sdf
{
  class SDFORMAT_VISIBLE SDFHelper
  {
    /// \brief Constructor
    public: SDFHelper();

    /// \brief Destructor
    public: virtual ~SDFHelper();

    /// \brief Get the root element name. The default is "sdf"
    /// \return Root element name.
    public: virtual std::string GetRootName() const;

    /// \brief Associate paths to a URI.
    /// Example paramters: "model://", "/usr/share/models:~/.gazebo/models"
    /// \param[in] _uri URI that will be mapped to _path
    /// \param[in] _path Colon separated set of paths.
    public: virtual std::string FindFile(const std::string &_filename,
                bool _searchLocalPath = true,
                bool _useCallback = false);

    /// \brief Convert a URI to a filename. This allows special processing of
    /// URI strings.
    /// \param[in] _uri The URI string.
    /// \return A string containing the fully-qualified path to an SDF file.
    public: virtual std::string UriToFilename(const std::string &_uri);

    /// \brief Allow special processing of included files.
    /// \param[in,out] _sdf The SDF that will be included..
    /// \param[in] _elemXml Raw XML for the <include>.
    /// \return True if successful.
    public: bool ProcessInclude(SDFPtr _sdf, TiXmlElement *_elemXml);
  };
}
#endif
