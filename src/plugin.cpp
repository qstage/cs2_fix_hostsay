#include <stdio.h>
#include "plugin.h"
#include "iserver.h"

#include "utils/module.h"
#include "player.h"

class GameSessionConfiguration_t
{
};

KHook::Virtual<ICvar, void, ConCommandRef, const CCommandContext &, const CCommand &> g_DispatchConCommand(&ICvar::DispatchConCommand, &g_ThisPlugin, &MMSPlugin::Hook_DispatchConCommand, nullptr);
KHook::Virtual<IServerGameClients, void, CPlayerSlot, ENetworkDisconnectionReason, const char *, uint64, const char *> g_ClientDisconnect(&IServerGameClients::ClientDisconnect, &g_ThisPlugin, nullptr, &MMSPlugin::Hook_ClientDisconnect);
KHook::Virtual<IGameEventManager2, int, const char *, bool> g_LoadEventsFromFile(&IGameEventManager2::LoadEventsFromFile, &g_ThisPlugin, nullptr, &MMSPlugin::Hook_LoadEventsFromFile);
KHook::Virtual<INetworkServerService, void, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *> g_StartupServer(&INetworkServerService::StartupServer, &g_ThisPlugin, nullptr, &MMSPlugin::Hook_StartupServer);

IGameEventManager2 *g_gameEventManager[2]{};

MMSPlugin g_ThisPlugin;
PLUGIN_EXPOSE(MMSPlugin, g_ThisPlugin);

class CPlayerTeamEvent : public IGameEventListener2
{
	void FireGameEvent(IGameEvent *pEvent) override
	{
		if (auto *pPlayer = GetPlayer(pEvent->GetPlayerController("userid")))
			pPlayer->SetIsInGame(true);
	}
} g_PlayerTeamEvent;

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

	g_DispatchConCommand.Add(g_pCVar);
	g_StartupServer.Add(g_pNetworkServerService);
	g_ClientDisconnect.Add(g_pSource2GameClients);

	CModule server(GAMEBIN, "server");
	g_gameEventManager[0] = (IGameEventManager2 *)server.FindVirtualTable("CGameEventManager");

	g_LoadEventsFromFile.AddGlobal((IGameEventManager2 *)&g_gameEventManager[0]);

	g_SMAPI->AddListener(this, this);
	META_CONVAR_REGISTER(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	return true;
}

KHook::Return<void> MMSPlugin::Hook_StartupServer(INetworkServerService *, const GameSessionConfiguration_t &config, ISource2WorldSession *pSession, const char *pszMapName)
{
	g_gameEventManager[1]->AddListener(&g_PlayerTeamEvent, "player_team", true);

	return {KHook::Action::Ignore};
}

KHook::Return<int> MMSPlugin::Hook_LoadEventsFromFile(IGameEventManager2 *thisptr, const char *filename, bool bSearchAll)
{
	ExecuteOnce(g_gameEventManager[1] = thisptr);

	return {KHook::Action::Ignore};
}

KHook::Return<void> MMSPlugin::Hook_ClientDisconnect(IServerGameClients *, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char *pszName, uint64 xuid, const char *pszNetworkID)
{
	if (auto *pPlayer = GetPlayer(slot))
		pPlayer->SetIsInGame(false);

	return {KHook::Action::Ignore};
}

KHook::Return<void> MMSPlugin::Hook_DispatchConCommand(ICvar *, ConCommandRef cmdHandle, const CCommandContext &ctx, const CCommand &args)
{
	bool bSay = !V_stricmp(args.Arg(0), "say");
	bool bTeamSay = !V_stricmp(args.Arg(0), "say_team");

	int iSlot = ctx.GetPlayerSlot().Get();
	if (iSlot == -1)
		return {KHook::Action::Ignore};

	CPlayer *pPlayer = GetPlayer(ctx.GetPlayerSlot());

	if ((bSay || bTeamSay) && (!pPlayer || !pPlayer->IsInGame()))
	{
		Message("Blocked chat message from user ID %i not fully in game\n", g_pEngineServer->GetPlayerUserId(iSlot).Get());
		return {KHook::Action::Supersede};
	}

	return {KHook::Action::Ignore};
}

bool MMSPlugin::Unload(char *error, size_t maxlen)
{
	g_DispatchConCommand.Remove(g_pCVar);
	g_StartupServer.Remove(g_pNetworkServerService);
	g_ClientDisconnect.Remove(g_pSource2GameClients);

	g_LoadEventsFromFile.RemoveGlobal((IGameEventManager2 *)&g_gameEventManager[0]);

	g_gameEventManager[1]->RemoveListener(&g_PlayerTeamEvent);

	return true;
}