/* stdafx.h
 * Copyright 2015 Marco Freudenberger.
 * 
 * This file is part of mfimage library.
 * 
 * Design and implementation by:
 * --------------------------------------------------
 * - Marco Freudenberger (Marco.Freudenberger@gmx.de)
 *
 */
 
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
	/* WINDOWS */
#   include "win/targetver.h"
#   define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#else
	/* UNIXOID */
#endif

/* OS-System independant */
#include <cstring>
#include <cstdint>
#include <climits>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>

// TODO: reference additional headers your program requires here
