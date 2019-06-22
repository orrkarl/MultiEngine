#include <base/Platform.h>

#include <algorithm>

namespace nr
{

Platform::makeDefault(Platform provided)
{
    defaultPlatform = provided;
}

Platform::getDefault()
{
    return defaultPlatform;
}

std::vector<Platform> Platform::getAvailablePlatforms(cl_status* err)
{
    cl_uint platformCount;
    clGetPlatformIDs(0, nullptr, &platformCount);

    std::vector<cl_platform_id> platformIDs(platformCount);
    clGetPlatformIDs(platformCount, &platformIDs.front(), nullptr);

    std::vector<Platform> ret(platformCount);
    std::transform(platformIDs.cbegin(), platformIDs.cend(), ret.begin(), Platform::Platform);

    return ret;
}

Platform::Platform()
    : Wrapped()
{
}

Platform::Platform(const cl_platform_id& platform, const NRbool retain = false)
    : Wrapped(Platform, retain)
{
}

Platform::Platform(const Platform& other)
    : Wrapped(other)
{
}

Platform::Platform(Platform&& other)
    : Wrapped(other)
{
}

Platform& Platform::operator=(const Platform& other)
{
    return Wrapped::operator=(other);
}

Platform& Platform::operator=(Platform&& other)
{
    return Wrapped::operator=(other);
}

Platform::operator cl_platform_id() const 
{
    return object;
}

std::vector<Device> Platform::getDevicesByType(cl_device_type type, cl_status* err)
{
    cl_uint deviceCount;
    clGetDeviceIDs(object, type, 0, nullptr, &deviceCount);

    std::vector<cl_device_id> deviceIDs(deviceCount);
    clGetDeviceIDs(object, type, deviceCount, &deviceIDs.front(), nullptr);

    std::vector<Device> ret(deviceCount);
    std::transform(platformIDs.cbegin(), platformIDs.cend(), ret.begin(), Device::Device);

    return ret;
}

}