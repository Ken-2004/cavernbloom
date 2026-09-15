#pragma once

#include <filesystem>

namespace cavernbloom {

// Resolve the running executable through the OS, independently of the working directory.
[[nodiscard]] std::filesystem::path executableDirectory();

} // namespace cavernbloom
