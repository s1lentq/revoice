#pragma once

#include "revoice_shared.h"

#define REVOICE_CFG_FILE		"revoice.cfg"

class CRevoiceConfig {
private:
	int m_LogMode;

	bool parseCfgParam(const char* param, const char* value);

public:
	static CRevoiceConfig* createDefault();
	static CRevoiceConfig* load(const char* fname);

	bool hasLogMode(revoice_log_mode m) {
		return (m_LogMode & m) == m;
	}

};

extern CRevoiceConfig* g_RevoiceConfig;
extern bool Revoice_Cfg_LoadDefault();
