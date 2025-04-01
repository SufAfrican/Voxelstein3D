#pragma once

/// Class that enables writing a .dmp file when the application crashes through unhandled exception
class Minidump
{
public:
  static void Activate(void);
  static void Deactivate(void);
};