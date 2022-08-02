#ifndef kqnetcommon_
#define kqnetcommon_

#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include "../asio-1.20.0/include/asio.hpp"
#include "../asio-1.20.0/include/asio/ts/buffer.hpp"
#include "../asio-1.20.0/include/asio/ts/internet.hpp"

#endif