#pragma once

// disable warning: possible loss of data
#pragma warning(disable: 4267)
// disable warning: possible loss of data
#pragma warning(disable: 4244)

// Windows XP SP2
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#define NTDDI_VERSION NTDDI_WINXPSP2
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <list>
#include <vector>
#include <map>

#include "Engine/Core/Misc.h"
#include "Engine/Core/Vector2D.h"
#include "Engine/Core/Vector3D.h"
#include "Engine/Core/Matrix44.h"
#include "Engine/Core/Quaternion.h"
#include "Engine/Core/Interpolation.h"
#include "Engine/Core/StringUtils.h"
#include "Engine/Core/Time.h"
#include "Engine/MemoryLeakFinder.h"
#include "Engine/Config.h"
#include "Engine/Sprite.h"
#include "Engine/Voxlap.h"
#include "Engine/AnimationFile.h"
#include "Engine/AnimatedSprite.h"
#include "Engine/PhysicsSolver.h"

#include "Animator/Drawing.h"
#include "Animator/Animator.h"
#include "Animator/AnimatorTool.h"
#include "Animator/AnimWindow.h"