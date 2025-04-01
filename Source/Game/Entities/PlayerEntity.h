#pragma once

#include "Game/Entities/CharacterEntity.h"

class Weapon;

/// Player entity
class PlayerEntity : public CharacterEntity
{
public:
  PlayerEntity();
  ~PlayerEntity();

  /// Updates the entity state
  void Tick(void);

  /// Entity was damaged
  virtual void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager);

public:
  /// Returns currently selected weapon, or NULL if none is selected
  Weapon *GetSelectedWeapon(void);

  /// Weapon number in weapon array
  enum WeaponNumber
  {
    Weapon_Knife = 0,
    Weapon_Pistol = 1,
    Weapon_Chaingun = 2,
    Weapon_Grenade = 3,
    Weapon_StickyBomb = 4,
    Weapon_Num = 5
  };

  /// Gives player weapon and or ammo
  void GiveWeaponItem(WeaponNumber iWeapon, bool bGiveWeapon, int iGiveAmmo);

  /// Gives player flashlight
  inline void GiveFlashlight(void) { m_bHasFlashlight = true; }
  /// Returns true if player has flashlight
  inline bool HasFlashlight(void) { return m_bHasFlashlight; }

  /// Gameplay stats
  struct Stats
  {
    Stats();
    /// Kills
    int iKills;
    /// Treasure pickups
    int iTreasures;
    /// Item pickups
    int iItems;
  };

  /// Returns stats for current level
  inline Stats &GetStats(void) { return m_Stats; }

public:
  /// Tick where last found treasure, for animating face
  unsigned int m_iLastFoundTreasure;
  /// Tick where last found chaingun, for animating face
  unsigned int m_iLastFoundChaingun;
  /// How many ticks has been holding trigger, for animating face
  unsigned int m_iHoldingTriggerTime;

public:
  /// Tick player last jumped in
  unsigned int m_iLastJumpTick;
  /// Weapons
  std::vector<Weapon *> m_Weapons;
  /// Ammo for weapons that haven't been picked up yet
  std::vector<int> m_CollectedAmmo;
  /// Currently selected weapon
  unsigned int m_iSelectedWeapon;
  /// Amount of damaged received in last tick
  int m_iLastTickDamage;
  /// Has flashlight
  bool m_bHasFlashlight;
  /// Stats for current level
  Stats m_Stats;
};