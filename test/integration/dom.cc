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

#include "sdf/sdf.hh"
#include "test_config.h"

/////////////////////////////////////////////////
TEST(DOM, ParseBlank)
{
  boost::filesystem::path path = PROJECT_SOURCE_PATH;
  path = path / "test" / "data" / "blank.sdf";

  // Parse the SDF file, and get a Sdf object.
  sdf::Sdf sdf = sdf::parse(path.string());

  EXPECT_EQ(sdf.version(), "1.4");
}

/////////////////////////////////////////////////
TEST(DOM, ParseWorldBlank)
{
  boost::filesystem::path path = PROJECT_SOURCE_PATH;
  path = path / "test" / "data" / "blank_world.sdf";

  // Parse the SDF file, and get a Sdf object.
  sdf::Sdf sdf = sdf::parse(path.string());

  EXPECT_EQ(sdf.world_size(), 1u);

  EXPECT_EQ(sdf.world(0).name(), "default");
}
