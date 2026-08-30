/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023-2026 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "igameevents.h"
#include "utlstring.h"

class CGameEventListener;
extern IGameEventManager2 *g_gameEventManager;

inline std::vector<CGameEventListener*> g_vecEventListeners;
typedef void (*FnEventListenerCallback)(IGameEvent* event);

class CGameEventListener : public IGameEventListener2
{
public:
	CGameEventListener(FnEventListenerCallback callback, const char* pszEventName) :
		m_Callback(callback), m_pszEventName(pszEventName)
	{
		g_vecEventListeners.push_back(this);
	}

	~CGameEventListener() override
	{
	}

	// FireEvent is called by EventManager if event just occured
	// KeyValue memory will be freed by manager if not needed anymore
	void FireGameEvent(IGameEvent* event) override
	{
		m_Callback(event);
	}

	const char* GetEventName() { return m_pszEventName; }

private:
	FnEventListenerCallback m_Callback;
	const char* m_pszEventName;
};

inline void RegisterEventListeners()
{
	static bool bRegistered = false;

	if (bRegistered || !g_gameEventManager)
		return;

	for (CGameEventListener* pListener : g_vecEventListeners)
		g_gameEventManager->AddListener(pListener, pListener->GetEventName(), true);

	bRegistered = true;
}

inline void UnregisterEventListeners()
{
	if (!g_gameEventManager)
		return;

	for (CGameEventListener* pListener : g_vecEventListeners)
		g_gameEventManager->RemoveListener(pListener);

	g_vecEventListeners.clear();
}

#define GAME_EVENT_F(_event)                                          \
	void _event##_callback(IGameEvent*);                              \
	CGameEventListener _event##_listener(_event##_callback, #_event); \
	void _event##_callback(IGameEvent* pEvent)