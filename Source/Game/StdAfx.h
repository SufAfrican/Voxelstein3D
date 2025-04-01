#pragma once

// disable warning: possible loss of data
#pragma warning(disable: 4267)
// disable warning: possible loss of data
#pragma warning(disable: 4244)

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
#include "Engine/Core/Collision.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/StringUtils.h"
#include "Engine/Core/SmartPointer.h"
#include "Engine/Sound/SoundManager.h"
#include "Engine/Sound/Sound.h"
#include "Engine/MemoryLeakFinder.h"
#include "Engine/Config.h"
#include "Engine/Voxlap.h"
#include "Engine/Sprite.h"
#include "Engine/AnimatedSprite.h"
#include "Engine/AnimationFile.h"
#include "Engine/PhysicsSolver.h"
#include "Game/Game.h"
