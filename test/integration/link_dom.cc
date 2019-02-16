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

#include <string>
#include <gtest/gtest.h>

#include <ignition/math/Pose3.hh>
#include "sdf/Collision.hh"
#include "sdf/Element.hh"
#include "sdf/Error.hh"
#include "sdf/Filesystem.hh"
#include "sdf/Link.hh"
#include "sdf/Model.hh"
#include "sdf/parser.hh"
#include "sdf/Root.hh"
#include "sdf/Sensor.hh"
#include "sdf/Types.hh"
#include "sdf/Visual.hh"
#include "sdf/World.hh"
#include "test_config.h"

//////////////////////////////////////////////////
TEST(DOMLink, NotALink)
{
  // Create an Element that is not a link
  sdf::ElementPtr element(new sdf::Element);
  element->SetName("world");
  sdf::Link link;
  std::shared_ptr<sdf::FrameGraph> frameGraph(new sdf::FrameGraph);
  sdf::Errors errors = link.Load(element, frameGraph);
  ASSERT_FALSE(errors.empty());
  EXPECT_EQ(sdf::ErrorCode::ELEMENT_INCORRECT_TYPE, errors[0].Code());
  EXPECT_TRUE(errors[0].Message().find("Attempting to load a Link") !=
               std::string::npos);
}

//////////////////////////////////////////////////
TEST(DOMLink, NoName)
{
  // Create a "link" with no name
  sdf::ElementPtr element(new sdf::Element);
  element->SetName("link");

  sdf::Link link;
  std::shared_ptr<sdf::FrameGraph> frameGraph(new sdf::FrameGraph);
  sdf::Errors errors = link.Load(element, frameGraph);
  ASSERT_FALSE(errors.empty());
  EXPECT_EQ(sdf::ErrorCode::ATTRIBUTE_MISSING, errors[0].Code());
  EXPECT_TRUE(errors[0].Message().find("link name is required") !=
               std::string::npos);
}

//////////////////////////////////////////////////
TEST(DOMLink, NoFrameGraph)
{
  // Create a "link"
  sdf::ElementPtr element(new sdf::Element);
  element->SetName("link");
  element->AddAttribute("name", "string", "link", true, "name");

  sdf::Link link;
  sdf::Errors errors = link.Load(element, nullptr);
  ASSERT_FALSE(errors.empty());
  EXPECT_EQ(sdf::ErrorCode::FUNCTION_ARGUMENT_MISSING, errors[0].Code());

  EXPECT_TRUE(errors[0].Message().find("frame graph is required") !=
               std::string::npos);
}

//////////////////////////////////////////////////
TEST(DOMLink, CollisionSiblingsSameName)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "link_collision_siblings_same_name.sdf");

  // Load the SDF file
  sdf::Root root;
  sdf::Errors errors = root.Load(testFile);
  ASSERT_FALSE(errors.empty());
  EXPECT_EQ(1u, errors.size());
  EXPECT_EQ(sdf::ErrorCode::DUPLICATE_NAME, errors[0].Code());
  EXPECT_NE(std::string::npos, errors[0].Message().find(
      "collision with name[collision] already exists"));
}

//////////////////////////////////////////////////
TEST(DOMLink, CollisionCousinsSameName)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "link_collision_cousins_same_name.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  // Get the first model
  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_NE(nullptr, model);
  EXPECT_EQ("link_collision_cousins_same_name", model->Name());
  EXPECT_EQ(2u, model->LinkCount());
  EXPECT_EQ(0u, model->JointCount());

  const sdf::Link *link1 = model->LinkByName("link1");
  ASSERT_TRUE(link1 != nullptr);
  const sdf::Link *link2 = model->LinkByName("link2");
  ASSERT_TRUE(link2 != nullptr);
  EXPECT_EQ(1u, link1->CollisionCount());
  EXPECT_EQ(1u, link2->CollisionCount());

  const sdf::Collision *collision1 = link1->CollisionByName("collision");
  ASSERT_TRUE(collision1 != nullptr);
  const sdf::Collision *collision2 = link2->CollisionByName("collision");
  ASSERT_TRUE(collision2 != nullptr);

  using Pose3d = ignition::math::Pose3d;

  EXPECT_EQ(model->Name(), link1->PoseFrame());
  EXPECT_EQ(Pose3d(1, 0, 0, 0, 0, 0), link1->Pose());
  EXPECT_EQ(Pose3d(1, 0, 0, 0, 0, 0), link1->PoseInFrame(model->Name()));

  EXPECT_EQ(model->Name(), link2->PoseFrame());
  EXPECT_EQ(Pose3d(2, 0, 0, 0, 0, 0), link2->Pose());
  EXPECT_EQ(Pose3d(2, 0, 0, 0, 0, 0), link2->PoseInFrame(model->Name()));

  EXPECT_EQ(link1->Name(), collision1->PoseFrame());
  EXPECT_EQ(Pose3d(0, 1, 0, 0, 0, 0), collision1->Pose());
  // this collision has the same name as its cousin
  // duplicate name in frame graph causes PoseInFrame to return infinite pose
  EXPECT_FALSE(collision1->PoseInFrame(model->Name()).IsFinite());

  EXPECT_EQ(link2->Name(), collision2->PoseFrame());
  EXPECT_EQ(Pose3d(0, 2, 0, 0, 0, 0), collision2->Pose());
  // this collision has the same name as its cousin
  // duplicate name in frame graph causes PoseInFrame to return infinite pose
  EXPECT_FALSE(collision2->PoseInFrame(model->Name()).IsFinite());
}

//////////////////////////////////////////////////
TEST(DOMLink, VisualSiblingsSameName)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "link_visual_siblings_same_name.sdf");

  // Load the SDF file
  sdf::Root root;
  sdf::Errors errors = root.Load(testFile);
  ASSERT_FALSE(errors.empty());
  EXPECT_EQ(1u, errors.size());
  EXPECT_EQ(sdf::ErrorCode::DUPLICATE_NAME, errors[0].Code());
  EXPECT_NE(std::string::npos, errors[0].Message().find(
      "visual with name[visual] already exists"));
}

//////////////////////////////////////////////////
TEST(DOMLink, VisualCousinsSameName)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "link_visual_cousins_same_name.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  // Get the first model
  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_NE(nullptr, model);
  EXPECT_EQ("link_visual_cousins_same_name", model->Name());
  EXPECT_EQ(2u, model->LinkCount());
  EXPECT_EQ(0u, model->JointCount());

  const sdf::Link *link1 = model->LinkByName("link1");
  ASSERT_TRUE(link1 != nullptr);
  const sdf::Link *link2 = model->LinkByName("link2");
  ASSERT_TRUE(link2 != nullptr);
  EXPECT_EQ(1u, link1->VisualCount());
  EXPECT_EQ(1u, link2->VisualCount());

  const sdf::Visual *visual1 = link1->VisualByName("visual");
  ASSERT_TRUE(visual1 != nullptr);
  const sdf::Visual *visual2 = link2->VisualByName("visual");
  ASSERT_TRUE(visual2 != nullptr);

  using Pose3d = ignition::math::Pose3d;

  EXPECT_EQ(model->Name(), link1->PoseFrame());
  EXPECT_EQ(Pose3d(1, 0, 0, 0, 0, 0), link1->Pose());
  EXPECT_EQ(Pose3d(1, 0, 0, 0, 0, 0), link1->PoseInFrame(model->Name()));

  EXPECT_EQ(model->Name(), link2->PoseFrame());
  EXPECT_EQ(Pose3d(2, 0, 0, 0, 0, 0), link2->Pose());
  EXPECT_EQ(Pose3d(2, 0, 0, 0, 0, 0), link2->PoseInFrame(model->Name()));

  EXPECT_EQ(link1->Name(), visual1->PoseFrame());
  EXPECT_EQ(Pose3d(0, 1, 0, 0, 0, 0), visual1->Pose());
  // this visual has the same name as its cousin
  // duplicate name in frame graph causes PoseInFrame to return infinite pose
  EXPECT_FALSE(visual1->PoseInFrame(model->Name()).IsFinite());

  EXPECT_EQ(link2->Name(), visual2->PoseFrame());
  EXPECT_EQ(Pose3d(0, 2, 0, 0, 0, 0), visual2->Pose());
  // this visual has the same name as its cousin
  // duplicate name in frame graph causes PoseInFrame to return infinite pose
  EXPECT_FALSE(visual2->PoseInFrame(model->Name()).IsFinite());
}

//////////////////////////////////////////////////
TEST(DOMLink, LoadVisualCollision)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "empty.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  // Get the first world
  const sdf::World *world = root.WorldByIndex(0);
  ASSERT_NE(nullptr, world);
  EXPECT_EQ("default", world->Name());

  // Get the first model
  const sdf::Model *model = world->ModelByIndex(0);
  ASSERT_NE(nullptr, model);
  EXPECT_EQ("ground_plane", model->Name());

  // Get the first link
  const sdf::Link *link = model->LinkByIndex(0);
  ASSERT_NE(nullptr, link);
  EXPECT_EQ("link", link->Name());

  // Get the first visual
  EXPECT_EQ(1u, link->VisualCount());
  EXPECT_TRUE(link->VisualNameExists("visual"));
  EXPECT_FALSE(link->VisualNameExists("visuals"));
  const sdf::Visual *visual = link->VisualByIndex(0);
  ASSERT_NE(nullptr, visual);
  EXPECT_EQ("visual", visual->Name());

  // Get the first collision
  EXPECT_EQ(1u, link->CollisionCount());
  EXPECT_TRUE(link->CollisionNameExists("collision"));
  EXPECT_FALSE(link->CollisionNameExists("collisions"));
  const sdf::Collision *collision = link->CollisionByIndex(0);
  ASSERT_NE(nullptr, collision);
  EXPECT_EQ("collision", collision->Name());
}

//////////////////////////////////////////////////
TEST(DOMLink, InertialDoublePendulum)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "double_pendulum.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_NE(nullptr, model);

  const sdf::Link *baseLink = model->LinkByIndex(0);
  ASSERT_NE(nullptr, baseLink);
  EXPECT_EQ(ignition::math::Pose3d::Zero, baseLink->PoseInFrame("base"));
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 0, 0, 0, 0), baseLink->Pose());
  EXPECT_EQ("double_pendulum_with_base", baseLink->PoseFrame());

  const ignition::math::Inertiald inertial = baseLink->Inertial();
  EXPECT_DOUBLE_EQ(100.0, inertial.MassMatrix().Mass());
  EXPECT_DOUBLE_EQ(1.0, inertial.MassMatrix().DiagonalMoments().X());
  EXPECT_DOUBLE_EQ(1.0, inertial.MassMatrix().DiagonalMoments().Y());
  EXPECT_DOUBLE_EQ(1.0, inertial.MassMatrix().DiagonalMoments().Z());
  EXPECT_DOUBLE_EQ(0.0, inertial.MassMatrix().OffDiagonalMoments().X());
  EXPECT_DOUBLE_EQ(0.0, inertial.MassMatrix().OffDiagonalMoments().Y());
  EXPECT_DOUBLE_EQ(0.0, inertial.MassMatrix().OffDiagonalMoments().Z());

  const sdf::Link *upperLink = model->LinkByIndex(1);
  ASSERT_NE(nullptr, upperLink);
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 0, 0, 0, 0),
      upperLink->PoseInFrame("upper_link"));
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 2.1, -1.5708, 0, 0),
      upperLink->Pose());
  EXPECT_EQ("double_pendulum_with_base", upperLink->PoseFrame());

  const ignition::math::Inertiald inertialUpper = upperLink->Inertial();
  EXPECT_DOUBLE_EQ(1.0, inertialUpper.MassMatrix().Mass());
  EXPECT_DOUBLE_EQ(1.0, inertialUpper.MassMatrix().DiagonalMoments().X());
  EXPECT_DOUBLE_EQ(1.0, inertialUpper.MassMatrix().DiagonalMoments().Y());
  EXPECT_DOUBLE_EQ(1.0, inertialUpper.MassMatrix().DiagonalMoments().Z());
  EXPECT_DOUBLE_EQ(0.0, inertialUpper.MassMatrix().OffDiagonalMoments().X());
  EXPECT_DOUBLE_EQ(0.0, inertialUpper.MassMatrix().OffDiagonalMoments().Y());
  EXPECT_DOUBLE_EQ(0.0, inertialUpper.MassMatrix().OffDiagonalMoments().Z());
  EXPECT_DOUBLE_EQ(0.0, inertialUpper.Pose().Pos().X());
  EXPECT_DOUBLE_EQ(0.0, inertialUpper.Pose().Pos().Y());
  EXPECT_DOUBLE_EQ(0.5, inertialUpper.Pose().Pos().Z());
  EXPECT_TRUE(inertial.MassMatrix().IsValid());

  const sdf::Link *lowerLink = model->LinkByIndex(2);
  ASSERT_TRUE(lowerLink != nullptr);
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 0, 0, 0, 0),
      lowerLink->PoseInFrame("lower_link"));
  EXPECT_EQ(ignition::math::Pose3d(0.25, 1.0, 2.1, -2, 0, 0),
      lowerLink->Pose());
  EXPECT_EQ("double_pendulum_with_base", lowerLink->PoseFrame());
}

//////////////////////////////////////////////////
TEST(DOMLink, InertialComplete)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "inertial_complete.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_NE(nullptr, model);

  const sdf::Link *link = model->LinkByIndex(0);
  ASSERT_NE(nullptr, link);

  const ignition::math::Inertiald inertial = link->Inertial();
  EXPECT_DOUBLE_EQ(17.982, inertial.MassMatrix().Mass());
  EXPECT_DOUBLE_EQ(0.125569, inertial.MassMatrix().DiagonalMoments().X());
  EXPECT_DOUBLE_EQ(0.0972062, inertial.MassMatrix().DiagonalMoments().Y());
  EXPECT_DOUBLE_EQ(0.117937, inertial.MassMatrix().DiagonalMoments().Z());
  EXPECT_DOUBLE_EQ(0.0008, inertial.MassMatrix().OffDiagonalMoments().X());
  EXPECT_DOUBLE_EQ(-0.000499757,
      inertial.MassMatrix().OffDiagonalMoments().Y());
  EXPECT_DOUBLE_EQ(-0.0005, inertial.MassMatrix().OffDiagonalMoments().Z());
  EXPECT_DOUBLE_EQ(0.01, inertial.Pose().Pos().X());
  EXPECT_DOUBLE_EQ(0.0, inertial.Pose().Pos().Y());
  EXPECT_DOUBLE_EQ(0.02, inertial.Pose().Pos().Z());
  EXPECT_TRUE(inertial.MassMatrix().IsValid());
}

//////////////////////////////////////////////////
TEST(DOMLink, InertialInvalid)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "inertial_invalid.sdf");

  // Load the SDF file
  sdf::Root root;
  auto errors = root.Load(testFile);
  EXPECT_FALSE(errors.empty());
  ASSERT_EQ(1u, errors.size());
  EXPECT_EQ(errors[0].Code(), sdf::ErrorCode::LINK_INERTIA_INVALID);
  EXPECT_EQ(errors[0].Message(), "A link named link has invalid inertia.");

  // TODO: make this failure less severe?
  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_EQ(model, nullptr);
}

//////////////////////////////////////////////////
TEST(DOMLink, Sensors)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "sensors.sdf");

  // Load the SDF file
  sdf::Root root;
  auto errors = root.Load(testFile);
  EXPECT_TRUE(errors.empty());

  // Get the first model
  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_NE(nullptr, model);
  EXPECT_EQ("model", model->Name());

  // Get the first link
  const sdf::Link *link = model->LinkByIndex(0);
  ASSERT_NE(nullptr, link);
  EXPECT_EQ("link", link->Name());
  EXPECT_EQ(17u, link->SensorCount());

  // Get the altimeter sensor
  const sdf::Sensor *altimeterSensor = link->SensorByIndex(0);
  ASSERT_NE(nullptr, altimeterSensor);
  EXPECT_EQ("altimeter_sensor", altimeterSensor->Name());
  EXPECT_EQ(sdf::SensorType::ALTIMETER, altimeterSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d::Zero, altimeterSensor->Pose());

  // Get the camera sensor
  EXPECT_TRUE(link->SensorNameExists("camera_sensor"));
  EXPECT_FALSE(link->SensorNameExists("bad_camera_sensor"));
  const sdf::Sensor *cameraSensor = link->SensorByName("camera_sensor");
  ASSERT_NE(nullptr, cameraSensor);
  EXPECT_EQ("camera_sensor", cameraSensor->Name());
  EXPECT_EQ(sdf::SensorType::CAMERA, cameraSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(1, 2, 3, 0, 0, 0), cameraSensor->Pose());

  // Get the contact sensor
  const sdf::Sensor *contactSensor = link->SensorByName("contact_sensor");
  ASSERT_NE(nullptr, contactSensor);
  EXPECT_EQ("contact_sensor", contactSensor->Name());
  EXPECT_EQ(sdf::SensorType::CONTACT, contactSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(4, 5, 6, 0, 0, 0), contactSensor->Pose());

  // Get the depth sensor
  const sdf::Sensor *depthSensor = link->SensorByName("depth_sensor");
  ASSERT_NE(nullptr, depthSensor);
  EXPECT_EQ("depth_sensor", depthSensor->Name());
  EXPECT_EQ(sdf::SensorType::DEPTH, depthSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(7, 8, 9, 0, 0, 0), depthSensor->Pose());

  // Get the force_torque sensor
  const sdf::Sensor *forceTorqueSensor =
    link->SensorByName("force_torque_sensor");
  ASSERT_NE(nullptr, forceTorqueSensor);
  EXPECT_EQ("force_torque_sensor", forceTorqueSensor->Name());
  EXPECT_EQ(sdf::SensorType::FORCE_TORQUE, forceTorqueSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(10, 11, 12, 0, 0, 0),
      forceTorqueSensor->Pose());

  // Get the gps sensor
  const sdf::Sensor *gpsSensor = link->SensorByName("gps_sensor");
  ASSERT_NE(nullptr, gpsSensor);
  EXPECT_EQ("gps_sensor", gpsSensor->Name());
  EXPECT_EQ(sdf::SensorType::GPS, gpsSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(13, 14, 15, 0, 0, 0), gpsSensor->Pose());

  // Get the gpuRay sensor
  const sdf::Sensor *gpuRaySensor = link->SensorByName("gpu_ray_sensor");
  ASSERT_NE(nullptr, gpuRaySensor);
  EXPECT_EQ("gpu_ray_sensor", gpuRaySensor->Name());
  EXPECT_EQ(sdf::SensorType::GPU_RAY, gpuRaySensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(1, 2, 3, 0, 0, 0), gpuRaySensor->Pose());

  // Get the imu sensor
  const sdf::Sensor *imuSensor = link->SensorByName("imu_sensor");
  ASSERT_NE(nullptr, imuSensor);
  EXPECT_EQ("imu_sensor", imuSensor->Name());
  EXPECT_EQ(sdf::SensorType::IMU, imuSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(4, 5, 6, 0, 0, 0), imuSensor->Pose());

  // Get the logical camera sensor
  const sdf::Sensor *logicalCameraSensor =
    link->SensorByName("logical_camera_sensor");
  ASSERT_NE(nullptr, logicalCameraSensor);
  EXPECT_EQ("logical_camera_sensor", logicalCameraSensor->Name());
  EXPECT_EQ(sdf::SensorType::LOGICAL_CAMERA, logicalCameraSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(7, 8, 9, 0, 0, 0),
      logicalCameraSensor->Pose());

  // Get the magnetometer sensor
  const sdf::Sensor *magnetometerSensor =
    link->SensorByName("magnetometer_sensor");
  ASSERT_NE(nullptr, magnetometerSensor);
  EXPECT_EQ("magnetometer_sensor", magnetometerSensor->Name());
  EXPECT_EQ(sdf::SensorType::MAGNETOMETER, magnetometerSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(10, 11, 12, 0, 0, 0),
      magnetometerSensor->Pose());

  // Get the multicamera sensor
  const sdf::Sensor *multicameraSensor =
    link->SensorByName("multicamera_sensor");
  ASSERT_NE(nullptr, multicameraSensor);
  EXPECT_EQ("multicamera_sensor", multicameraSensor->Name());
  EXPECT_EQ(sdf::SensorType::MULTICAMERA, multicameraSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(13, 14, 15, 0, 0, 0),
      multicameraSensor->Pose());

  // Get the ray sensor
  const sdf::Sensor *raySensor = link->SensorByName("ray_sensor");
  ASSERT_NE(nullptr, raySensor);
  EXPECT_EQ("ray_sensor", raySensor->Name());
  EXPECT_EQ(sdf::SensorType::RAY, raySensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(1, 2, 3, 0, 0, 0), raySensor->Pose());

  // Get the rfid sensor
  const sdf::Sensor *rfidSensor = link->SensorByName("rfid_sensor");
  ASSERT_NE(nullptr, rfidSensor);
  EXPECT_EQ("rfid_sensor", rfidSensor->Name());
  EXPECT_EQ(sdf::SensorType::RFID, rfidSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(4, 5, 6, 0, 0, 0), rfidSensor->Pose());

  // Get the rfid tag
  const sdf::Sensor *rfidTag = link->SensorByName("rfid_tag");
  ASSERT_NE(nullptr, rfidTag);
  EXPECT_EQ("rfid_tag", rfidTag->Name());
  EXPECT_EQ(sdf::SensorType::RFIDTAG, rfidTag->Type());
  EXPECT_EQ(ignition::math::Pose3d(7, 8, 9, 0, 0, 0), rfidTag->Pose());

  // Get the sonar sensor
  const sdf::Sensor *sonarSensor = link->SensorByName("sonar_sensor");
  ASSERT_NE(nullptr, sonarSensor);
  EXPECT_EQ("sonar_sensor", sonarSensor->Name());
  EXPECT_EQ(sdf::SensorType::SONAR, sonarSensor->Type());
  EXPECT_EQ(ignition::math::Pose3d(10, 11, 12, 0, 0, 0), sonarSensor->Pose());

  // Get the wireless receiver
  const sdf::Sensor *wirelessReceiver = link->SensorByName("wireless_receiver");
  ASSERT_NE(nullptr, wirelessReceiver);
  EXPECT_EQ("wireless_receiver", wirelessReceiver->Name());
  EXPECT_EQ(sdf::SensorType::WIRELESS_RECEIVER, wirelessReceiver->Type());
  EXPECT_EQ(ignition::math::Pose3d(13, 14, 15, 0, 0, 0),
      wirelessReceiver->Pose());

  // Get the wireless transmitter
  const sdf::Sensor *wirelessTransmitter =
    link->SensorByName("wireless_transmitter");
  ASSERT_NE(nullptr, wirelessTransmitter);
  EXPECT_EQ("wireless_transmitter", wirelessTransmitter->Name());
  EXPECT_EQ(sdf::SensorType::WIRELESS_TRANSMITTER, wirelessTransmitter->Type());
  EXPECT_EQ(ignition::math::Pose3d(1, 2, 3, 0, 0, 0),
      wirelessTransmitter->Pose());
}

//////////////////////////////////////////////////
TEST(DOMLink, LinkChain)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "link_chain.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  const sdf::Model *model = root.ModelByIndex(0);
  ASSERT_TRUE(model != nullptr);
  const sdf::Link *linkOne = model->LinkByName("one");
  ASSERT_TRUE(linkOne != nullptr);
  const sdf::Link *linkTwo = model->LinkByName("two");
  ASSERT_TRUE(linkTwo != nullptr);
  const sdf::Link *linkThree = model->LinkByName("three");
  ASSERT_TRUE(linkThree != nullptr);
  const sdf::Link *linkFour = model->LinkByName("four");
  ASSERT_TRUE(linkFour != nullptr);

  // Expect reversing link and frame names should negate the pose
  for (const std::string & linkNameA : {"one", "two", "three", "four"})
  {
    for (const std::string & linkNameB : {"one", "two", "three", "four"})
    {
      EXPECT_EQ(model->LinkByName(linkNameA)->PoseInFrame(linkNameB),
               -model->LinkByName(linkNameB)->PoseInFrame(linkNameA))
        << "linkNameA[" << linkNameA << "] "
        << "linkNameB[" << linkNameB << "]";
    }
  }

  using Pose3d = ignition::math::Pose3d;

  // raw pose data
  EXPECT_TRUE(model->PoseFrame().empty());
  EXPECT_EQ(Pose3d(0, 0, 0, 0, 0, 0), model->Pose());

  EXPECT_EQ(model->Name(), linkOne->PoseFrame());
  EXPECT_EQ(Pose3d(1, -1, 0, 0, 0, 0), linkOne->Pose());

  EXPECT_EQ(model->Name(), linkTwo->PoseFrame());
  EXPECT_EQ(Pose3d(3, 0, 0, 0, 0, 0), linkTwo->Pose());

  EXPECT_EQ("two", linkThree->PoseFrame());
  EXPECT_EQ(Pose3d(0, 2, 0, 0, 0, 0), linkThree->Pose());

  EXPECT_EQ("three", linkFour->PoseFrame());
  EXPECT_EQ(Pose3d(0, 0, -5, 0, 0, 0), linkFour->Pose());

  // link poses in model frame
  EXPECT_EQ(Pose3d(1, -1, 0, 0, 0, 0), linkOne->PoseInFrame(model->Name()));
  EXPECT_EQ(Pose3d(3, 0, 0, 0, 0, 0), linkTwo->PoseInFrame(model->Name()));
  EXPECT_EQ(Pose3d(3, 2, 0, 0, 0, 0), linkThree->PoseInFrame(model->Name()));
  EXPECT_EQ(Pose3d(3, 2, -5, 0, 0, 0), linkFour->PoseInFrame(model->Name()));

  // model pose in link frames should be negative of link poses in model frame
  EXPECT_EQ(-Pose3d(1, -1, 0, 0, 0, 0), model->PoseInFrame("one"));
  EXPECT_EQ(-Pose3d(3, 0, 0, 0, 0, 0),  model->PoseInFrame("two"));
  EXPECT_EQ(-Pose3d(3, 2, 0, 0, 0, 0),  model->PoseInFrame("three"));
  EXPECT_EQ(-Pose3d(3, 2, -5, 0, 0, 0), model->PoseInFrame("four"));

  // link poses in frame "one"
  EXPECT_EQ(Pose3d(0, 0, 0, 0, 0, 0), linkOne->PoseInFrame("one"));
  EXPECT_EQ(Pose3d(2, 1, 0, 0, 0, 0), linkTwo->PoseInFrame("one"));
  EXPECT_EQ(Pose3d(2, 3, 0, 0, 0, 0), linkThree->PoseInFrame("one"));
  EXPECT_EQ(Pose3d(2, 3, -5, 0, 0, 0), linkFour->PoseInFrame("one"));

  // link poses in frame "two"
  EXPECT_EQ(Pose3d(-2, -1, 0, 0, 0, 0), linkOne->PoseInFrame("two"));
  EXPECT_EQ(Pose3d(0, 0, 0, 0, 0, 0), linkTwo->PoseInFrame("two"));
  EXPECT_EQ(Pose3d(0, 2, 0, 0, 0, 0), linkThree->PoseInFrame("two"));
  EXPECT_EQ(Pose3d(0, 2, -5, 0, 0, 0), linkFour->PoseInFrame("two"));

  // link poses in frame "three"
  EXPECT_EQ(Pose3d(-2, -3, 0, 0, 0, 0), linkOne->PoseInFrame("three"));
  EXPECT_EQ(Pose3d(0, -2, 0, 0, 0, 0), linkTwo->PoseInFrame("three"));
  EXPECT_EQ(Pose3d(0, 0, 0, 0, 0, 0), linkThree->PoseInFrame("three"));
  EXPECT_EQ(Pose3d(0, 0, -5, 0, 0, 0), linkFour->PoseInFrame("three"));

  // link poses in frame "four"
  EXPECT_EQ(Pose3d(-2, -3, 5, 0, 0, 0), linkOne->PoseInFrame("four"));
  EXPECT_EQ(Pose3d(0, -2, 5, 0, 0, 0), linkTwo->PoseInFrame("four"));
  EXPECT_EQ(Pose3d(0, 0, 5, 0, 0, 0), linkThree->PoseInFrame("four"));
  EXPECT_EQ(Pose3d(0, 0, 0, 0, 0, 0), linkFour->PoseInFrame("four"));

  // link poses in frame "other_frame"
  EXPECT_EQ(-Pose3d(9, 11, 10, 0, 0, 0), linkOne->PoseInFrame("other_frame"));
  EXPECT_EQ(-Pose3d(7, 10, 10, 0, 0, 0), linkTwo->PoseInFrame("other_frame"));
  EXPECT_EQ(-Pose3d(7, 8, 10, 0, 0, 0), linkThree->PoseInFrame("other_frame"));
  EXPECT_EQ(-Pose3d(7, 8, 15, 0, 0, 0), linkFour->PoseInFrame("other_frame"));

  // link poses in frame "frame_in_two"
  EXPECT_EQ(Pose3d(-3, -3, -3, 0, 0, 0), linkOne->PoseInFrame("frame_in_two"));
  EXPECT_EQ(Pose3d(-1, -2, -3, 0, 0, 0), linkTwo->PoseInFrame("frame_in_two"));
  EXPECT_EQ(Pose3d(-1, 0, -3, 0, 0, 0), linkThree->PoseInFrame("frame_in_two"));
  EXPECT_EQ(Pose3d(-1, 0, -8, 0, 0, 0), linkFour->PoseInFrame("frame_in_two"));

  // test changing the pose information programmatically
  // need a non-const pointer to link
  sdf::Link *link1 = model->LinkByName("one");
  ASSERT_TRUE(link1 != nullptr);
  const ignition::math::Pose3d newPose(4, 5, 6, .1, 0, 0);
  link1->SetPose(newPose);
  EXPECT_EQ(newPose, link1->Pose());
  EXPECT_EQ(newPose, link1->PoseInFrame());
  EXPECT_EQ(newPose, link1->PoseInFrame(model->Name()));
}

