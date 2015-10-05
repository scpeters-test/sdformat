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
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "sdf/Console.hh"
#include "sdf/Converter.hh"
#include "sdf/SDFImpl.hh"
#include "sdf/Param.hh"
#include "sdf/parser.hh"
#include "sdf/sdf_config.h"

#include "sdf/parser_urdf.hh"

namespace sdf
{
//////////////////////////////////////////////////
bool init(SDFPtr _sdf)
{
  bool result = false;

  std::string filename;
  std::string fileToFind = "root.sdf";

  if (sdf::SDF::Version() == "1.0" || sdf::SDF::Version() == "1.2")
    fileToFind = "gazebo.sdf";

  filename = sdf::findFile(fileToFind);

  FILE *ftest = fopen(filename.c_str(), "r");
  if (ftest)
  {
    fclose(ftest);
    if (initFile(filename, _sdf))
    {
      result = true;
    }
    else
    {
      sdferr << "Unable to init SDF file[" << filename << "]\n";
    }
  }
  else
  {
    sdferr << "Unable to find or open SDF file[" << fileToFind << "]\n";
  }

  return result;
}

//////////////////////////////////////////////////
template <typename TPtr>
inline bool _initFile(const std::string &_filename, TPtr _sdf)
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
bool initFile(const std::string &_filename, SDFPtr _sdf)
{
  return _initFile(_filename, _sdf);
}

//////////////////////////////////////////////////
bool initFile(const std::string &_filename, ElementPtr _sdf)
{
  return _initFile(_filename, _sdf);
}

//////////////////////////////////////////////////
bool initString(const std::string &_xmlString, SDFPtr _sdf)
{
  TiXmlDocument xmlDoc;
  xmlDoc.Parse(_xmlString.c_str());
  if (xmlDoc.Error())
  {
    sdferr << "Failed to parse string as XML: " << xmlDoc.ErrorDesc() << '\n';
    return false;
  }

  return initDoc(&xmlDoc, _sdf);
}

//////////////////////////////////////////////////
inline TiXmlElement *_initDocGetElement(TiXmlDocument *_xmlDoc)
{
  if (!_xmlDoc)
  {
    sdferr << "Could not parse the xml\n";
    return nullptr;
  }

  TiXmlElement *element = _xmlDoc->FirstChildElement("element");
  if (!element)
  {
    sdferr << "Could not find the 'element' element in the xml file\n";
    return nullptr;
  }

  return element;
}

//////////////////////////////////////////////////
bool initDoc(TiXmlDocument *_xmlDoc, SDFPtr _sdf)
{
  auto element = _initDocGetElement(_xmlDoc);
  if (!element)
  {
    return false;
  }

  return initXml(element, _sdf->Root());
}

//////////////////////////////////////////////////
bool initDoc(TiXmlDocument *_xmlDoc, ElementPtr _sdf)
{
  auto element = _initDocGetElement(_xmlDoc);
  if (!element)
  {
    return false;
  }

  return initXml(element, _sdf);
}

//////////////////////////////////////////////////
bool initXml(TiXmlElement *_xml, ElementPtr _sdf)
{
  const char *refString = _xml->Attribute("ref");
  if (refString)
    _sdf->SetReferenceSDF(std::string(refString));

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

  if (!xmlDoc.LoadFile(filename))
  {
    sdferr << "Error parsing XML in file [" << filename << "]: "
           << xmlDoc.ErrorDesc() << '\n';
    return false;
  }
  if (readDoc(&xmlDoc, _sdf, filename))
    return true;
  else
  {
    sdf::URDF2SDF u2g;
    TiXmlDocument doc = u2g.InitModelFile(filename);
    if (sdf::readDoc(&doc, _sdf, "urdf file"))
    {
      sdfdbg << "parse from urdf file [" << _filename << "].\n";
      return true;
    }
    else
    {
      sdferr << "parse as old deprecated model file failed.\n";
      return false;
    }
  }

  return false;
}

//////////////////////////////////////////////////
bool readString(const std::string &_xmlString, SDFPtr _sdf)
{
  TiXmlDocument xmlDoc;
  xmlDoc.Parse(_xmlString.c_str());
  if (xmlDoc.Error())
  {
    sdferr << "Error parsing XML from string: " << xmlDoc.ErrorDesc() << '\n';
    return false;
  }
  if (readDoc(&xmlDoc, _sdf, "data-string"))
    return true;
  else
  {
    sdf::URDF2SDF u2g;
    TiXmlDocument doc = u2g.InitModelString(_xmlString);
    if (sdf::readDoc(&doc, _sdf, "urdf string"))
    {
      sdfdbg << "Parsing from urdf.\n";
      return true;
    }
    else
    {
      sdferr << "parse as old deprecated model file failed.\n";
      return false;
    }
  }

  return false;
}

//////////////////////////////////////////////////
bool readString(const std::string &_xmlString, ElementPtr _sdf)
{
  TiXmlDocument xmlDoc;
  xmlDoc.Parse(_xmlString.c_str());
  if (xmlDoc.Error())
  {
    sdferr << "Error parsing XML from string: " << xmlDoc.ErrorDesc() << '\n';
    return false;
  }
  if (readDoc(&xmlDoc, _sdf, "data-string"))
    return true;
  else
  {
    sdferr << "parse as sdf version " << SDF::Version() << " failed, "
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
  TiXmlElement *sdfNode = _xmlDoc->FirstChildElement("sdf");
  if (!sdfNode)
    sdfNode = _xmlDoc->FirstChildElement("gazebo");

  if (sdfNode && sdfNode->Attribute("version"))
  {
    if (strcmp(sdfNode->Attribute("version"), SDF::Version().c_str()) != 0)
    {
      sdfdbg << "Converting a deprecated source[" << _source << "].\n";
      Converter::Convert(_xmlDoc, SDF::Version());
    }

    // parse new sdf xml
    TiXmlElement *elemXml = _xmlDoc->FirstChildElement(_sdf->Root()->GetName());
    if (!readXml(elemXml, _sdf->Root()))
    {
      sdferr << "Unable to read element <" << _sdf->Root()->GetName() << ">\n";
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
                    SDF::Version().c_str()) != 0)
      sdfdbg << "SDF version ["
            << sdfNode->Attribute("version")
            << "] is not " << SDF::Version() << "\n";
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
  TiXmlElement *sdfNode = _xmlDoc->FirstChildElement("sdf");
  if (!sdfNode)
    sdfNode = _xmlDoc->FirstChildElement("gazebo");

  if (sdfNode && sdfNode->Attribute("version"))
  {
    if (strcmp(sdfNode->Attribute("version"),
               SDF::Version().c_str()) != 0)
    {
      sdfwarn << "Converting a deprecated SDF source[" << _source << "].\n";
      Converter::Convert(_xmlDoc, SDF::Version());
    }

    TiXmlElement *elemXml = sdfNode;
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
                    SDF::Version().c_str()) != 0)
      sdfdbg << "SDF version ["
            << sdfNode->Attribute("version")
            << "] is not " << SDF::Version() << "\n";
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

  // check for nested sdf
  std::string refSDFStr = _sdf->ReferenceSDF();
  if (!refSDFStr.empty())
  {
    ElementPtr refSDF;
    refSDF.reset(new Element);
    std::string refFilename = refSDFStr + ".sdf";
    initFile(refFilename, refSDF);
    _sdf->RemoveFromParent();
    _sdf->Copy(refSDF);
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
        std::string modelPath;

        if (elemXml->FirstChildElement("uri"))
        {
          modelPath = sdf::findFile(
              elemXml->FirstChildElement("uri")->GetText(), true, true);

          // Test the model path
          if (modelPath.empty())
          {
            sdferr << "Unable to find uri["
              << elemXml->FirstChildElement("uri")->GetText() << "]\n";

            std::string uri = elemXml->FirstChildElement("uri")->GetText();
            size_t modelFound = uri.find("model://");
            if ( modelFound != 0u)
            {
              sdferr << "Invalid uri[" << uri << "]. Should be model://"
                    << uri << "\n";
            }
            continue;
          }
          else
          {
            boost::filesystem::path dir(modelPath);
            if (!boost::filesystem::exists(dir) ||
                !boost::filesystem::is_directory(dir))
            {
              sdferr << "Directory doesn't exist[" << modelPath << "]\n";
              continue;
            }
          }

          boost::filesystem::path manifestPath = modelPath;

          /// \todo This hardcoded bit is very Gazebo centric. It should
          /// be abstracted away, possible through a plugin to SDF.
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
              TiXmlElement *sdfXML = modelXML->FirstChildElement("sdf");

              TiXmlElement *sdfSearch = sdfXML;

              // Find the SDF element that matches our current SDF version.
              while (sdfSearch)
              {
                if (sdfSearch->Attribute("version") &&
                    std::string(sdfSearch->Attribute("version")) == SDF_VERSION)
                {
                  sdfXML = sdfSearch;
                  break;
                }

                sdfSearch = sdfSearch->NextSiblingElement("sdf");
              }

              filename = modelPath + "/" + sdfXML->GetText();
            }
          }
          else
          {
            sdferr << "Error parsing XML in file ["
                   << manifestPath.string() << "]: "
                   << manifestDoc.ErrorDesc() << '\n';
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

        // NOTE: sdf::init is an expensive call. For performance reason,
        // a new sdf pointer is created here by cloning a fresh sdf template
        // pointer instead of calling init every iteration.
        // SDFPtr includeSDF(new SDF);
        // init(includeSDF);
        static SDFPtr includeSDFTemplate;
        if (!includeSDFTemplate)
        {
          includeSDFTemplate.reset(new SDF);
          init(includeSDFTemplate);
        }
        SDFPtr includeSDF(new SDF);
        includeSDF->Root(includeSDFTemplate->Root()->Clone());

        if (!readFile(filename, includeSDF))
        {
          sdferr << "Unable to read file[" << filename << "]\n";
          return false;
        }

        if (elemXml->FirstChildElement("name"))
        {
          includeSDF->Root()->GetElement("model")->GetAttribute(
              "name")->SetFromString(
                elemXml->FirstChildElement("name")->GetText());
        }

        if (elemXml->FirstChildElement("pose"))
        {
          includeSDF->Root()->GetElement("model")->GetElement(
              "pose")->GetValue()->SetFromString(
                elemXml->FirstChildElement("pose")->GetText());
        }

        if (elemXml->FirstChildElement("static"))
        {
          includeSDF->Root()->GetElement("model")->GetElement(
              "static")->GetValue()->SetFromString(
                elemXml->FirstChildElement("static")->GetText());
        }

        for (TiXmlElement *childElemXml = elemXml->FirstChildElement();
             childElemXml; childElemXml = childElemXml->NextSiblingElement())
        {
          if (std::string("plugin") == childElemXml->Value())
          {
            sdf::ElementPtr pluginElem;
            pluginElem = includeSDF->Root()->GetElement(
                "model")->AddElement("plugin");

            pluginElem->GetAttribute("filename")->SetFromString(
                childElemXml->Attribute("filename"));
            pluginElem->GetAttribute("name")->SetFromString(
                childElemXml->Attribute("name"));
          }
        }

        if (_sdf->GetName() == "model")
        {
          addNestedModel(_sdf, includeSDF->Root());
        }
        else
        {
          includeSDF->Root()->GetFirstElement()->SetParent(_sdf);
          _sdf->InsertElement(includeSDF->Root()->GetFirstElement());
          // TODO: This was used to store the included filename so that when
          // a world is saved, the included model's SDF is not stored in the
          // world file. This highlights the need to make model inclusion
          // a core feature of SDF, and not a hack that that parser handles
          // includeSDF->Root()->GetFirstElement()->SetInclude(
          // elemXml->Attribute("filename"));
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
        sdfwarn << "XML Element[" << elemXml->Value()
          << "], child of element[" << _xml->Value()
          << "] not defined in SDF. Ignoring[" << elemXml->Value() << "]. "
          << "You may have an incorrect SDF file, or an sdformat version "
          << "that doesn't support this element.\n";
        continue;
      }
    }

    // Check that all required elements have been set
    unsigned int descCounter = 0;
    for (descCounter = 0;
         descCounter != _sdf->GetElementDescriptionCount(); ++descCounter)
    {
      ElementPtr elemDesc = _sdf->GetElementDescription(descCounter);

      if (elemDesc->GetRequired() == "1" || elemDesc->GetRequired() == "+")
      {
        if (!_sdf->HasElement(elemDesc->GetName()))
        {
          if (_sdf->GetName() == "joint" &&
              _sdf->Get<std::string>("type") != "ball")
          {
            sdferr << "XML Missing required element[" << elemDesc->GetName()
              << "], child of element[" << _sdf->GetName() << "]\n";
            return false;
          }
          else
          {
            // Add default element
            _sdf->AddElement(elemDesc->GetName());
          }
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
  TiXmlElement *elemXml = NULL;
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

      for (TiXmlAttribute *attribute = elemXml->FirstAttribute();
           attribute; attribute = attribute->Next())
      {
        element->AddAttribute(attribute->Name(), "string", "", 1, "");
        element->GetAttribute(attribute->Name())->SetFromString(
          attribute->ValueStr());
      }

      copyChildren(element, elemXml);
      _sdf->InsertElement(element);
    }
  }
}

/////////////////////////////////////////////////
void addNestedModel(ElementPtr _sdf, ElementPtr _includeSDF)
{
  ElementPtr modelPtr = _includeSDF->GetElement("model");
  ElementPtr elem = modelPtr->GetFirstElement();
  std::map<std::string, std::string> replace;

  ignition::math::Pose3d modelPose =
    modelPtr->Get<ignition::math::Pose3d>("pose");

  std::string modelName = modelPtr->Get<std::string>("name");
  while (elem)
  {
    if (elem->GetName() == "link")
    {
      std::string elemName = elem->Get<std::string>("name");
      std::string newName =  modelName + "::" + elemName;
      replace[elemName] = newName;
      if (elem->HasElementDescription("pose"))
      {
        ignition::math::Pose3d offsetPose =
          elem->Get<ignition::math::Pose3d>("pose");
        ignition::math::Pose3d newPose = ignition::math::Pose3d(
          modelPose.Pos() +
            modelPose.Rot().RotateVector(offsetPose.Pos()),
            modelPose.Rot() * offsetPose.Rot());
        elem->GetElement("pose")->Set(newPose);
      }
    }
    else if (elem->GetName() == "joint")
    {
      // for joints, we need to
      //   prefix name like we did with links, and
      std::string elemName = elem->Get<std::string>("name");
      std::string newName =  modelName + "::" + elemName;
      replace[elemName] = newName;
      //   rotate the joint axis because they are model-global
      if (elem->HasElement("axis"))
      {
        ElementPtr axisElem = elem->GetElement("axis");
        ignition::math::Vector3d newAxis =  modelPose.Rot().RotateVector(
          axisElem->Get<ignition::math::Vector3d>("xyz"));
        axisElem->GetElement("xyz")->Set(newAxis);
      }
    }
    elem = elem->GetNextElement();
  }

  std::string str = _includeSDF->ToString("");
  for (std::map<std::string, std::string>::iterator iter = replace.begin();
       iter != replace.end(); ++iter)
  {
    boost::replace_all(str, std::string("\"")+iter->first + "\"",
                       std::string("\"") + iter->second + "\"");
    boost::replace_all(str, std::string("'")+iter->first + "'",
                       std::string("'") + iter->second + "'");
    boost::replace_all(str, std::string(">")+iter->first + "<",
                       std::string(">") + iter->second + "<");
  }

  _includeSDF->ClearElements();
  readString(str, _includeSDF);

  elem = _includeSDF->GetElement("model")->GetFirstElement();
  ElementPtr nextElem;
  while (elem)
  {
    nextElem = elem->GetNextElement();

    if (elem->GetName() != "pose")
    {
      elem->SetParent(_sdf);
      _sdf->InsertElement(elem);
    }
    elem = nextElem;
  }
}
}
