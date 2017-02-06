#ifndef __VDS_CORE_TARGETVER_H_
#define __VDS_CORE_TARGETVER_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SDKDDKVer.h>
#include <windows.h>
#include <Shlobj.h>

#pragma comment(lib, "Shell32.lib")

#else

#include <sys/types.h>

#endif

#include "stddef.h"

#include <vector>
#include <list>
#include <thread>
#include <future>
#include <chrono>

#endif// __VDS_CORE_TARGETVER_H_