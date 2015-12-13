#pragma once

#include "rehlds_api.h"

#define MAX_PLAYERS					32
#define MAX_STEAMIDSALTLEN			64

#define LOG_PREFIX					"[REVOICE]: "

#define HLDS_APPID					10
#define HLDS_APPVERSION				"1.1.2.7/Stdio"


enum revoice_log_mode {
	rl_none = 0,
	rl_console = 1,
	rl_logfile = 2,
};

enum revoice_codec_type {
	vct_none,
	vct_silk,
	vct_speex,
};

template <typename T>
T _min(T a, T b) {
	return (a < b) ? a : b;
}

template <typename T>
T _max(T a, T b) {
	return (a < b) ? b : a;
}

template <typename T>
T clamp(T a, T min, T max) {
	return (a > max) ? max : (a < min) ? min : a;
}

extern char* trimbuf(char *str);
extern void LCPrintf(bool critical, const char *fmt, ...);

extern bool Revoice_Load();
extern bool Revoice_Utils_Init();
extern void util_syserror(const char* fmt, ...);

