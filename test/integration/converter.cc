/*
 * Copyright 2015 Open Source Robotics Foundation
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

#include <array>
#include <string>

#include <gtest/gtest.h>
#include <tinyxml.h>

#include "sdf/sdf.hh"
#include "sdf/Converter.hh"

#include "test_config.h"

/// \brief Use different sdf versions for ParserStringConverter Test.
void ParserStringConverter(const std::string &_version);

const std::string CONVERT_DOC_15_16 =
  sdf::filesystem::append(PROJECT_SOURCE_PATH, "sdf", "1.6", "1_5.convert");
const std::string CONVERT_DOC_16_17 =
  sdf::filesystem::append(PROJECT_SOURCE_PATH, "sdf", "1.7", "1_6.convert");

/////////////////////////////////////////////////
/// Test conversion of imu in 1.5 to 1.6
TEST(ConverterIntegration, IMU_15_to_16)
{
  // The imu noise in 1.5 format
  std::string xmlString = R"(
<?xml version="1.0" ?>
<sdf version="1.5">
  <world name="default">
    <model name="box_old_imu_noise">
      <link name="link">
        <sensor name='imu_sensor' type='imu'>
          <imu>
            <noise>
              <type>gaussian</type>
              <rate>
                <mean>0</mean>
                <stddev>0.0002</stddev>
                <bias_mean>7.5e-06</bias_mean>
                <bias_stddev>8e-07</bias_stddev>
              </rate>
              <accel>
                <mean>0</mean>
                <stddev>0.017</stddev>
                <bias_mean>0.1</bias_mean>
                <bias_stddev>0.001</bias_stddev>
              </accel>
            </noise>
          </imu>
        </sensor>
      </link>
    </model>
  </world>
</sdf>)";

  TiXmlDocument xmlDoc;
  xmlDoc.Parse(xmlString.c_str());

  // Convert
  TiXmlDocument convertXmlDoc;
  convertXmlDoc.LoadFile(CONVERT_DOC_15_16);
  sdf::Converter::Convert(&xmlDoc, &convertXmlDoc);

  // Check some basic elements
  TiXmlElement *convertedElem =  xmlDoc.FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "sdf");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "world");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "model");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "link");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "sensor");

  // Get the imu
  TiXmlElement *imuElem = convertedElem->FirstChildElement();
  EXPECT_EQ(imuElem->ValueStr(), "imu");

  // Get the angular_velocity
  TiXmlElement *angVelElem = imuElem->FirstChildElement();
  EXPECT_EQ(angVelElem->ValueStr(), "angular_velocity");

  // Get the linear_acceleration
  TiXmlElement *linAccElem = angVelElem->NextSiblingElement();
  EXPECT_EQ(linAccElem->ValueStr(), "linear_acceleration");

  std::array<char, 3> axis = {'x', 'y', 'z'};

  TiXmlElement *angVelAxisElem = angVelElem->FirstChildElement();
  TiXmlElement *linAccAxisElem = linAccElem->FirstChildElement();

  // Iterate over <x>, <y>, and <z> elements under <angular_velocity> and
  // <linear_acceleration>
  for (auto const &a : axis)
  {
    EXPECT_EQ(angVelAxisElem->Value()[0], a);
    EXPECT_EQ(linAccAxisElem->Value()[0], a);

    TiXmlElement *angVelAxisNoiseElem = angVelAxisElem->FirstChildElement();
    TiXmlElement *linAccAxisNoiseElem = linAccAxisElem->FirstChildElement();

    EXPECT_EQ(angVelAxisNoiseElem->ValueStr(), "noise");
    EXPECT_EQ(linAccAxisNoiseElem->ValueStr(), "noise");

    EXPECT_STREQ(angVelAxisNoiseElem->Attribute("type"), "gaussian");
    EXPECT_STREQ(linAccAxisNoiseElem->Attribute("type"), "gaussian");

    EXPECT_STREQ(angVelAxisNoiseElem->FirstChildElement("mean")->GetText(),
                 "0");
    EXPECT_STREQ(linAccAxisNoiseElem->FirstChildElement("mean")->GetText(),
                 "0");

    EXPECT_STREQ(angVelAxisNoiseElem->FirstChildElement("stddev")->GetText(),
                 "0.0002");
    EXPECT_STREQ(linAccAxisNoiseElem->FirstChildElement("stddev")->GetText(),
                 "0.017");

    EXPECT_STREQ(angVelAxisNoiseElem->FirstChildElement("bias_mean")->GetText(),
                 "7.5e-06");
    EXPECT_STREQ(linAccAxisNoiseElem->FirstChildElement("bias_mean")->GetText(),
                 "0.1");

    EXPECT_STREQ(angVelAxisNoiseElem->FirstChildElement(
          "bias_stddev")->GetText(), "8e-07");
    EXPECT_STREQ(linAccAxisNoiseElem->FirstChildElement(
          "bias_stddev")->GetText(), "0.001");

    angVelAxisElem = angVelAxisElem->NextSiblingElement();
    linAccAxisElem = linAccAxisElem->NextSiblingElement();
  }
}

/////////////////////////////////////////////////
/// Test conversion using the parser sdf file converter interface.
TEST(ConverterIntegration, ParserFileConverter)
{
  std::string filename = sdf::filesystem::append(PROJECT_SOURCE_PATH, "test",
                                                 "integration", "audio.sdf");

  sdf::SDFPtr sdf(new sdf::SDF());
  sdf::init(sdf);

  EXPECT_TRUE(sdf::convertFile(filename, "1.6", sdf));

  sdf::ElementPtr rootElem = sdf->Root();
  ASSERT_NE(nullptr, rootElem);
  EXPECT_EQ("1.6", rootElem->Get<std::string>("version"));
  EXPECT_EQ("1.4", sdf->OriginalVersion());
  EXPECT_EQ("1.4", rootElem->OriginalVersion());

  sdf::ElementPtr modelElem = rootElem->GetElement("model");
  ASSERT_NE(nullptr, modelElem);
  EXPECT_EQ(modelElem->Get<std::string>("name"), "full_audio_parameters");
  EXPECT_EQ("1.4", modelElem->OriginalVersion());

  sdf::ElementPtr linkElem = modelElem->GetElement("link");
  ASSERT_NE(nullptr, linkElem);
  EXPECT_EQ(linkElem->Get<std::string>("name"), "link");
  EXPECT_EQ("1.4", linkElem->OriginalVersion());

  sdf::ElementPtr collElem = linkElem->GetElement("collision");
  ASSERT_NE(nullptr, collElem);
  EXPECT_EQ(collElem->Get<std::string>("name"), "collision");
  EXPECT_EQ("1.4", collElem->OriginalVersion());

  sdf::ElementPtr sinkElem = linkElem->GetElement("audio_sink");
  ASSERT_NE(nullptr, sinkElem);
  EXPECT_EQ("1.4", sinkElem->OriginalVersion());

  sdf::ElementPtr sourceElem = linkElem->GetElement("audio_source");
  ASSERT_NE(nullptr, sourceElem);
  EXPECT_EQ("1.4", sourceElem->OriginalVersion());
}

/////////////////////////////////////////////////
/// Convert to a previous SDF version
TEST(ConverterIntegration, convertFileToNotLatestVersion)
{
  std::string filename = sdf::filesystem::append(PROJECT_SOURCE_PATH, "test",
                                                 "integration", "audio.sdf");

  sdf::SDFPtr sdf(new sdf::SDF());
  sdf::init(sdf);

  EXPECT_TRUE(sdf::convertFile(filename, "1.5", sdf));

  sdf::ElementPtr rootElem = sdf->Root();
  ASSERT_NE(nullptr, rootElem);
  EXPECT_EQ("1.5", rootElem->Get<std::string>("version"));
  EXPECT_EQ("1.4", sdf->OriginalVersion());
  EXPECT_EQ("1.4", rootElem->OriginalVersion());
}

/////////////////////////////////////////////////
/// Test conversion using the parser sdf string converter interface.
TEST(ConverterIntegration, ParserStringConverter)
{
  ParserStringConverter("1.5");
}

TEST(ConverterIntegration, ParserStringConverterFrom14)
{
  ParserStringConverter("1.4");
}

void ParserStringConverter(const std::string &_version)
{
  // The gravity and magnetic_field in 1.5 format
  std::string xmlString = R"(
<?xml version="1.0" ?>
<sdf version=")" + _version + R"(">
  <world name="default">
    <physics type="ode">
      <gravity>1 0 -9.8</gravity>
      <magnetic_field>1 2 3</magnetic_field>
    </physics>
  </world>
</sdf>)";

  sdf::SDFPtr sdf(new sdf::SDF());
  sdf::init(sdf);

  EXPECT_TRUE(sdf::convertString(xmlString, "1.6", sdf));
  ASSERT_NE(nullptr, sdf->Root());
  EXPECT_EQ(sdf->Root()->GetName(), "sdf");
  EXPECT_EQ("1.6", sdf->Root()->Get<std::string>("version"));
  EXPECT_EQ(_version, sdf->OriginalVersion());
  EXPECT_EQ(_version, sdf->Root()->OriginalVersion());

  sdf::ElementPtr worldElem = sdf->Root()->GetElement("world");
  ASSERT_NE(nullptr, worldElem);
  EXPECT_EQ(worldElem->Get<std::string>("name"), "default");
  EXPECT_EQ(_version, worldElem->OriginalVersion());

  sdf::ElementPtr physicsElem = worldElem->GetElement("physics");
  ASSERT_NE(nullptr, physicsElem);
  EXPECT_EQ(physicsElem->Get<std::string>("name"), "default_physics");
  EXPECT_EQ(physicsElem->Get<std::string>("type"), "ode");
  EXPECT_EQ(_version, physicsElem->OriginalVersion());

  // gravity and magnetic_field should have been moved from physics to world
  EXPECT_FALSE(physicsElem->HasElement("gravity"));
  EXPECT_FALSE(physicsElem->HasElement("magnetic_field"));

  sdf::ElementPtr gravityElem = worldElem->GetElement("gravity");
  ASSERT_NE(nullptr, gravityElem);
  EXPECT_EQ(gravityElem->Get<ignition::math::Vector3d>(),
            ignition::math::Vector3d(1, 0, -9.8));
  EXPECT_EQ(_version, gravityElem->OriginalVersion());

  sdf::ElementPtr magElem = worldElem->GetElement("magnetic_field");
  ASSERT_NE(nullptr, magElem);
  EXPECT_EQ(magElem->Get<ignition::math::Vector3d>(),
            ignition::math::Vector3d(1, 2, 3));
  EXPECT_EQ(_version, magElem->OriginalVersion());
}

/////////////////////////////////////////////////
/// Test conversion of gravity, magnetic_field in 1.5 to 1.6
TEST(ConverterIntegration, World_15_to_16)
{
  // The gravity and magnetic_field in 1.5 format
  std::string xmlString = R"(
<?xml version="1.0" ?>
<sdf version="1.5">
  <world name="default">
    <physics type="ode">
      <gravity>1 0 -9.8</gravity>
      <magnetic_field>1 2 3</magnetic_field>
    </physics>
  </world>
</sdf>)";

  TiXmlDocument xmlDoc;
  xmlDoc.Parse(xmlString.c_str());

  // Convert
  TiXmlDocument convertXmlDoc;
  convertXmlDoc.LoadFile(CONVERT_DOC_15_16);
  sdf::Converter::Convert(&xmlDoc, &convertXmlDoc);

  // Check some basic elements
  TiXmlElement *convertedElem =  xmlDoc.FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "sdf");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "world");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "physics");

  // gravity and magnetic_field should have been moved from physics to world
  EXPECT_EQ(nullptr, convertedElem->FirstChildElement("gravity"));
  EXPECT_EQ(nullptr, convertedElem->FirstChildElement("magnetic_field"));

  // Get the gravity
  TiXmlElement *gravityElem = convertedElem->NextSiblingElement("gravity");
  ASSERT_NE(nullptr, gravityElem);
  EXPECT_STREQ(gravityElem->GetText(), "1 0 -9.8");

  // Get the magnetic_field
  TiXmlElement *magneticFieldElem =
    convertedElem->NextSiblingElement("magnetic_field");
  ASSERT_NE(nullptr, magneticFieldElem);
  EXPECT_STREQ(magneticFieldElem->GetText(), "1 2 3");
}

/////////////////////////////////////////////////
/// Test conversion of pose attributes in 1.6 to 1.7
TEST(ConverterIntegration, Pose_16_to_17)
{
  // A world file with pose elements in 1.5 format
  std::string xmlString = R"(
<?xml version="1.0" ?>
<sdf version="1.6">
  <world name="default">
    <model name="model">
      <pose frame="world">0 0 0 0 0 0</pose>
      <link name="parent"/>
      <link name="child">
        <pose frame="joint">0 0 0 0 0 0</pose>
      </link>
      <joint name="joint" type="fixed">
        <parent>parent</parent>
        <child>child</child>
        <pose frame="parent">0 0 0 0 0 0</pose>
      </joint>
    </model>
  </world>
</sdf>)";

  TiXmlDocument xmlDoc;
  xmlDoc.Parse(xmlString.c_str());

  // Convert
  TiXmlDocument convertXmlDoc;
  convertXmlDoc.LoadFile(CONVERT_DOC_16_17);
  sdf::Converter::Convert(&xmlDoc, &convertXmlDoc);

  // Check some basic elements
  TiXmlElement *convertedElem =  xmlDoc.FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "sdf");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "world");
  convertedElem = convertedElem->FirstChildElement();
  EXPECT_EQ(convertedElem->ValueStr(), "model");

  TiXmlElement *modelPoseElem = convertedElem->FirstChildElement();
  ASSERT_NE(nullptr, modelPoseElem);
  EXPECT_EQ("pose", modelPoseElem->ValueStr());
  // frame attribute should have been moved to relative_to
  EXPECT_EQ(nullptr, modelPoseElem->Attribute("frame"));
  EXPECT_NE(nullptr, modelPoseElem->Attribute("relative_to"));
  EXPECT_STREQ("world", modelPoseElem->Attribute("relative_to"));

  TiXmlElement *parentLinkElem = modelPoseElem->NextSiblingElement();
  ASSERT_NE(nullptr, parentLinkElem);
  EXPECT_EQ("link", parentLinkElem->ValueStr());
  EXPECT_EQ(nullptr, parentLinkElem->FirstChildElement());

  TiXmlElement *childLinkElem = parentLinkElem->NextSiblingElement();
  ASSERT_NE(nullptr, childLinkElem);
  EXPECT_EQ("link", childLinkElem->ValueStr());
  TiXmlElement *childLinkPoseElem = childLinkElem->FirstChildElement();
  ASSERT_NE(nullptr, childLinkPoseElem);
  EXPECT_EQ("pose", childLinkPoseElem->ValueStr());
  // frame attribute should have been moved to relative_to
  EXPECT_EQ(nullptr, childLinkPoseElem->Attribute("frame"));
  EXPECT_NE(nullptr, childLinkPoseElem->Attribute("relative_to"));
  EXPECT_STREQ("joint", childLinkPoseElem->Attribute("relative_to"));

  TiXmlElement *jointLinkElem = childLinkElem->NextSiblingElement();
  ASSERT_NE(nullptr, jointLinkElem);
  EXPECT_EQ("joint", jointLinkElem->ValueStr());
  TiXmlElement *jointLinkPoseElem = jointLinkElem->FirstChildElement("pose");
  ASSERT_NE(nullptr, jointLinkPoseElem);
  EXPECT_EQ("pose", jointLinkPoseElem->ValueStr());
  // frame attribute should have been moved to relative_to
  EXPECT_EQ(nullptr, jointLinkPoseElem->Attribute("frame"));
  EXPECT_NE(nullptr, jointLinkPoseElem->Attribute("relative_to"));
  EXPECT_STREQ("parent", jointLinkPoseElem->Attribute("relative_to"));
}
