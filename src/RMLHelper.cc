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
#include <boost/filesystem.hpp>
#include "RMLHelper.hh"

using namespace rml;

///////////////////////////////////////////////
std::string RMLHelper::GetRootName() const
{
  return "rml";
}

///////////////////////////////////////////////
std::string RMLHelper::UriToFilename(const std::string &_includePath)
{
  std::string filename;

  boost::filesystem::path manifestPath = _includePath;

  if (boost::filesystem::exists(manifestPath / "model.config"))
  {
    manifestPath /= "model.config";
  }
  else
  {
    sdfwarn << "The manifest.xml for a model is deprecated. "
      << "Please rename manifest.xml to "
      << "model.config" << ".\n";

    manifestPath /= "manifest.xml";
  }

  TiXmlDocument manifestDoc;
  if (manifestDoc.LoadFile(manifestPath.string()))
  {
    TiXmlElement *modelXML = manifestDoc.FirstChildElement("model");
    if (!modelXML)
      sdferr << "No <model> element in manifest["
        << manifestPath << "]\n";
    else
    {
      TiXmlElement *rmlXML = modelXML->FirstChildElement("rml");

      TiXmlElement *rmlSearch = rmlXML;

      // Find the SDF element that matches our current SDF version.
      while (rmlSearch)
      {
        if (rmlSearch->Attribute("version") &&
            std::string(rmlSearch->Attribute("version")) == SDF_VERSION)
        {
          rmlXML = rmlSearch;
          break;
        }

        rmlSearch = rmlSearch->NextSiblingElement("rml");
      }

      filename = _includePath + "/" + rmlXML->GetText();
    }
  }

  return filename;
}

///////////////////////////////////////////////
bool RMLHelper::ProcessInclude(sdf::SDFPtr _includeSDF,
                               TiXmlElement *_elemXml)
{
  if (_elemXml->FirstChildElement("name"))
  {
    _includeSDF->root->GetElement("model")->GetAttribute(
        "name")->SetFromString(
          _elemXml->FirstChildElement("name")->GetText());
  }

  if (_elemXml->FirstChildElement("pose"))
  {
    _includeSDF->root->GetElement("model")->GetElement(
        "pose")->GetValue()->SetFromString(
          _elemXml->FirstChildElement("pose")->GetText());
  }

  if (_elemXml->FirstChildElement("static"))
  {
    _includeSDF->root->GetElement("model")->GetElement(
        "static")->GetValue()->SetFromString(
          _elemXml->FirstChildElement("static")->GetText());
  }

  for (TiXmlElement *childElemXml = _elemXml->FirstChildElement();
      childElemXml; childElemXml = childElemXml->NextSiblingElement())
  {
    if (std::string("plugin") == childElemXml->Value())
    {
      sdf::ElementPtr pluginElem;
      pluginElem = _includeSDF->root->GetElement(
          "model")->AddElement("plugin");

      pluginElem->GetAttribute("filename")->SetFromString(
          childElemXml->Attribute("filename"));
      pluginElem->GetAttribute("name")->SetFromString(
          childElemXml->Attribute("name"));
    }
  }

  return true;
}
