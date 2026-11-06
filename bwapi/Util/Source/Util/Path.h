#pragma once

#include <filesystem>

namespace Util {

namespace FS = std::filesystem;  // scm-sim: std::experimental on MSVC 2017

using Path = FS::path;
using FS::create_directories;

} // Util
