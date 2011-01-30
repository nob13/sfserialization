#pragma once

/**
 * @file
 * Types needed for sfserialization library
 */

#include <set>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include "winsupport.h"

#ifndef WIN32
#include <stdint.h>
#endif

namespace sf {

typedef std::vector<char> ByteArrayBase; ///< In libschnee sf::ByteArray derives from std::vector<char>

}
