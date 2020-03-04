// meta_api.cpp - minimal implementation of metamod's plugin interface

// This is intended to illustrate the (more or less) bare minimum code
// required for a valid metamod plugin, and is targeted at those who want
// to port existing HL/SDK DLL code to run as a metamod plugin.

/*
 * Copyright (c) 2001-2003 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#include "precompiled.h"

// Description of plugin
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION,	// ifvers
	"Revoice",	// name
	APP_VERSION,	// version
	APP_COMMIT_DATE,	// date
	"The Legion",	// author
	"",	// url
	"REVOICE",	// logtag, all caps please
	PT_STARTUP,	// (when) loadable
	PT_NEVER,	// (when) unloadable
};

// Global vars from metamod:
meta_globals_t *gpMetaGlobals;		// metamod globals
gamedll_funcs_t *gpGamedllFuncs;	// gameDLL function tables
mutil_funcs_t *gpMetaUtilFuncs;		// metamod utility functions

enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

// Metamod requesting info about this plugin:
//  ifvers			(given) interface_version metamod is using
//  pPlugInfo		(requested) struct with info about plugin
//  pMetaUtilFuncs	(given) table of utility functions provided by metamod
C_DLLEXPORT int Meta_Query(char * /*ifvers */, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs)
{
	// Give metamod our plugin_info struct
	*pPlugInfo = &Plugin_info;

	// Get metamod utility function table.
	gpMetaUtilFuncs = pMetaUtilFuncs;

	return TRUE;
}

DLL_FUNCTIONS g_DLLFuncTable;
C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable)
	{
		UTIL_LogPrintf("GetEntityAPI2 called with null pFunctionTable");
		return FALSE;
	}

	if (*interfaceVersion != INTERFACE_VERSION)
	{
		UTIL_LogPrintf("GetEntityAPI2 version mismatch; requested=%d ours=%d", *interfaceVersion, INTERFACE_VERSION);
		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}

	memset(&g_DLLFuncTable, 0, sizeof(DLL_FUNCTIONS));

	g_DLLFuncTable.pfnClientConnect = ClientConnect_Pre;

	memcpy(pFunctionTable, &g_DLLFuncTable, sizeof(DLL_FUNCTIONS));

	return TRUE;
}

DLL_FUNCTIONS g_DLLFuncTable_Post;
C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable)
	{
		ALERT(at_logged, "%s called with null pFunctionTable", __FUNCTION__);
		return FALSE;
	}

	if (*interfaceVersion != INTERFACE_VERSION)
	{
		ALERT(at_logged, "%s version mismatch; requested=%d ours=%d", __FUNCTION__, *interfaceVersion, INTERFACE_VERSION);
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}

	memset(&g_DLLFuncTable_Post, 0, sizeof(DLL_FUNCTIONS));

	g_DLLFuncTable_Post.pfnServerActivate = ServerActivate_Post;

	memcpy(pFunctionTable, &g_DLLFuncTable_Post, sizeof(DLL_FUNCTIONS));

	return TRUE;
}

NEW_DLL_FUNCTIONS g_NewDLLFuncTable;
C_DLLEXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pNewFunctionTable, int* interfaceVersion)
{
	if (!pNewFunctionTable)
	{
		UTIL_LogPrintf("GetNewDLLFunctions called with null pNewFunctionTable");
		return(FALSE);
	}

	if (*interfaceVersion != NEW_DLL_FUNCTIONS_VERSION)
	{
		UTIL_LogPrintf("GetNewDLLFunctions version mismatch; requested=%d ours=%d", *interfaceVersion, NEW_DLL_FUNCTIONS_VERSION);
		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return FALSE;
	}

	memset(&g_NewDLLFuncTable, 0, sizeof(NEW_DLL_FUNCTIONS));

	g_NewDLLFuncTable.pfnCvarValue2 = CvarValue2_Pre;

	memcpy(pNewFunctionTable, &g_NewDLLFuncTable, sizeof(NEW_DLL_FUNCTIONS));
	return TRUE;
}

// Metamod attaching plugin to the server.
//  now				(given) current phase, ie during map, during changelevel, or at startup
//  pFunctionTable	(requested) table of function tables this plugin catches
//  pMGlobals		(given) global vars from metamod
//  pGamedllFuncs	(given) copy of function tables from game dll
META_FUNCTIONS gMetaFunctionTable;
C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME /* now */, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs)
{
	if(!pMGlobals) {
		LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
		return FALSE;
	}

	gpMetaGlobals = pMGlobals;

	if(!pFunctionTable) {
		LOG_ERROR(PLID, "Meta_Attach called with null pFunctionTable");
		return FALSE;
	}

	gpGamedllFuncs = pGamedllFuncs;

	gMetaFunctionTable.pfnGetEntityAPI2 = GetEntityAPI2;
	gMetaFunctionTable.pfnGetEntityAPI2_Post = GetEntityAPI2_Post;
	gMetaFunctionTable.pfnGetNewDLLFunctions = GetNewDLLFunctions;

	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));

	return Revoice_Load() ? TRUE : FALSE;
}

// Metamod detaching plugin from the server.
// now		(given) current phase, ie during map, etc
// reason	(given) why detaching (refresh, console unload, forced unload, etc)
C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME /* now */, PL_UNLOAD_REASON /* reason */)
{
	Revoice_Main_Detach();
	return TRUE;
}

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we
// do some setup operations here.
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals)
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}
