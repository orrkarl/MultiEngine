#pragma once

#include "../general/predefs.h"

namespace nr
{

namespace utils
{

NR_SHARED_EXPORT GLenum fromNRType(Type type);

NR_SHARED_EXPORT Error fromGLError(GLenum err);

NR_SHARED_EXPORT Error getLastGLError();

NR_SHARED_EXPORT Error fromCLError(const cl_int& err);

NR_SHARED_EXPORT const string stringFromCLError(const cl_int& error);

}

}