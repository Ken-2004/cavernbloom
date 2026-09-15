#include "RuntimePaths.hpp"

#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <cstdint>
#endif

namespace cavernbloom {

std::filesystem::path executableDirectory()
{
#if defined(_WIN32)
    std::vector<wchar_t> buffer(256);
    while (buffer.size() <= 32768) {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                "Cannot locate the CavernBloom executable");
        }
        if (length < buffer.size()) {
            return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
    throw std::runtime_error("The CavernBloom executable path exceeds the supported Windows path length.");
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    (void)_NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        throw std::runtime_error("Cannot locate the CavernBloom executable.");
    }
    return std::filesystem::canonical(buffer.data()).parent_path();
#elif defined(__linux__)
    return std::filesystem::canonical("/proc/self/exe").parent_path();
#else
    throw std::runtime_error("Executable directory discovery is not implemented for this platform.");
#endif
}

} // namespace cavernbloom
