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
#include <stdlib.h>
#include <stdio.h>
#include <map>

#include <boost/filesystem.hpp>

#include "sdf/Console.hh"
#include "sdf/Converter.hh"
#include "sdf/SDFImpl.hh"
#include "sdf/Param.hh"
#include "sdf/parser.hh"
#include "sdf/sdf_config.h"

namespace sdf
{
typedef std::list<boost::filesystem::path> PathList;
typedef std::map<std::string, PathList> URIPathMap;

URIPathMap g_uriPathMap;
SDFHelper g_sdfHelper;

/////////////////////////////////////////////////
std::string findFile(const std::string &_filename, bool _searchLocalPath,
    bool _useCallback)
{
  boost::filesystem::path path = _filename;

  // Check to see if _filename is URI. If so, resolve the URI path.
  for (URIPathMap::iterator iter = g_uriPathMap.begin();
       iter != g_uriPathMap.end(); ++iter)
  {
    // Check to see if the URI in the global map is the first part of the
    // given filename
    if (_filename.find(iter->first) == 0)
    {
      std::string suffix = _filename;
      boost::replace_first(suffix, iter->first, "");

      // Check each path in the list.
      for (PathList::iterator pathIter = iter->second.begin();
           pathIter != iter->second.end(); ++pathIter)
      {
        // Return the path string if the path + suffix exists.
        if (boost::filesystem::exists((*pathIter) / suffix))
          return ((*pathIter) / suffix).string();
      }
    }
  }

  // Next check the install path.
  path = boost::filesystem::path(SDF_SHARE_PATH) / _filename;
  if (boost::filesystem::exists(path))
    return path.string();

  // Next check the versioned install path.
  path = boost::filesystem::path(SDF_VERSION_PATH) / _filename;
  if (boost::filesystem::exists(path))
    return path.string();

  // Next check SDF_PATH environment variable
  char *pathCStr = getenv("SDF_PATH");
  if (pathCStr)
  {
    std::vector<std::string> paths;
    boost::split(paths, pathCStr, boost::is_any_of(":"));
    for (std::vector<std::string>::iterator iter = paths.begin();
         iter != paths.end(); ++iter)
    {
      path = boost::filesystem::path(*iter) / _filename;
      if (boost::filesystem::exists(path))
        return path.string();
    }
  }

  // Next check to see if the given file exists.
  path = boost::filesystem::path(_filename);
  if (boost::filesystem::exists(path))
    return path.string();


  // Finally check the local path, if the flag is set.
  if (_searchLocalPath)
  {
    path = boost::filesystem::current_path() / _filename;

    if (boost::filesystem::exists(path))
      return path.string();
  }

  // If we still haven't found the file, use the registered callback if the
  // flag has been set
  if (_useCallback)
  {
    return g_sdfHelper.FindFile(_filename);
  }

  return std::string();
}

/////////////////////////////////////////////////
void addURIPath(const std::string &_uri, const std::string &_path)
{
  // Split _path on colons.
  std::list<std::string> parts;
  boost::split(parts, _path, boost::is_any_of(":"));

  // Add each part of the colon separated path to the global URI map.
  for (std::list<std::string>::iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    boost::filesystem::path path = *iter;

    // Only add valid paths
    if (!(*iter).empty() && boost::filesystem::exists(path) &&
        boost::filesystem::is_directory(path))
    {
      g_uriPathMap[_uri].push_back(path);
    }
  }
}

/////////////////////////////////////////////////
void setHelper(const SDFHelper &_helper)
{
  g_sdfHelper = _helper;
}

//////////////////////////////////////////////////
bool init(SDFPtr _sdf)
{
  bool result = false;

  std::string filename;
  filename = sdf::findFile("root.sdf");

  FILE *ftest = fopen(filename.c_str(), "r");
  if (ftest && initFile(filename, _sdf))
  {
    result = true;
    fclose(ftest);
  }
  else
    sdferr << "Unable to find or open SDF file[" << filename << "]\n";

  return result;
}

//////////////////////////////////////////////////
bool initFile(const std::string &_filename, SDFPtr _sdf)
{
  std::string filename = sdf::findFile(_filename);

  TiXmlDocument xmlDoc;
  if (xmlDoc.LoadFile(filename))
  {
    return initDoc(&xmlDoc, _sdf);
  }
  else
    sdferr << "Unable to load file[" << _filename << "]\n";

  return false;
}

//////////////////////////////////////////////////
bool initFile(const std::string &_filename, ElementPtr _sdf)
{
  std::string filename = sdf::findFile(_filename);

  TiXmlDocument xmlDoc;
  if (xmlDoc.LoadFile(filename))
    return initDoc(&xmlDoc, _sdf);
  else
    sdferr << "Unable to load file[" << _filename << "]\n";

  return false;
}

//////////////////////////////////////////////////
bool initString(const std::string &_xmlString, SDFPtr _sdf)
{
  TiXmlDocument xmlDoc;
  xmlDoc.Parse(_xmlString.c_str());

  return initDoc(&xmlDoc, _sdf);
}

//////////////////////////////////////////////////
bool initDoc(TiXmlDocument *_xmlDoc, SDFPtr _sdf)
{
  if (!_xmlDoc)
  {
    sdferr << "Could not parse the xml\n";
    return false;
  }

  TiXmlElement *xml = _xmlDoc->FirstChildElement("element");
  if (!xml)
  {
    sdferr << "Could not find the 'element' element in the xml file\n";
    return false;
  }

  return initXml(xml, _sdf->root);
}

//////////////////////////////////////////////////
bool initDoc(TiXmlDocument *_xmlDoc, ElementPtr _sdf)
{
  if (!_xmlDoc)
  {
    sdferr << "Could not parse the xml\n";
    return false;
  }

  TiXmlElement *xml = _xmlDoc->FirstChildElement("element");
  if (!xml)
  {
    sdferr << "Could not find the 'element' element in the xml file\n";
    return false;
  }

  return initXml(xml, _sdf);
}

//////////////////////////////////////////////////
bool initXml(TiXmlElement *_xml, ElementPtr _sdf)
{
  const char *nameString = _xml->Attribute("name");
  if (!nameString)
  {
    sdferr << "Element is missing the name attribute\n";
    return false;
  }
  _sdf->SetName(std::string(nameString));

  const char *requiredString = _xml->Attribute("required");
  if (!requiredString)
  {
    sdferr << "Element is missing the required attributed\n";
    return false;
  }
  _sdf->SetRequired(requiredString);

  const char *elemTypeString = _xml->Attribute("type");
  if (elemTypeString)
  {
    bool required = std::string(requiredString) == "1" ? true : false;
    const char *elemDefaultValue = _xml->Attribute("default");
    std::string description;
    TiXmlElement *descChild = _xml->FirstChildElement("description");
    if (descChild && descChild->GetText())
      description = descChild->GetText();

    _sdf->AddValue(elemTypeString, elemDefaultValue, required, description);
  }

  // Get all attributes
  for (TiXmlElement *child = _xml->FirstChildElement("attribute");
      child; child = child->NextSiblingElement("attribute"))
  {
    TiXmlElement *descriptionChild = child->FirstChildElement("description");
    const char *name = child->Attribute("name");
    const char *type = child->Attribute("type");
    const char *defaultValue = child->Attribute("default");

    requiredString = child->Attribute("required");

    if (!name)
    {
      sdferr << "Attribute is missing a name\n";
      return false;
    }
    if (!type)
    {
      sdferr << "Attribute is missing a type\n";
      return false;
    }
    if (!defaultValue)
    {
      sdferr << "Attribute[" << name << "] is missing a default\n";
      return false;
    }
    if (!requiredString)
    {
      sdferr << "Attribute is missing a required string\n";
      return false;
    }
    std::string requiredStr = requiredString;
    boost::trim(requiredStr);
    bool required = requiredStr == "1" ? true : false;
    std::string description;

    if (descriptionChild && descriptionChild->GetText())
      description = descriptionChild->GetText();

    _sdf->AddAttribute(name, type, defaultValue, required, description);
  }

  // Read the element description
  TiXmlElement *descChild = _xml->FirstChildElement("description");
  if (descChild && descChild->GetText())
  {
    _sdf->SetDescription(descChild->GetText());
  }

  // Get all child elements
  for (TiXmlElement *child = _xml->FirstChildElement("element");
      child; child = child->NextSiblingElement("element"))
  {
    const char *copyDataString = child->Attribute("copy_data");
    if (copyDataString &&
        (std::string(copyDataString) == "true" ||
         std::string(copyDataString) == "1"))
    {
      _sdf->SetCopyChildren(true);
    }
    else
    {
      ElementPtr element(new Element);
      initXml(child, element);
      _sdf->AddElementDescription(element);
    }
  }

  // Get all include elements
  for (TiXmlElement *child = _xml->FirstChildElement("include");
      child; child = child->NextSiblingElement("include"))
  {
    std::string filename = child->Attribute("filename");

    ElementPtr element(new Element);

    initFile(filename, element);
    _sdf->AddElementDescription(element);
  }

  return true;
}


//////////////////////////////////////////////////
bool readFile(const std::string &_filename, SDFPtr _sdf)
{
  TiXmlDocument xmlDoc;
  std::string filename = sdf::findFile(_filename);

  if (filename.empty())
  {
    sdferr << "Error finding file [" << _filename << "].\n";
    return false;
  }

  xmlDoc.LoadFile(filename);
  if (readDoc(&xmlDoc, _sdf, filename))
    return true;
  else
    sdferr << "Failed to parse SDF file.\n";

  return false;
}

//////////////////////////////////////////////////
bool readString(const std::string &_xmlString, SDFPtr _sdf)
{
  TiXmlDocument xmlDoc;
  xmlDoc.Parse(_xmlString.c_str());
  if (readDoc(&xmlDoc, _sdf, "data-string"))
    return true;
  else
    sdferr << "Failed to parse SDF file.\n";

  return false;
}

//////////////////////////////////////////////////
bool readString(const std::string &_xmlString, ElementPtr _sdf)
{
  TiXmlDocument xmlDoc;
  xmlDoc.Parse(_xmlString.c_str());
  if (readDoc(&xmlDoc, _sdf, "data-string"))
    return true;
  else
  {
    sdferr << "parse as sdf version " << SDF::version << " failed, "
          << "should try to parse as old deprecated format\n";
    return false;
  }
}

//////////////////////////////////////////////////
bool readDoc(TiXmlDocument *_xmlDoc, SDFPtr _sdf, const std::string &_source)
{
  if (!_xmlDoc)
  {
    sdfwarn << "Could not parse the xml from source[" << _source << "]\n";
    return false;
  }

  // check sdf version, use old parser if necessary
  TiXmlElement *sdfNode = _xmlDoc->FirstChildElement(g_sdfHelper.GetRootName());

  if (sdfNode && sdfNode->Attribute("version"))
  {
    if (strcmp(sdfNode->Attribute("version"), SDF::version.c_str()) != 0)
    {
      sdfwarn << "Converting a deprecated source[" << _source << "].\n";
      Converter::Convert(_xmlDoc, SDF::version, g_sdfHelper.GetRootName());
    }

    // parse new sdf xml
    TiXmlElement *elemXml = _xmlDoc->FirstChildElement(_sdf->root->GetName());
    if (!readXml(elemXml, _sdf->root))
    {
      sdferr << "Unable to read element <" << _sdf->root->GetName() << ">\n";
      return false;
    }
  }
  else
  {
    // try to use the old deprecated parser
    if (!sdfNode)
      sdfdbg << "No <sdf> element in file[" << _source << "]\n";
    else if (!sdfNode->Attribute("version"))
      sdfdbg << "SDF <sdf> element has no version in file["
             << _source << "]\n";
    else if (strcmp(sdfNode->Attribute("version"),
                    SDF::version.c_str()) != 0)
      sdfdbg << "SDF version ["
            << sdfNode->Attribute("version")
            << "] is not " << SDF::version << "\n";
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
bool readDoc(TiXmlDocument *_xmlDoc, ElementPtr _sdf,
             const std::string &_source)
{
  if (!_xmlDoc)
  {
    sdfwarn << "Could not parse the xml\n";
    return false;
  }

  // check sdf version, use old parser if necessary
  TiXmlElement *sdfNode = _xmlDoc->FirstChildElement(g_sdfHelper.GetRootName());

  if (sdfNode && sdfNode->Attribute("version"))
  {
    if (strcmp(sdfNode->Attribute("version"),
               SDF::version.c_str()) != 0)
    {
      sdfwarn << "Converting a deprecated SDF source[" << _source << "].\n";
      Converter::Convert(_xmlDoc, SDF::version, g_sdfHelper.GetRootName());
    }

    TiXmlElement* elemXml = sdfNode;
    if (sdfNode->Value() != _sdf->GetName() &&
        sdfNode->FirstChildElement(_sdf->GetName()))
    {
      elemXml = sdfNode->FirstChildElement(_sdf->GetName());
    }

    /* parse new sdf xml */
    if (!readXml(elemXml, _sdf))
    {
      sdfwarn << "Unable to parse sdf element["
             << _sdf->GetName() << "]\n";
      return false;
    }
  }
  else
  {
    // try to use the old deprecated parser
    if (!sdfNode)
      sdfdbg << "SDF has no <sdf> element\n";
    else if (!sdfNode->Attribute("version"))
      sdfdbg << "<sdf> element has no version\n";
    else if (strcmp(sdfNode->Attribute("version"),
                    SDF::version.c_str()) != 0)
      sdfdbg << "SDF version ["
            << sdfNode->Attribute("version")
            << "] is not " << SDF::version << "\n";
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
bool readXml(TiXmlElement *_xml, ElementPtr _sdf)
{
  if (_sdf->GetRequired() == "-1")
  {
    sdfwarn << "SDF Element[" << _sdf->GetName() << "] is deprecated\n";
    return true;
  }

  if (!_xml)
  {
    if (_sdf->GetRequired() == "1" || _sdf->GetRequired() =="+")
    {
      sdferr << "SDF Element<" << _sdf->GetName() << "> is missing\n";
      return false;
    }
    else
      return true;
  }

  if (_xml->GetText() != NULL && _sdf->GetValue())
  {
    _sdf->GetValue()->SetFromString(_xml->GetText());
  }

  TiXmlAttribute *attribute = _xml->FirstAttribute();

  unsigned int i = 0;

  // Iterate over all the attributes defined in the give XML element
  while (attribute)
  {
    // Find the matching attribute in SDF
    for (i = 0; i < _sdf->GetAttributeCount(); ++i)
    {
      ParamPtr p = _sdf->GetAttribute(i);
      if (p->GetKey() == attribute->Name())
      {
        // Set the value of the SDF attribute
        if (!p->SetFromString(attribute->ValueStr()))
        {
          sdferr << "Unable to read attribute[" << p->GetKey() << "]\n";
          return false;
        }
        break;
      }
    }

    if (i == _sdf->GetAttributeCount())
    {
      sdfwarn << "XML Attribute[" << attribute->Name()
             << "] in element[" << _xml->Value()
             << "] not defined in SDF, ignoring.\n";
    }

    attribute = attribute->Next();
  }

  // Check that all required attributes have been set
  for (i = 0; i < _sdf->GetAttributeCount(); ++i)
  {
    ParamPtr p = _sdf->GetAttribute(i);
    if (p->GetRequired() && !p->GetSet())
    {
      sdferr << "Required attribute[" << p->GetKey()
        << "] in element[" << _xml->Value() << "] is not specified in SDF.\n";
      return false;
    }
  }

  if (_sdf->GetCopyChildren())
  {
    copyChildren(_sdf, _xml);
  }
  else
  {
    std::string filename;

    // Iterate over all the child elements
    TiXmlElement *elemXml = NULL;
    for (elemXml = _xml->FirstChildElement(); elemXml;
         elemXml = elemXml->NextSiblingElement())
    {
      if (std::string("include") == elemXml->Value())
      {
        std::string includePath;

        if (elemXml->FirstChildElement("uri"))
        {
          includePath = sdf::findFile(
              elemXml->FirstChildElement("uri")->GetText(), true, true);

          // Test the include path
          if (includePath.empty())
          {
            sdferr << "Unable to find uri["
              << elemXml->FirstChildElement("uri")->GetText() << "]\n";
            continue;
          }
          else
          {
            boost::filesystem::path dir(includePath);
            if (!boost::filesystem::exists(dir) ||
                !boost::filesystem::is_directory(dir))
            {
              sdferr << "Directory doesn't exist[" << includePath << "]\n";
              continue;
            }
          }

          // Allow special processing of URI paths.
          filename = g_sdfHelper.UriToFilename(includePath);
          if (filename.empty())
          {
            sdferr << "Unable to process <include> with URI["
                   << includePath << "]\n";
            continue;
          }
        }
        else
        {
          if (elemXml->Attribute("filename"))
          {
            sdferr << "<include filename='...'/> is deprecated. Should be "
                  << "<include><uri>...</uri></include\n";

            filename = sdf::findFile(elemXml->Attribute("filename"), false);
          }
          else
          {
            sdferr << "<include> element missing 'uri' attribute\n";
            continue;
          }
        }

        SDFPtr includeSDF(new SDF);
        init(includeSDF);

        if (!readFile(filename, includeSDF))
        {
          sdferr << "Unable to read file[" << filename << "]\n";
          return false;
        }

        // Allow special processing of includes
        if (g_sdfHelper.ProcessInclude(includeSDF, elemXml))
        {
          includeSDF->root->GetFirstElement()->SetParent(_sdf);
          _sdf->InsertElement(includeSDF->root->GetFirstElement());
          // TODO: This was used to store the included filename so that when
          // an SDF is saved, the included SDF is not stored in the
          // main SDF file. This highlights the need to make SDF inclusion
          // a core feature of SDF, and not a hack that that parser handles
          // includeSDF->root->GetFirstElement()->SetInclude(elemXml->Attribute(
          //      "filename"));
        }

        continue;
      }

      // Find the matching element in SDF
      unsigned int descCounter = 0;
      for (descCounter = 0;
           descCounter != _sdf->GetElementDescriptionCount(); ++descCounter)
      {
        ElementPtr elemDesc = _sdf->GetElementDescription(descCounter);
        if (elemDesc->GetName() == elemXml->Value())
        {
          ElementPtr element = elemDesc->Clone();
          element->SetParent(_sdf);
          if (readXml(elemXml, element))
            _sdf->InsertElement(element);
          else
          {
            sdferr << "Error reading element <" << elemXml->Value() << ">\n";
            return false;
          }
          break;
        }
      }

      if (descCounter == _sdf->GetElementDescriptionCount())
      {
        sdferr << "XML Element[" << elemXml->Value()
          << "], child of element[" << _xml->Value()
          << "] not defined in SDF. Ignoring.[" << _sdf->GetName() << "]\n";
        return false;
      }
    }

    // Chek that all required elements have been set
    unsigned int descCounter = 0;
    for (descCounter = 0;
         descCounter != _sdf->GetElementDescriptionCount(); ++descCounter)
    {
      ElementPtr elemDesc = _sdf->GetElementDescription(descCounter);

      if (elemDesc->GetRequired() == "1" || elemDesc->GetRequired() == "+")
      {
        if (!_sdf->HasElement(elemDesc->GetName()))
        {
          sdferr << "XML Missing required element[" << elemDesc->GetName()
            << "], child of element[" << _sdf->GetName() << "]\n";
          return false;
        }
      }
    }
  }

  return true;
}

/////////////////////////////////////////////////
void copyChildren(ElementPtr _sdf, TiXmlElement *_xml)
{
  // Iterate over all the child elements
  TiXmlElement* elemXml = NULL;
  for (elemXml = _xml->FirstChildElement(); elemXml;
       elemXml = elemXml->NextSiblingElement())
  {
    std::string elem_name = elemXml->ValueStr();

    if (_sdf->HasElementDescription(elem_name))
    {
      sdf::ElementPtr element = _sdf->AddElement(elem_name);

      // FIXME: copy attributes
      for (TiXmlAttribute *attribute = elemXml->FirstAttribute();
           attribute; attribute = attribute->Next())
      {
        element->GetAttribute(attribute->Name())->SetFromString(
          attribute->ValueStr());
      }

      // copy value
      std::string value = elemXml->GetText();
      if (!value.empty())
          element->GetValue()->SetFromString(value);
      copyChildren(element, elemXml);
    }
    else
    {
      ElementPtr element(new Element);
      element->SetParent(_sdf);
      element->SetName(elem_name);
      if (elemXml->GetText() != NULL)
        element->AddValue("string", elemXml->GetText(), "1");

      copyChildren(element, elemXml);
      _sdf->InsertElement(element);
    }
  }
}
}
