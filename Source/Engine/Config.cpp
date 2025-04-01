#include "StdAfx.h"

namespace Config
{
// define variables
#define DEFINECONFIG_FLOAT(name, value, comment) float name = value; bool name##_configured = false;
#define DEFINECONFIG_INT(name, value, comment) int name = value; bool name##_configured = false;
#define DEFINECONFIG_BOOL(name, value, comment) bool name = value; bool name##_configured = false;
#define DEFINECONFIG_VECTOR3D(name, value, comment) Vector3D name = value; bool name##_configured = false;
#define DEFINECONFIG_ROT3D(name, value, comment) Vector3D name = value; bool name##_configured = false;
#define DEFINECONFIG_HEX(name, value, comment) int name = value; bool name##_configured = false;
#include "Config_Vars.h"
#undef DEFINECONFIG_FLOAT
#undef DEFINECONFIG_INT
#undef DEFINECONFIG_BOOL
#undef DEFINECONFIG_VECTOR3D
#undef DEFINECONFIG_ROT3D
#undef DEFINECONFIG_HEX

// variable saving
#define WRITECONFIG_FLOAT(name, value, comment) fprintf(pFile, "# %s\n%s = %g\n\n", comment, #name, name);
#define WRITECONFIG_INT(name, value, comment) fprintf(pFile, "# %s\n%s = %i\n\n", comment, #name, name);
#define WRITECONFIG_BOOL(name, value, comment) fprintf(pFile, "# %s\n%s = %i\n\n", comment, #name, name ? 1 : 0);
#define WRITECONFIG_VECTOR3D(name, value, comment) fprintf(pFile, "# %s\n%s = %g,%g,%g\n\n", comment, #name, name.x, name.y, name.z);
#define WRITECONFIG_ROT3D(name, value, comment) { Vector3D t = RadiansToDegrees(name); fprintf(pFile, "# %s\n%s = %g,%g,%g\n\n", comment, #name, t.x, t.y, t.z); }
#define WRITECONFIG_HEX(name, value, comment) fprintf(pFile, "# %s\n%s = %x\n\n", comment, #name, name);

// variable reading
#define READCONFIG_FLOAT(name, value, comment) if(sscanf(strLine.c_str(), #name"=%g", &name) == 1) { name##_configured = true; return true; }
#define READCONFIG_INT(name, value, comment) if(sscanf(strLine.c_str(), #name"=%i", &name) == 1) { name##_configured = true; return true; }
#define READCONFIG_BOOL(name, value, comment) {int bvar = 0; if(sscanf(strLine.c_str(), #name"=%i", &bvar) == 1) { name = bvar == 0 ? false : true; name##_configured = true; return true; }}
#define READCONFIG_VECTOR3D(name, value, comment) if(sscanf(strLine.c_str(), #name"=%g,%g,%g", &name.x, &name.y, &name.z) == 3) { name##_configured = true; return true; }
#define READCONFIG_ROT3D(name, value, comment) if(sscanf(strLine.c_str(), #name"=%g,%g,%g", &name.x, &name.y, &name.z) == 3) { name = DegreesToRadians(name); name##_configured = true; return true; }
#define READCONFIG_HEX(name, value, comment) if(sscanf(strLine.c_str(), #name"=%x", &name) == 1) { name##_configured = true; return true; }

  extern std::map<std::string, unsigned int> g_VariableNameMap;
  extern void BuildVariableNameMap(void);

  bool ParseVariableWithIndex(int i, const std::string &strLine)
  {
    switch(i)
    {
#define DEFINECONFIG_FLOAT(name, value, comment) case __COUNTER__: { READCONFIG_FLOAT(name, value, comment); break; }
#define DEFINECONFIG_INT(name, value, comment) case __COUNTER__: { READCONFIG_INT(name, value, comment); break; }
#define DEFINECONFIG_BOOL(name, value, comment) case __COUNTER__: { READCONFIG_BOOL(name, value, comment); break; }
#define DEFINECONFIG_VECTOR3D(name, value, comment) case __COUNTER__: { READCONFIG_VECTOR3D(name, value, comment); break; }
#define DEFINECONFIG_ROT3D(name, value, comment) case __COUNTER__: { READCONFIG_ROT3D(name, value, comment); break; }
#define DEFINECONFIG_HEX(name, value, comment) case __COUNTER__: { READCONFIG_HEX(name, value, comment); break; }
#include "Config_Vars.h"
#undef DEFINECONFIG_FLOAT
#undef DEFINECONFIG_INT
#undef DEFINECONFIG_BOOL
#undef DEFINECONFIG_VECTOR3D
#undef DEFINECONFIG_ROT3D
#undef DEFINECONFIG_HEX
      default:
        printf("ParseVariableIndex - Bad variable index %i in line \"%s\"", i, strLine.c_str());
        break;
    }
    return false;
  }

  static bool ParseVar(char *pBuffer)
  {
    // clean whitespace
    std::string strLine;
    while(pBuffer[0] != '\0')
    {
      if(pBuffer[0] != ' ' && pBuffer[0] != '\t' && pBuffer[0] != '\r' && pBuffer[0] != '\n')
      {
        char str[2] = { pBuffer[0], '\0' };
        strLine += str;
      }
      pBuffer++;
    }
    
    // ignore empty
    if(strLine.empty())
      return true;
    // ignore comment
    if(strLine[0] == '#')
      return true;

    std::string::size_type iCutPos = strLine.find('=');
    if(iCutPos != std::string::npos)
    {
      std::string strVar = strLine.substr(0,iCutPos);
      std::map<std::string, unsigned int>::iterator it = g_VariableNameMap.find(strVar);
      if(it != g_VariableNameMap.end())
      {
        return ParseVariableWithIndex(it->second, strLine);
      }
    }
    return false;
  }

  static void ErrorVariableNotConfigured(const char *strName)
  {
    printf("Variable %s is not in configuration file!\n", strName);
  }

  void Load(void)
  {
    FILE *pFile = fopen("config.ini", "rt");
    if(pFile == NULL)
    {
      printf("Cannot open config file for reading!\n");
      Save();
      return;
    }

    BuildVariableNameMap();

    unsigned int iLine = 0;
    while(!feof(pFile))
    {
      char pBuffer[1024];
      fgets(pBuffer, 1024, pFile);
      if(!ParseVar(pBuffer))
        printf("Parse error in config line %i: \"%s\"\n", iLine, pBuffer);
      iLine++;
    }
    fclose(pFile);

    // open for append
    pFile = fopen("config.ini", "at");
    if(pFile == NULL)
    {
      printf("Cannot open config file for writing!\n");
      return;
    }
// write all variables that didn't exist in the config file
#define START_IFNOT_CONFIGURED(name) if(!(name##_configured)) { ErrorVariableNotConfigured(#name); 
#define END_IFNOT_CONFIGURED() }
#define DEFINECONFIG_FLOAT(name, value, comment) START_IFNOT_CONFIGURED(name) WRITECONFIG_FLOAT(name, value, comment) END_IFNOT_CONFIGURED()
#define DEFINECONFIG_INT(name, value, comment) START_IFNOT_CONFIGURED(name) WRITECONFIG_INT(name, value, comment) END_IFNOT_CONFIGURED()
#define DEFINECONFIG_BOOL(name, value, comment) START_IFNOT_CONFIGURED(name) WRITECONFIG_BOOL(name, value, comment) END_IFNOT_CONFIGURED()
#define DEFINECONFIG_VECTOR3D(name, value, comment) START_IFNOT_CONFIGURED(name) WRITECONFIG_VECTOR3D(name, value, comment) END_IFNOT_CONFIGURED()
#define DEFINECONFIG_ROT3D(name, value, comment) START_IFNOT_CONFIGURED(name) WRITECONFIG_ROT3D(name, value, comment) END_IFNOT_CONFIGURED()
#define DEFINECONFIG_HEX(name, value, comment) START_IFNOT_CONFIGURED(name) WRITECONFIG_HEX(name, value, comment) END_IFNOT_CONFIGURED()
#include "Config_Vars.h"
#undef DEFINECONFIG_FLOAT
#undef DEFINECONFIG_INT
#undef DEFINECONFIG_BOOL
#undef DEFINECONFIG_VECTOR3D
#undef DEFINECONFIG_ROT3D
#undef DEFINECONFIG_HEX

    fclose(pFile);
  }

  void SaveUnconfigured(void)
  {
    FILE *pFile = fopen("config.ini", "at");
    if(pFile == NULL)
    {
      printf("Cannot open config file for writing!\n");
      return;
    }
  }

  void Save(void)
  {
    FILE *pFile = fopen("config.ini", "wt");
    if(pFile == NULL)
    {
      printf("Cannot open config file for writing!\n");
      return;
    }

// write variables
#define DEFINECONFIG_FLOAT(name, value, comment) WRITECONFIG_FLOAT(name, value, comment)
#define DEFINECONFIG_INT(name, value, comment) WRITECONFIG_INT(name, value, comment)
#define DEFINECONFIG_BOOL(name, value, comment) WRITECONFIG_BOOL(name, value, comment)
#define DEFINECONFIG_VECTOR3D(name, value, comment) WRITECONFIG_VECTOR3D(name, value, comment)
#define DEFINECONFIG_ROT3D(name, value, comment) WRITECONFIG_ROT3D(name, value, comment)
#define DEFINECONFIG_HEX(name, value, comment) WRITECONFIG_HEX(name, value, comment)
#include "Config_Vars.h"
#undef DEFINECONFIG_FLOAT
#undef DEFINECONFIG_INT
#undef DEFINECONFIG_BOOL
#undef DEFINECONFIG_VECTOR3D
#undef DEFINECONFIG_ROT3D
#undef DEFINECONFIG_HEX
    fclose(pFile);
  }
}