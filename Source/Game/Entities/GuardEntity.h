#pragma once

#include "Game/Entities/SoldierEntity.h"

/// Guard enemy entity (brown uniform)
class GuardEntity : public SoldierEntity
{
public:
  GuardEntity();

  virtual void OnCreateRagdoll(void);
};