#pragma once

#include "revoice_shared.h"
#include "revoice_player.h"

extern qboolean mm_ClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);
extern void mm_CvarValue2(const edict_t *pEnt, int requestID, const char *cvarName, const char *value);

extern bool Revoice_Main_Init();
