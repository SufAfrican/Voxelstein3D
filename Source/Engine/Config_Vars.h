DEFINECONFIG_BOOL(Input_bMouseInverted, false, "Mouse y-axis invert")
DEFINECONFIG_FLOAT(Input_fMouseSensitivityX, 0.5f, "Mouse sensitivity (x-axis)")
DEFINECONFIG_FLOAT(Input_fMouseSensitivityY, 0.3f, "Mouse sensitivity (y-axis)")

DEFINECONFIG_FLOAT(Player_fMoveSpeed, 300.0f, "Player move speed");
DEFINECONFIG_FLOAT(Player_fJumpSpeed, 700.0f, "Player jump speed");
DEFINECONFIG_FLOAT(Player_fJumpDelay, 0.25f, "Player jump delay (seconds)");
DEFINECONFIG_FLOAT(Player_fGravity, 5000.0f, "Player gravity");
DEFINECONFIG_FLOAT(Player_fHeight, 50.0f, "Player height");
DEFINECONFIG_FLOAT(Player_fCrouchHeight, 30.0f, "Player height when crouching");
DEFINECONFIG_FLOAT(Player_fRadius, 16.0f, "Player radius (width)");
DEFINECONFIG_FLOAT(Player_fStepHeight, 10.0f, "Player step height (for stairs etc.)");
DEFINECONFIG_FLOAT(Player_fUseDistance, 100.0f, "How close player has to be when pressing USE");
DEFINECONFIG_FLOAT(Player_fFlashLightDistance, 70.0f, "Flash light distance");

DEFINECONFIG_BOOL(Pistol_bFiresByClick, true, "Fire requires clicking")
DEFINECONFIG_FLOAT(Pistol_fFireDelay, 0.4f, "Firing delay (seconds)")
DEFINECONFIG_FLOAT(Pistol_fFireDistance, 2000.0f, "Max distance for bullet to hit");
DEFINECONFIG_FLOAT(Pistol_fSwingingSpeed, 10.0f, "Walk-swinging speed (animation when player is walking)");
DEFINECONFIG_FLOAT(Pistol_fSwingingAmount, 0.5f, "Walk-swinging amount (animation when player is walking)");
DEFINECONFIG_INT(Pistol_AmmoInWeapon, 8, "Ammo in pistol pickup");
DEFINECONFIG_INT(Pistol_AmmoInClip, 8, "Ammo in pistol clip");
DEFINECONFIG_INT(Pistol_iDamageMin, 5, "Minimum damage");
DEFINECONFIG_INT(Pistol_iDamageMax, 10, "Maximum damage");

DEFINECONFIG_BOOL(Knife_bFiresByClick, true, "Fire requires clicking")
DEFINECONFIG_FLOAT(Knife_fStabDelay, 0.4f, "Firing delay (seconds)")
DEFINECONFIG_FLOAT(Knife_fHitDistance, 20.0f, "Max distance for stab to hit");
DEFINECONFIG_FLOAT(Knife_fSwingingSpeed, 10.0f, "Walk-swinging speed (animation when player is walking)");
DEFINECONFIG_FLOAT(Knife_fSwingingAmount, 0.5f, "Walk-swinging amount (animation when player is walking)");
DEFINECONFIG_INT(Knife_iDamageMin, 5, "Minimum damage");
DEFINECONFIG_INT(Knife_iDamageMax, 15, "Maximum damage");

DEFINECONFIG_BOOL(Chaingun_bFiresByClick, true, "Fire requires clicking")
DEFINECONFIG_FLOAT(Chaingun_fFireDelay, 0.4f, "Firing delay (seconds)")
DEFINECONFIG_FLOAT(Chaingun_fFireDistance, 2000.0f, "Max distance for bullet to hit");
DEFINECONFIG_FLOAT(Chaingun_fSwingingSpeed, 10.0f, "Walk-swinging speed (animation when player is walking)");
DEFINECONFIG_FLOAT(Chaingun_fSwingingAmount, 0.5f, "Walk-swinging amount (animation when player is walking)");
DEFINECONFIG_FLOAT(Chaingun_fBulletPenetrationDepth, 30.0f, "Depth that chaingun bullet cuts through");
DEFINECONFIG_FLOAT(Chaingun_fFireSpreadFactor, 0.1f, "How much chaingun bullets spread (accuracy)");
DEFINECONFIG_INT(Chaingun_AmmoInWeapon, 50, "Ammo in chaingun pickup");
DEFINECONFIG_INT(Chaingun_AmmoInClip, 100, "Ammo in chaingun ammo box");
DEFINECONFIG_INT(Chaingun_iDamageMin, 20, "Minimum damage");
DEFINECONFIG_INT(Chaingun_iDamageMax, 30, "Maximum damage");
DEFINECONFIG_INT(Chaingun_iDamageMinPenetrated, 7, "Minimum damage for penetrated bullet");
DEFINECONFIG_INT(Chaingun_iDamageMaxPenetrated, 13, "Maximum damage for penetrated bullet");

DEFINECONFIG_FLOAT(Door_fRotationSpeed, 0.1f, "Door rotation speed");
DEFINECONFIG_FLOAT(Door_fOpenDistance, 50.0f, "Door distance from player to be openable")

DEFINECONFIG_BOOL(Debug_bShowFPS, false, "Show FPS meter");
DEFINECONFIG_BOOL(Debug_bVisualizeBulletPenetration, false, "Show bullet penetrations");
DEFINECONFIG_BOOL(Debug_bVisualizeCollision, false, "Show entity collisions");
DEFINECONFIG_BOOL(Debug_bVisualizePhysicsParticles, false, "Show physics particles");
DEFINECONFIG_BOOL(Debug_bVisualizeEntityTree, false, "Show entity tree");
DEFINECONFIG_BOOL(Debug_bVisualizeEntitySize, false, "Show entity size");
DEFINECONFIG_BOOL(Debug_bShowIntro, true, "Show intro screens");

DEFINECONFIG_BOOL(Grenade_bFiresByClick, true, "Fire requires clicking");
DEFINECONFIG_FLOAT(Grenade_fFireDelay, 1.0f, "Firing delay (seconds)");
DEFINECONFIG_FLOAT(Grenade_fExplodeTime, 2.0f, "How long stays before exploding");
DEFINECONFIG_FLOAT(Grenade_fExplodeRadius, 20.0f, "Exploding radius");
DEFINECONFIG_FLOAT(Grenade_fThrowSpeed, 300.0f, "Throwing speed");
DEFINECONFIG_FLOAT(Grenade_fBounciness, 0.25f, "Bounciness factor for grenade");
DEFINECONFIG_FLOAT(Grenade_fDamageRadius, 150.0f, "Damage radius");
DEFINECONFIG_INT(Grenade_iDamageRays, 100, "Number of damage rays to spawn");
DEFINECONFIG_INT(Grenade_iDamageMin, 5, "Minimum damage of one ray");
DEFINECONFIG_INT(Grenade_iDamageMax, 15, "Maximum damage of one ray");

DEFINECONFIG_BOOL(StickyBomb_bFiresByClick, true, "Fire requires clicking");
DEFINECONFIG_FLOAT(StickyBomb_fFireDelay, 1.0f, "Firing delay (seconds)");
DEFINECONFIG_FLOAT(StickyBomb_fPlacingDistance, 40.0f, "Required distance to wall for placing bomb");
DEFINECONFIG_FLOAT(StickyBomb_fDetonationDelay, 4.0f, "How long the bomb ticks");
DEFINECONFIG_FLOAT(StickyBomb_fDamageRayRadius, 200.0f, "Radius for instant damage rays");
DEFINECONFIG_INT(StickyBomb_iDamageRayAmount, 10, "Number of damage rays to spawn");
DEFINECONFIG_INT(StickyBomb_iDamageRayMin, 20, "Minimum damage of one ray");
DEFINECONFIG_INT(StickyBomb_iDamageRayMax, 50, "Maximum damage of one ray");
DEFINECONFIG_INT(StickyBomb_iCloudsX, 10, "Number of clouds spawned horizontally");
DEFINECONFIG_INT(StickyBomb_iCloudsY, 5, "Number of clouds spawned vertically");
DEFINECONFIG_FLOAT(StickyBomb_fCloudRadiusMin, 4.9f, "Minimum cloud radius");
DEFINECONFIG_FLOAT(StickyBomb_fCloudRadiusMax, 5.1f, "Maximum cloud radius");
DEFINECONFIG_FLOAT(StickyBomb_fCloudEnergyDecay, 0.92f, "How much clouds preserve energy");
DEFINECONFIG_FLOAT(StickyBomb_fCloudLift, 0.025f, "How much clouds raise up");
DEFINECONFIG_FLOAT(StickyBomb_fCloudVelocity, 10.0f, "How fast clouds move");
DEFINECONFIG_FLOAT(StickyBomb_fCloudExpansion, 1.15f, "How quickly clouds expand");
DEFINECONFIG_FLOAT(StickyBomb_fCloudMaxRadius, 15.0f, "Radius after which can't expand anymore");
DEFINECONFIG_FLOAT(StickyBomb_fCloudStainRadius, 20.0f, "Radius for black stain from cloud");
DEFINECONFIG_FLOAT(StickyBomb_fCloudHoleEnergy, 0.25f, "How much energy cloud must have to make a hole");
DEFINECONFIG_INT(StickyBomb_iCloudHoleAmount, 5, "How many holes a single cloud creates");
DEFINECONFIG_FLOAT(StickyBomb_fCloudHoleRadius, 15.0f, "Radius of a single hole made by cloud");
DEFINECONFIG_FLOAT(StickyBomb_fCloudHoleSpread, 55.0f, "Distance to spread holes around");
DEFINECONFIG_FLOAT(StickyBomb_fCloudCollisionEnergyDecay, 0.8f, "How much clouds preserve energy on collision");
DEFINECONFIG_FLOAT(StickyBomb_fCloudEntityDamage, 100.0f, "Damage clouds give to entities");

DEFINECONFIG_INT(Guard_iHealth, 20, "Health");
DEFINECONFIG_FLOAT(Guard_fShootDelay, 1.5f, "How much time between shots");
DEFINECONFIG_FLOAT(Guard_fShootPrepareDelay, 0.75f, "How long to aim before shooting");
DEFINECONFIG_FLOAT(Guard_fMoveToNewPositionDelay, 5.0f, "How long to keep shooting from same position");
DEFINECONFIG_FLOAT(Guard_fShootInaccuracy, 0.15f, "How much to spread shots");
DEFINECONFIG_FLOAT(Guard_fDamagedDelay, 0.5f, "How long to wait after being damaged");

DEFINECONFIG_INT(Officer_iHealth, 20, "Health");
DEFINECONFIG_FLOAT(Officer_fShootDelay, 1.5f, "How much time between shots");
DEFINECONFIG_FLOAT(Officer_fShootPrepareDelay, 0.75f, "How long to aim before shooting");
DEFINECONFIG_FLOAT(Officer_fMoveToNewPositionDelay, 5.0f, "How long to keep shooting from same position");
DEFINECONFIG_FLOAT(Officer_fShootInaccuracy, 0.15f, "How much to spread shots");
DEFINECONFIG_FLOAT(Officer_fDamagedDelay, 0.5f, "How long to wait after being damaged");

DEFINECONFIG_HEX(Keybind_Forward, 0x11, "Move forward");
DEFINECONFIG_HEX(Keybind_Backward, 0x1F, "Move backward");
DEFINECONFIG_HEX(Keybind_Left, 0x1E, "Move left");
DEFINECONFIG_HEX(Keybind_Right, 0x20, "Move right");
DEFINECONFIG_HEX(Keybind_Jump, 0x39, "Jump");
DEFINECONFIG_HEX(Keybind_Crouch1, 0x1D, "Crouch");
DEFINECONFIG_HEX(Keybind_Crouch2, 0x2E, "Crouch alternative");
DEFINECONFIG_HEX(Keybind_Use, 0x12, "Use");
DEFINECONFIG_HEX(Keybind_Flashlight, 0x21, "Flashlight");
DEFINECONFIG_HEX(Keybind_MouseInvert, 0x15, "Mouse invert");
