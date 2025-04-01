#pragma once

namespace Config
{
  /// Loads config file
  void Load(void);
  /// Rewrites the whole config file with current settings
  void Save(void);

// declare config variables
#define DEFINECONFIG_FLOAT(name, value, comment) extern float name;
#define DEFINECONFIG_INT(name, value, comment) extern int name;
#define DEFINECONFIG_BOOL(name, value, comment) extern bool name;
#define DEFINECONFIG_VECTOR3D(name, value, comment) extern Vector3D name;
#define DEFINECONFIG_ROT3D(name, value, comment) extern Vector3D name;
#define DEFINECONFIG_HEX(name, value, comment) extern int name;
#include "Config_Vars.h"
#undef DEFINECONFIG_FLOAT
#undef DEFINECONFIG_INT
#undef DEFINECONFIG_BOOL
#undef DEFINECONFIG_VECTOR3D
#undef DEFINECONFIG_ROT3D
#undef DEFINECONFIG_HEX
};