#include "StdAfx.h"

namespace Config
{
  std::map<std::string, unsigned int> g_VariableNameMap;

  void BuildVariableNameMap(void)
  {
    #define DEFINECONFIG_FLOAT(name, value, comment) g_VariableNameMap[#name] = __COUNTER__;
    #define DEFINECONFIG_INT(name, value, comment) g_VariableNameMap[#name] = __COUNTER__;
    #define DEFINECONFIG_BOOL(name, value, comment) g_VariableNameMap[#name] = __COUNTER__;
    #define DEFINECONFIG_VECTOR3D(name, value, comment) g_VariableNameMap[#name] = __COUNTER__;
    #define DEFINECONFIG_ROT3D(name, value, comment) g_VariableNameMap[#name] = __COUNTER__;
    #define DEFINECONFIG_HEX(name, value, comment) g_VariableNameMap[#name] = __COUNTER__;
    #include "Config_Vars.h"
    #undef DEFINECONFIG_FLOAT
    #undef DEFINECONFIG_INT
    #undef DEFINECONFIG_BOOL
    #undef DEFINECONFIG_VECTOR3D
    #undef DEFINECONFIG_ROT3D
    #undef DEFINECONFIG_HEX
  }
}