/*
 * Copyright 2015-2017 Open Source Robotics Foundation
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

#include <iostream>
#include <gtest/gtest.h>
#include "sdf/sdf.hh"
#include "sdf/parser_urdf.hh"

#include "test_config.h"

/////////////////////////////////////////////////
std::string custom_exec(std::string _cmd)
{
#ifdef _WIN32
  FILE *pipe = _popen(_cmd.c_str(), "r");
#else
  FILE *pipe = popen(_cmd.c_str(), "r");
#endif

  if (!pipe)
  {
    return "ERROR";
  }

  char buffer[128];
  std::string result = "";

  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != NULL)
    {
      result += buffer;
    }
  }

#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif

  return result;
}

const std::string sdfString(
  "<?xml version='1.0'?>\n"
  "<sdf version='1.5'>\n"
  "    <model name='break_bot'>\n"
  "        <link name='root-link'>\n"
  "           <sensor type='gps' name='mysensor' />"
  "        </link>\n"
  "    </model>\n"
  "</sdf>");

const std::string pythonMeminfo("python "
  PROJECT_SOURCE_PATH "/tools/get_mem_info.py");

int getMemoryUsage()
{
  return std::stoi(custom_exec(pythonMeminfo));
}

//////////////////////////////////////////////////
TEST(ElementMemoryLeak, SDFCreateDestroy)
{
  // Initial memory usage
  int memoryLimit = getMemoryUsage();
  std::cout << "initial memory: " << memoryLimit << std::endl;

  // Allow 15x increase (based on testing with Ubuntu and OSX)
  memoryLimit *= 15;

  for (unsigned int i = 0; i < 50; ++i)
  {
    sdf::SDF modelSDF;
    modelSDF.SetFromString(sdfString);

    int memoryUsage = getMemoryUsage();
    EXPECT_LT(memoryUsage, memoryLimit);
  }

  std::cout << "  final memory: "
            << getMemoryUsage()
            << std::endl;
}
