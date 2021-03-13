#pragma once
#include <string>

namespace mpm {
template <class Tensor> std::string make_string(const Tensor &v);
}

// Detect Platform
#if defined(_WIN32)
/* Windows x86 or x64  */
#define MPM_PLATFORM_WINDOWS
#elif defined(__linux__)
#define MPM_PLATFORM_LINUX
#endif

#ifndef MPM_NO_DEBUG

// define debugbreak tools
#if defined(MPM_PLATFORM_WINDOWS)
#define MPM_DEBUGBREAK() __debugbreak()
#elif defined(MPM_PLATFORM_LINUX)
#include <signal.h>
#define MPM_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet"
#endif

#else

#define MPM_DEBUGBREAK()

#endif