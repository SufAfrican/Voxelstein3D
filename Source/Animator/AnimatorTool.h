#pragma once

class AnimatorTool
{
public:
  virtual ~AnimatorTool() {}

  /// Updates tool
  virtual void Run(float fDeltaTime) = 0;
};
