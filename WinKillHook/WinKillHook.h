#pragma once

#ifdef WINKILLHOOK_EXPORTS
#define WINKILLHOOK_API __declspec(dllexport)
#else
#define WINKILLHOOK_API __declspec(dllimport)
#endif

WINKILLHOOK_API bool winkill_install_hook(HWND);
WINKILLHOOK_API bool winkill_remove_hook();
