/*
 * Copyright 2013 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <boost/filesystem.hpp>

#include "sdf/Console.hh"

using namespace sdf;

Console *Console::myself = NULL;

//////////////////////////////////////////////////
Console::Console()
{
  this->msgStream = &std::cout;
  this->errStream = &std::cerr;

  char* home;
  if (!(home = getenv("HOME")))
    sdfwarn << "Missing HOME environment variable; "
      "no log file will be created.";

  boost::filesystem::path logPath(home);
  logPath = logPath / ".sdf/" / "sdf.log";
  this->logStream.open(logPath.string().c_str(), std::ios::out);
}

//////////////////////////////////////////////////
Console::~Console()
{
}

//////////////////////////////////////////////////
Console *Console::Instance()
{
  if (myself == NULL)
    myself = new Console();

  return myself;
}

//////////////////////////////////////////////////
void Console::SetQuiet(bool)
{
}

//////////////////////////////////////////////////
std::ostream &Console::ColorMsg(const std::string &lbl, int color)
{
  // if (**this->quietP)
  // return this->nullStream;
  // else
  // {
  *this->msgStream << "\033[1;" << color << "m" << lbl << "\033[0m ";
  this->logStream << lbl;
  return *this->msgStream;
  // }
}

//////////////////////////////////////////////////
std::ostream &Console::ColorErr(const std::string &lbl,
                                const std::string &file,
                                unsigned int line, int color)
{
  int index = file.find_last_of("/") + 1;

  *this->errStream << "\033[1;" << color << "m" << lbl << " [" <<
    file.substr(index , file.size() - index)<< ":" << line << "]\033[0m ";
  this->logStream << lbl << " [" << file.substr(index , file.size() - index) <<
     ":" << line;

  return *this->errStream;
}

//////////////////////////////////////////////////
std::ofstream &Console::Log()
{
  return this->logStream;
}
