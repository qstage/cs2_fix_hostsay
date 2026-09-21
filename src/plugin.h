#ifndef _INCLUDE_METAMOD_SOURCE_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_PLUGIN_H_

#include <ISmmPlugin.h>
#include <igameevents.h>
#include "version_gen.h"

class MMSPlugin : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);

public:
	KHook::Return<void> Hook_StartupServer(INetworkServerService *, const GameSessionConfiguration_t &config, ISource2WorldSession *pSession, const char *pszMapName);
	KHook::Return<void> Hook_ClientDisconnect(IServerGameClients *, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char *pszName, uint64 xuid, const char *pszNetworkID);
	KHook::Return<void> Hook_DispatchConCommand(ICvar *, ConCommandRef cmdHandle, const CCommandContext &ctx, const CCommand &args);
	KHook::Return<int> Hook_LoadEventsFromFile(IGameEventManager2 *thisptr, const char *filename, bool bSearchAll);

public:
	const char *GetAuthor() { return PLUGIN_AUTHOR; }
	const char *GetName() { return PLUGIN_DISPLAY_NAME; }
	const char *GetDescription() { return PLUGIN_DESCRIPTION; }
	const char *GetURL() { return PLUGIN_URL; }
	const char *GetLicense() { return PLUGIN_LICENSE; }
	const char *GetVersion() { return PLUGIN_FULL_VERSION; }
	const char *GetDate() { return __DATE__; }
	const char *GetLogTag() { return PLUGIN_LOGTAG; }
};

extern MMSPlugin g_ThisPlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_PLUGIN_H_
