#pragma once
// scmbot's own prelude for a bot's sources (on unless scmbot.json says "msvc": false): what a BWAPI bot written for
// Visual Studio gets without asking. MSVC's library pulls these headers in by itself; libc++ does not.
#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <thread>

// The "secure" C library calls MSVC has and others do not, both forms (the same as BWAPI's own build has).
#include "msvc-crt.h"

// A temporary passed where MSVC lets it bind to a non-const reference (scmbot rewrites such a call to use this).
template <class T> inline T &scmbot_lvalue(T &&t) { return t; }
