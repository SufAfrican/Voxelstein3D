#pragma once

/// Convert some/path/file.ext to some/path
inline std::string GetPath(const std::string &strFile)
{
  // find last \ char
  std::string::size_type pos = strFile.rfind('\\');
  if (pos != std::string::npos)
  {
    // split string from there
    return strFile.substr(0, pos);
  }

  // if still has .
  pos = strFile.rfind('.');
  if (pos != std::string::npos)
  {
    // return nothing
    return std::string();
  }

  return strFile;
}

/// Makes string lower case
inline void MakeLowerCase(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), tolower);
}

/// Returns string in lower case
inline std::string GetUpperCase(const std::string &strFile)
{
  std::string str = strFile;
  std::transform(str.begin(), str.end(), str.begin(), toupper);
  return str;
}

/// Makes string upper case
inline void MakeUpperCase(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), toupper);
}

/// Returns string in upper case
inline std::string GetLowerCase(const std::string &strFile)
{
  std::string str = strFile;
  std::transform(str.begin(), str.end(), str.begin(), tolower);
  return str;
}
