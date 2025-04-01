#pragma once

#include "Game/Entities/SoldierEntity.h"

/// Officer enemy entity (white uniform)
class OfficerEntity : public SoldierEntity
{
public:
  OfficerEntity();

  virtual void OnCreateRagdoll(void);
};