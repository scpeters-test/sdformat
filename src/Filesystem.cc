/*
 * Copyright 2017 Open Source Robotics Foundation
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

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winnt.h>
#endif

#include "sdf/Filesystem.hh"

namespace sdf
{
namespace filesystem
{
#ifndef _WIN32
//////////////////////////////////////////////////
bool exists(const std::string &_path)
{
  struct stat path_stat;

  return ::stat(_path.c_str(), &path_stat) == 0;
}

//////////////////////////////////////////////////
bool is_directory(const std::string &_path)
{
  struct stat path_stat;

  if (::stat(_path.c_str(), &path_stat) != 0)
  {
    return false;
  }

  return S_ISDIR(path_stat.st_mode);
}

//////////////////////////////////////////////////
bool create_directory(const std::string &_path)
{
  return ::mkdir(_path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0;
}

//////////////////////////////////////////////////
std::string const separator(const std::string &_p)
{
  return _p + "/";
}

//////////////////////////////////////////////////
std::string current_path()
{
  std::string cur;

  for (int32_t path_max = 128;; path_max *= 2)  // loop 'til buffer large enough
  {
    std::vector<char> buf(path_max);

    if (::getcwd(buf.data(), buf.size()) == 0)
    {
      if (errno != ERANGE)
      {
        break;
      }
    }
    else
    {
      cur = std::string(buf.data());
      break;
    }
  }
  return cur;
}
#else  // Windows
//////////////////////////////////////////////////
static bool not_found_error(int errval)
{
  return errval == ERROR_FILE_NOT_FOUND
    || errval == ERROR_PATH_NOT_FOUND
    || errval == ERROR_INVALID_NAME  // "tools/jam/src/:sys:stat.h", "//foo"
    || errval == ERROR_INVALID_DRIVE  // USB card reader with no card inserted
    || errval == ERROR_NOT_READY  // CD/DVD drive with no disc inserted
    || errval == ERROR_INVALID_PARAMETER  // ":sys:stat.h"
    || errval == ERROR_BAD_PATHNAME  // "//nosuch" on Win64
    || errval == ERROR_BAD_NETPATH;  // "//nosuch" on Win32
}

//////////////////////////////////////////////////
static bool process_status_failure()
{
  int errval(::GetLastError());

  if (not_found_error(errval))
  {
    return false;
  }
  else if ((errval == ERROR_SHARING_VIOLATION))
  {
    return true;  // odd, but this is what boost does
  }
  return false;
}

struct handle_wrapper
{
  HANDLE handle;
  explicit handle_wrapper(HANDLE h)
    : handle(h) {}
  ~handle_wrapper()
  {
    if (handle != INVALID_HANDLE_VALUE)
    {
      ::CloseHandle(handle);
    }
  }
};

//  REPARSE_DATA_BUFFER related definitions are found in ntifs.h, which is part
//  of the Windows Device Driver Kit. Since that's inconvenient, the definitions
//  are provided here. See http://msdn.microsoft.com/en-us/library/ms791514.aspx

typedef struct _REPARSE_DATA_BUFFER {
  ULONG  ReparseTag;
  USHORT  ReparseDataLength;
  USHORT  Reserved;
  union {
    struct {
      USHORT  SubstituteNameOffset;
      USHORT  SubstituteNameLength;
      USHORT  PrintNameOffset;
      USHORT  PrintNameLength;
      ULONG  Flags;
      WCHAR  PathBuffer[1];
  /*  Example of distinction between substitute and print names:
        mklink /d ldrive c:\
        SubstituteName: c:\\??\
        PrintName: c:\
  */
    } SymbolicLinkReparseBuffer;
    struct {
      USHORT  SubstituteNameOffset;
      USHORT  SubstituteNameLength;
      USHORT  PrintNameOffset;
      USHORT  PrintNameLength;
      WCHAR  PathBuffer[1];
    } MountPointReparseBuffer;
    struct {
      UCHAR  DataBuffer[1];
    } GenericReparseBuffer;
  };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

//////////////////////////////////////////////////
HANDLE create_file_handle(const std::string &_path, DWORD _dwDesiredAccess,
                          DWORD _dwShareMode,
                          LPSECURITY_ATTRIBUTES _lpSecurityAttributes,
                          DWORD _dwCreationDisposition,
                          DWORD _dwFlagsAndAttributes,
                          HANDLE _hTemplateFile)
{
  return ::CreateFileW(_path.c_str(), _dwDesiredAccess, _dwShareMode,
                       _lpSecurityAttributes, _dwCreationDisposition,
                       _dwFlagsAndAttributes,
                       _hTemplateFile);
}

#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE  (16 * 1024)
#endif

//////////////////////////////////////////////////
bool is_reparse_point_a_symlink(const std::string &_path)
{
  handle_wrapper h(create_file_handle(_path, FILE_READ_EA,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                    nullptr));
  if (h.handle == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  std::vector<char> buf(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

  // Query the reparse data
  DWORD dwRetLen;
  BOOL result = ::DeviceIoControl(h.handle, FSCTL_GET_REPARSE_POINT, nullptr, 0,
                                  buf.data(), buf.size(), &dwRetLen, nullptr);
  if (!result)
  {
    return false;
  }

  return reinterpret_cast<const REPARSE_DATA_BUFFER*>(buf)->ReparseTag
    == IO_REPARSE_TAG_SYMLINK
    // Issue 9016 asked that NTFS directory junctions be recognized as
    // directories.  That is equivalent to recognizing them as symlinks, and
    // then the normal symlink mechanism will take care of recognizing them as
    // directories.
    //
    // Directory junctions are very similar to symlinks, but have some
    // performance and other advantages over symlinks. They can be created from
    // the command line with "mklink /j junction-name target-path".
    || reinterpret_cast<const REPARSE_DATA_BUFFER*>(buf)->ReparseTag
    == IO_REPARSE_TAG_MOUNT_POINT;  // aka "directory junction" or "junction"
}

//////////////////////////////////////////////////
bool exists(const std::string &_path)
{
  DWORD attr(::GetFileAttributesW(_path.c_str()));
  if (attr == 0xFFFFFFFF)
  {
    return process_status_failure();
  }

  //  reparse point handling;
  //    since GetFileAttributesW does not resolve symlinks, try to open a file
  //    handle to discover if the file exists
  if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
  {
    handle_wrapper h(
      create_file_handle(
        _path.c_str(),
        0,  // dwDesiredAccess; attributes only
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,  // lpSecurityAttributes
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0));  // hTemplateFile
    if (h.handle == INVALID_HANDLE_VALUE)
    {
      return process_status_failure();
    }

    if (!is_reparse_point_a_symlink(_path))
    {
      return true;
    }
  }

  return true;
}

//////////////////////////////////////////////////
bool is_directory(const std::string &_path)
{
  DWORD attr(::GetFileAttributesW(_path.c_str()));
  if (attr == 0xFFFFFFFF)
  {
    return process_status_failure();
  }

  //  reparse point handling;
  //    since GetFileAttributesW does not resolve symlinks, try to open a file
  //    handle to discover if the file exists
  if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
  {
    handle_wrapper h(
      create_file_handle(
        _path.c_str(),
        0,  // dwDesiredAccess; attributes only
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,  // lpSecurityAttributes
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0));  // hTemplateFile

    if (h.handle == INVALID_HANDLE_VALUE)
    {
      return process_status_failure();
    }

    if (!is_reparse_point_a_symlink(_path))
    {
      return true;
    }
  }

  return attr & FILE_ATTRIBUTE_DIRECTORY;
}

//////////////////////////////////////////////////
bool create_directory(const std::string &_path)
{
  return ::CreateDirectoryW(_path.c_str(), 0) != 0;
}

//////////////////////////////////////////////////
std::string const separator(const std::string &_p)
{
  return _p + "\\";
}

//////////////////////////////////////////////////
std::string current_path()
{
  DWORD sz;
  if ((sz = ::GetCurrentDirectoryW(0, nullptr)) == 0)
  {
    sz = 1;
  }

  std::vector<char> buf(sz);

  if (::GetCurrentDirectoryW(sz, buf.data()) == 0)
  {
    // error
    return std::string("");
  }
  else
  {
    std::string ret = std::string(buf.data());
    return ret;
  }
}

#endif
}
}
