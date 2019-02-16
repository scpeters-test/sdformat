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
#include <vector>
#include <ignition/math/Inertial.hh>
#include <ignition/math/Matrix4.hh>
#include <ignition/math/Pose3.hh>
#include <ignition/math/Vector3.hh>

#include "sdf/Collision.hh"
#include "sdf/Error.hh"
#include "sdf/Link.hh"
#include "sdf/Sensor.hh"
#include "sdf/Types.hh"
#include "sdf/Visual.hh"
#include "Utils.hh"

using namespace sdf;

class sdf::LinkPrivate
{
  /// \brief Pose of the link
  public: ignition::math::Pose3d pose = ignition::math::Pose3d::Zero;

  /// \brief Frame of the pose.
  public: std::string poseFrame = "";

  /// \brief The visuals specified in this link.
  public: std::vector<Visual> visuals;

  /// \brief The collisions specified in this link.
  public: std::vector<Collision> collisions;

  /// \brief The sensors specified in this link.
  public: std::vector<Sensor> sensors;

  /// \brief The inertial information for this link.
  public: ignition::math::Inertiald inertial {{1.0,
            ignition::math::Vector3d::One, ignition::math::Vector3d::Zero},
            ignition::math::Pose3d::Zero};

  /// \brief Pointer to the frame graph.
  public: std::shared_ptr<FrameGraph> frameGraph = nullptr;

  /// \brief Id of the frame for this object
  public: ignition::math::graph::VertexId frameVertexId;

  /// \brief The SDF element pointer used during load.
  public: sdf::ElementPtr sdf;
};

/////////////////////////////////////////////////
Link::Link()
  : dataPtr(new LinkPrivate)
{
  // Create the frame graph for the link, and add a node for the link.
  this->dataPtr->frameGraph.reset(new FrameGraph);
  this->dataPtr->frameVertexId = this->dataPtr->frameGraph->AddVertex(
      "", std::make_pair(this->dataPtr->pose, this->dataPtr->poseFrame)).Id();
}

/////////////////////////////////////////////////
Link::Link(Link &&_link)
{
  this->dataPtr = _link.dataPtr;
  _link.dataPtr = nullptr;
}

/////////////////////////////////////////////////
Link::~Link()
{
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

/////////////////////////////////////////////////
Errors Link::Load(ElementPtr _sdf, std::shared_ptr<FrameGraph> _frameGraph)
{
  Errors errors;

  this->dataPtr->sdf = _sdf;

  // Check that the provided SDF element is a <link>
  // This is an error that cannot be recovered, so return an error.
  if (_sdf->GetName() != "link")
  {
    errors.push_back({ErrorCode::ELEMENT_INCORRECT_TYPE,
        "Attempting to load a Link, but the provided SDF element is not a "
        "<link>."});
    return errors;
  }

  // Read the links's name
  std::string linkName;
  if (!loadName(_sdf, linkName))
  {
    errors.push_back({ErrorCode::ATTRIBUTE_MISSING,
                     "A link name is required, but the name is not set."});
  }

  // Load the pose. Ignore the return value since the pose is optional.
  loadPose(_sdf, this->dataPtr->pose, this->dataPtr->poseFrame);

  // Use the SDF parent as the pose frame if the poseFrame attribute is
  // empty.
  if (this->dataPtr->poseFrame.empty() && _sdf->GetParent())
    this->dataPtr->poseFrame = _sdf->GetParent()->Get<std::string>("name");

  if (_frameGraph)
  {
    // Add a vertex in the frame graph for this link.
    this->dataPtr->frameVertexId = _frameGraph->AddVertex(linkName,
        std::make_pair(this->dataPtr->pose, this->dataPtr->poseFrame)).Id();

    // Get the parent vertex based on this link's pose frame name.
    const ignition::math::graph::VertexRef_M<PoseWithFrameName> parentVertices =
        _frameGraph->Vertices(this->dataPtr->poseFrame);

    /// \todo check that parentVertices has an element, and potentially make
    /// sure it has only one element.

    // Connect the parent to the child
    _frameGraph->AddEdge(
        {parentVertices.begin()->first, this->dataPtr->frameVertexId},
        ignition::math::Matrix4d(this->dataPtr->pose));

    // Connect the child to the parent
    _frameGraph->AddEdge(
        {this->dataPtr->frameVertexId, parentVertices.begin()->first},
        ignition::math::Matrix4d(this->dataPtr->pose.Inverse()));

    this->dataPtr->frameGraph = _frameGraph;
  }
  else
  {
    errors.push_back({ErrorCode::FUNCTION_ARGUMENT_MISSING,
        "A frame graph is required to compute pose information."});
  }

  // Load all the visuals.
  Errors visLoadErrors = loadUniqueRepeated<Visual>(_sdf, "visual",
      this->dataPtr->visuals, _frameGraph);
  errors.insert(errors.end(), visLoadErrors.begin(), visLoadErrors.end());

  // Load all the collisions.
  Errors collLoadErrors = loadUniqueRepeated<Collision>(_sdf, "collision",
      this->dataPtr->collisions, _frameGraph);
  errors.insert(errors.end(), collLoadErrors.begin(), collLoadErrors.end());

  // Load all the sensors.
  Errors sensorLoadErrors = loadUniqueRepeated<Sensor>(_sdf, "sensor",
      this->dataPtr->sensors);
  errors.insert(errors.end(), sensorLoadErrors.begin(), sensorLoadErrors.end());

  ignition::math::Vector3d xxyyzz = ignition::math::Vector3d::One;
  ignition::math::Vector3d xyxzyz = ignition::math::Vector3d::Zero;
  ignition::math::Pose3d inertiaPose;
  std::string inertiaFrame = "";
  double mass = 1.0;

  if (_sdf->HasElement("inertial"))
  {
    sdf::ElementPtr inertialElem = _sdf->GetElement("inertial");

    if (inertialElem->HasElement("pose"))
      loadPose(inertialElem->GetElement("pose"), inertiaPose, inertiaFrame);

    // Get the mass.
    mass = inertialElem->Get<double>("mass", 1.0).first;

    if (inertialElem->HasElement("inertia"))
    {
      sdf::ElementPtr inertiaElem = inertialElem->GetElement("inertia");

      xxyyzz.X(inertiaElem->Get<double>("ixx", 1.0).first);
      xxyyzz.Y(inertiaElem->Get<double>("iyy", 1.0).first);
      xxyyzz.Z(inertiaElem->Get<double>("izz", 1.0).first);

      xyxzyz.X(inertiaElem->Get<double>("ixy", 0.0).first);
      xyxzyz.Y(inertiaElem->Get<double>("ixz", 0.0).first);
      xyxzyz.Z(inertiaElem->Get<double>("iyz", 0.0).first);
    }
  }
  if (!this->dataPtr->inertial.SetMassMatrix(
      ignition::math::MassMatrix3d(mass, xxyyzz, xyxzyz)))
  {
    errors.push_back({ErrorCode::LINK_INERTIA_INVALID,
                     "A link named " +
                     this->Name() +
                     " has invalid inertia."});
  }

  /// \todo: Handle inertia frame properly
  this->dataPtr->inertial.SetPose(inertiaPose);

  return errors;
}

/////////////////////////////////////////////////
std::string Link::Name() const
{
  return this->dataPtr->frameGraph->VertexFromId(
      this->dataPtr->frameVertexId).Name();
}

/////////////////////////////////////////////////
void Link::SetName(const std::string &_name) const
{
  // Store the name in the frame graph
  this->dataPtr->frameGraph->VertexFromId(
      this->dataPtr->frameVertexId).SetName(_name);
}

/////////////////////////////////////////////////
uint64_t Link::VisualCount() const
{
  return this->dataPtr->visuals.size();
}

/////////////////////////////////////////////////
const Visual *Link::VisualByIndex(const uint64_t _index) const
{
  if (_index < this->dataPtr->visuals.size())
    return &this->dataPtr->visuals[_index];
  return nullptr;
}

/////////////////////////////////////////////////
bool Link::VisualNameExists(const std::string &_name) const
{
  for (auto const &v : this->dataPtr->visuals)
  {
    if (v.Name() == _name)
    {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////
uint64_t Link::SensorCount() const
{
  return this->dataPtr->sensors.size();
}

/////////////////////////////////////////////////
const Sensor *Link::SensorByIndex(const uint64_t _index) const
{
  if (_index < this->dataPtr->sensors.size())
    return &this->dataPtr->sensors[_index];
  return nullptr;
}

/////////////////////////////////////////////////
bool Link::SensorNameExists(const std::string &_name) const
{
  for (auto const &s : this->dataPtr->sensors)
  {
    if (s.Name() == _name)
    {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////
const Sensor *Link::SensorByName(const std::string &_name) const
{
  for (auto const &s : this->dataPtr->sensors)
  {
    if (s.Name() == _name)
    {
      return &s;
    }
  }
  return nullptr;
}

/////////////////////////////////////////////////
uint64_t Link::CollisionCount() const
{
  return this->dataPtr->collisions.size();
}

/////////////////////////////////////////////////
const Collision *Link::CollisionByIndex(const uint64_t _index) const
{
  if (_index < this->dataPtr->collisions.size())
    return &this->dataPtr->collisions[_index];
  return nullptr;
}

/////////////////////////////////////////////////
bool Link::CollisionNameExists(const std::string &_name) const
{
  for (auto const &c : this->dataPtr->collisions)
  {
    if (c.Name() == _name)
    {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////
const ignition::math::Inertiald &Link::Inertial() const
{
  return this->dataPtr->inertial;
}

/////////////////////////////////////////////////
bool Link::SetInertial(const ignition::math::Inertiald &_inertial)
{
  this->dataPtr->inertial = _inertial;
  return _inertial.MassMatrix().IsValid();
}

/////////////////////////////////////////////////
ignition::math::Pose3d Link::PoseInFrame(const std::string &_frame) const
{
  return poseInFrame(
      this->Name(),
      _frame.empty() ? this->PoseFrame() : _frame,
      *this->dataPtr->frameGraph);
}

/////////////////////////////////////////////////
const ignition::math::Pose3d &Link::Pose() const
{
  return this->dataPtr->pose;
}

/////////////////////////////////////////////////
const std::string &Link::PoseFrame() const
{
  return this->dataPtr->poseFrame;
}

/////////////////////////////////////////////////
bool Link::SetPose(const ignition::math::Pose3d &_pose)
{
  const ignition::math::graph::VertexRef_M<PoseWithFrameName> frameVertices =
      this->dataPtr->frameGraph->Vertices(this->PoseFrame());

  if (frameVertices.size() != 1)
    return false;

  using Matrix4d = ignition::math::Matrix4d;
  ignition::math::graph::DirectedEdge<Matrix4d> &edgeFromParent =
        this->dataPtr->frameGraph->EdgeFromVertices(
            frameVertices.begin()->first, this->dataPtr->frameVertexId);
  if (edgeFromParent.Id() ==
      ignition::math::graph::DirectedEdge<Matrix4d>::NullEdge.Id())
    return false;

  ignition::math::graph::DirectedEdge<Matrix4d> &edgeToParent =
        this->dataPtr->frameGraph->EdgeFromVertices(
            this->dataPtr->frameVertexId, frameVertices.begin()->first);
  if (edgeToParent.Id() ==
      ignition::math::graph::DirectedEdge<Matrix4d>::NullEdge.Id())
    return false;

  this->dataPtr->pose = _pose;
  // Store the pose data in the frame graph vertex
  this->dataPtr->frameGraph->VertexFromId(
      this->dataPtr->frameVertexId).Data().first = _pose;
  // Update the edges
  edgeFromParent.Data() = Matrix4d(_pose);
  edgeToParent.Data() = Matrix4d(_pose.Inverse());

  return true;
}

/////////////////////////////////////////////////
bool Link::SetPoseFrame(const std::string &_frame)
{
  if (_frame.empty())
    return false;

  // Store the pose data in the frame graph vertex
  this->dataPtr->frameGraph->VertexFromId(
      this->dataPtr->frameVertexId).Data().second = _frame;
  this->dataPtr->poseFrame = _frame;
  return true;
}

/////////////////////////////////////////////////
const Visual *Link::VisualByName(const std::string &_name) const
{
  for (auto const &v : this->dataPtr->visuals)
  {
    if (v.Name() == _name)
    {
      return &v;
    }
  }
  return nullptr;
}

/////////////////////////////////////////////////
const Collision *Link::CollisionByName(const std::string &_name) const
{
  for (auto const &c : this->dataPtr->collisions)
  {
    if (c.Name() == _name)
    {
      return &c;
    }
  }
  return nullptr;
}

/////////////////////////////////////////////////
sdf::ElementPtr Link::Element() const
{
  return this->dataPtr->sdf;
}
