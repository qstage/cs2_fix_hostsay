#include <stdio.h>
#include "plugin.h"
#include "iserver.h"

#include "utils/module.h"
#include "player.h"

class GameSessionConfiguration_t
{
};

SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandRef, const CCommandContext &, const CCommand &);
SH_DECL_HOOK4_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);
SH_DECL_HOOK2(IGameEventManager2, LoadEventsFromFile, SH_NOATTRIB, 0, int, const char *, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);

int g_iLoadEventsFromFileId = -1;
IGameEventManager2 *g_gameEventManager = nullptr;

MMSPlugin g_ThisPlugin;
PLUGIN_EXPOSE(MMSPlugin, g_ThisPlugin);

void Message(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 255, 255), "[%s] %s", g_ThisPlugin.GetLogTag(), buf);

	va_end(args);
}

bool MMSPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameClients, IServerGameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);

	SH_ADD_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &MMSPlugin::Hook_DispatchConCommand), false);
	SH_ADD_HOOK(IServerGameClients, ClientPutInServer, g_pSource2GameClients, SH_MEMBER(this, &MMSPlugin::Hook_ClientPutInServer), true);
	SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &MMSPlugin::Hook_StartupServer), true);

	CModule server(GAMEBIN, "server");

	auto pCGameEventManagerVTable = (IGameEventManager2 *)server.FindVirtualTable("CGameEventManager");
	g_iLoadEventsFromFileId = SH_ADD_DVPHOOK(IGameEventManager2, LoadEventsFromFile, pCGameEventManagerVTable, SH_MEMBER(this, &MMSPlugin::Hook_LoadEventsFromFile), false);

	g_SMAPI->AddListener(this, this);
	META_CONVAR_REGISTER(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	if (late)
		RegisterEventListeners();

	return true;
}

int MMSPlugin::Hook_LoadEventsFromFile(const char *filename, bool bSearchAll)
{
	ExecuteOnce(g_gameEventManager = META_IFACEPTR(IGameEventManager2));

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void MMSPlugin::Hook_ClientPutInServer(CPlayerSlot slot, char const *pszName, int type, uint64 xuid)
{
	GetPlayer(slot.Get()).SetInGame(true);
}

GAME_EVENT_F(player_connect_full)
{
	int iSlot = GetPlayerSlot(pEvent->GetPlayerController("userid"));
	GetPlayer(iSlot).SetIsFullConnect(true);
}

GAME_EVENT_F(player_disconnect)
{
	int iSlot = GetPlayerSlot(pEvent->GetPlayerController("userid"));
	CPlayer& player = GetPlayer(iSlot);

	player.SetInGame(false);
	player.SetIsFullConnect(false);
}

void MMSPlugin::Hook_DispatchConCommand(ConCommandRef cmdHandle, const CCommandContext &ctx, const CCommand &args)
{
	bool bSay = !V_stricmp(args.Arg(0), "say");
	bool bTeamSay = !V_stricmp(args.Arg(0), "say_team");

	int iSlot = ctx.GetPlayerSlot().Get();
	CPlayer &player = GetPlayer(iSlot);

	if ((bSay || bTeamSay) && (!player.IsInGame() || !player.IsFullConnect()))
	{
		Message("Blocked chat message from user ID %i not fully in game (%i %i)\n", g_pEngineServer->GetPlayerUserId(iSlot).Get(), player.IsInGame(), player.IsFullConnect());
		RETURN_META(MRES_SUPERCEDE);
	}
}

bool MMSPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &MMSPlugin::Hook_DispatchConCommand), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientPutInServer, g_pSource2GameClients, SH_MEMBER(this, &MMSPlugin::Hook_ClientPutInServer), true);
	SH_REMOVE_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &MMSPlugin::Hook_StartupServer), true);

	SH_REMOVE_HOOK_ID(g_iLoadEventsFromFileId);

	UnregisterEventListeners();

	return true;
}