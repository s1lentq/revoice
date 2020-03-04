#pragma once

#include "revoice_shared.h"
#include "revoice_player.h"

bool Revoice_Load();
bool Revoice_Main_Init();
void Revoice_Main_Detach();

qboolean ClientConnect_Pre(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);
void ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax);
void CvarValue2_Pre(const edict_t* pEnt, int requestID, const char* cvarName, const char* cvarValue);
