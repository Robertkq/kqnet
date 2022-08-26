#ifndef kqnetcommon_
#define kqnetcommon_

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif



#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <cstdint>


#include "..\kqlib\include\kqlib.h"
#include "..\asio-1.20.0\include\asio.hpp"
#include "..\asio-1.20.0\include\asio\ts\buffer.hpp"
#include "..\asio-1.20.0\include\asio\ts\internet.hpp"

#endif