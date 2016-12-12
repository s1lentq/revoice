#include "precompiled.h"

IReunionApi* g_ReunionApi;

bool Revoice_ReunionApi_Init()
{
	g_ReunionApi = (IReunionApi *)g_RehldsFuncs->GetPluginApi("reunion");

	if (g_ReunionApi == NULL) {
		LCPrintf(true, "Failed to locate Reunion api\n");
		return false;
	}

	if (g_ReunionApi->version_major != REUNION_API_VERSION_MAJOR) {
		LCPrintf(true, "Reunion API major version mismatch; expected %d, real %d\n", REUNION_API_VERSION_MAJOR, g_ReunionApi->version_major);
		return false;
	}

	if (g_ReunionApi->version_minor < REUNION_API_VERSION_MINOR) {
		LCPrintf(true, "Reunion API minor version mismatch; expected at least %d, real %d\n", REUNION_API_VERSION_MINOR, g_ReunionApi->version_minor);
		return false;
	}

	return true;
}
