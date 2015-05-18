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

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <map>
#include "sdf/sdf.hh"

#include "test_config.h"

const std::string SDF_FIXED_JNT = std::string(PROJECT_SOURCE_PATH)
  + "/test/integration/fixed_joint_reduction.urdf";

const std::string SDF_FIXED_JNT_NO_LUMPING = std::string(PROJECT_SOURCE_PATH)
  + "/test/integration/fixed_joint_reduction_disabled.urdf";

/////////////////////////////////////////////////
bool findJointInModel(std::string desired_joint_name, sdf::SDFPtr robot)
{
  bool found = false;
  sdf::ElementPtr model = robot->Root()->GetElement("model");
  for (sdf::ElementPtr joint = model->GetElement("joint"); joint;
       joint = joint->GetNextElement("joint"))
  {
    std::string jointName = joint->Get<std::string>("name");
    if (jointName == desired_joint_name)
    {
        found = true;
        break;
    }
  }

  return found;
}

/////////////////////////////////////////////////
TEST(SDFParser, DisableFixedJointReductionTest)
{
  char *pathCStr = getenv("SDF_PATH");
  boost::filesystem::path path = PROJECT_SOURCE_PATH;
  path = path / "sdf" / SDF_VERSION;
  setenv("SDF_PATH", path.string().c_str(), 1);

  sdf::SDFPtr robot(new sdf::SDF());
  sdf::init(robot);
  ASSERT_TRUE(sdf::readFile(SDF_FIXED_JNT, robot));

  sdf::SDFPtr robot_disable_lumping(new sdf::SDF());
  sdf::init(robot_disable_lumping);
  ASSERT_TRUE(sdf::readFile(SDF_FIXED_JNT_NO_LUMPING, robot_disable_lumping));

  if (pathCStr)
  {
    setenv("SDF_PATH", pathCStr, 1);
  }
  else
  {
    unsetenv("SDF_PATH");
  }

  ASSERT_FALSE(findJointInModel("joint12a", robot));
  ASSERT_FALSE(findJointInModel("joint23a", robot));

  ASSERT_TRUE(findJointInModel("joint12a", robot_disable_lumping));
  ASSERT_FALSE(findJointInModel("joint23a", robot_disable_lumping));
}
