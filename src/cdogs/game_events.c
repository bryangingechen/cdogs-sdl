/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.

    Copyright (c) 2013-2015, Cong Xu
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "game_events.h"

#include <string.h>

#include "net_client.h"
#include "net_server.h"
#include "utils.h"

CArray gGameEvents;

void GameEventsInit(CArray *store)
{
	CArrayInit(store, sizeof(GameEvent));
}
void GameEventsTerminate(CArray *store)
{
	CArrayTerminate(store);
}


// Array indexed by GameEvent
static GameEventEntry sGameEventEntries[] =
{
	{ GAME_EVENT_NONE, false, false, false, false, NULL },

	{ GAME_EVENT_CLIENT_ID, false, false, false, false, NClientId_fields },
	{ GAME_EVENT_CAMPAIGN_DEF, false, false, false, false, NCampaignDef_fields },
	{ GAME_EVENT_ADD_PLAYERS, true, false, true, false, NAddPlayers_fields },
	{ GAME_EVENT_ADD_MAP_OBJECT, true, false, true, true, NAddMapObject_fields },
	{ GAME_EVENT_OBJECTIVE_COUNT, false, false, false, true, NObjectiveCount_fields },
	{ GAME_EVENT_CLIENT_READY, false, false, false, false, NULL },
	{ GAME_EVENT_NET_GAME_START, false, false, false, false, NULL },

	{ GAME_EVENT_SCORE, true, true, true, true, NULL },
	{ GAME_EVENT_SOUND_AT, true, false, true, true, NSound_fields },
	{ GAME_EVENT_SCREEN_SHAKE, false, false, true, true, NULL },
	{ GAME_EVENT_SET_MESSAGE, false, false, true, true, NULL },

	{ GAME_EVENT_GAME_START, true, false, true, true, NULL },

	{ GAME_EVENT_ACTOR_ADD, true, false, true, true, NActorAdd_fields },
	{ GAME_EVENT_ACTOR_MOVE, true, true, true, true, NActorMove_fields },
	{ GAME_EVENT_ACTOR_STATE, true, true, true, true, NActorState_fields },
	{ GAME_EVENT_ACTOR_DIR, true, true, true, true, NActorDir_fields },
	{ GAME_EVENT_ACTOR_PICKUP_ALL, false, true, true, true, NActorPickupAll_fields },
	{ GAME_EVENT_ACTOR_REPLACE_GUN, true, false, true, true, NActorReplaceGun_fields },
	{ GAME_EVENT_ACTOR_HEAL, true, false, true, true, NActorHeal_fields },
	{ GAME_EVENT_ACTOR_ADD_AMMO, true, false, true, true, NActorAddAmmo_fields },
	{ GAME_EVENT_ACTOR_USE_AMMO, true, true, true, true, NActorUseAmmo_fields },

	{ GAME_EVENT_ADD_PICKUP, true, false, true, true, NAddPickup_fields },
	{ GAME_EVENT_REMOVE_PICKUP, true, false, true, true, NRemovePickup_fields },

	{ GAME_EVENT_MOBILE_OBJECT_REMOVE, false, false, true, true, NULL },
	{ GAME_EVENT_PARTICLE_REMOVE, false, false, true, true, NULL },
	{ GAME_EVENT_ADD_BULLET, true, true, true, true, NAddBullet_fields },
	{ GAME_EVENT_ADD_PARTICLE, false, false, true, true, NULL },
	{ GAME_EVENT_HIT_CHARACTER, false, false, true, true, NULL },
	{ GAME_EVENT_ACTOR_IMPULSE, false, false, true, true, NULL },
	{ GAME_EVENT_DAMAGE_CHARACTER, false, false, true, true, NULL },
	{ GAME_EVENT_TRIGGER, true, false, true, true, NTrigger_fields },
	{ GAME_EVENT_EXPLORE_TILE, true, false, true, true, NExploreTile_fields },
	{ GAME_EVENT_RESCUE_CHARACTER, true, false, true, true, NRescueCharacter_fields },
	{ GAME_EVENT_OBJECTIVE_UPDATE, true, true, true, true, NObjectiveUpdate_fields },
	{ GAME_EVENT_ADD_KEYS, true, false, true, true, NAddKeys_fields },

	{ GAME_EVENT_MISSION_COMPLETE, true, false, true, true, NMissionComplete_fields },

	{ GAME_EVENT_MISSION_INCOMPLETE, true, false, true, true, NULL },
	{ GAME_EVENT_MISSION_PICKUP, true, false, true, true, NULL },
	{ GAME_EVENT_MISSION_END, true, false, true, true, NULL }
};
GameEventEntry GameEventGetEntry(const GameEventType e)
{
	return sGameEventEntries[(int)e];
}

void GameEventsEnqueue(CArray *store, GameEvent e)
{
	if (store->elemSize == 0)
	{
		return;
	}
	// If we're the server, broadcast any events that clients need
	// If we're the client, pass along to server, but only if it's for a local player
	// Otherwise we'd ping-pong the same updates from the server
	const GameEventEntry gee = sGameEventEntries[e.Type];
	if (gee.Broadcast)
	{
		NetServerBroadcastMsg(&gNetServer, gee.Type, &e.u);
	}
	if (gee.Submit)
	{
		int actorUID = -1;
		bool actorIsLocal = false;
		switch (e.Type)
		{
		case GAME_EVENT_ACTOR_MOVE: actorUID = e.u.ActorMove.UID; break;
		case GAME_EVENT_ACTOR_STATE: actorUID = e.u.ActorState.UID; break;
		case GAME_EVENT_ACTOR_DIR: actorUID = e.u.ActorDir.UID; break;
		case GAME_EVENT_ACTOR_PICKUP_ALL: actorUID = e.u.ActorPickupAll.UID; break;
		case GAME_EVENT_ACTOR_USE_AMMO: actorUID = e.u.UseAmmo.UID; break;
		case GAME_EVENT_ADD_BULLET:
			actorIsLocal = PlayerIsLocal(e.u.AddBullet.PlayerUID);
			break;
		default: break;
		}
		if (actorUID >= 0)
		{
			actorIsLocal = ActorIsLocalPlayer(actorUID);
		}
		if (actorIsLocal)
		{
			NetClientSendMsg(&gNetClient, gee.Type, &e.u);
		}
	}

	CArrayPushBack(store, &e);
}
static bool EventComplete(const void *elem);
void GameEventsClear(CArray *store)
{
	CArrayRemoveIf(store, EventComplete);
}
static bool EventComplete(const void *elem)
{
	return ((const GameEvent *)elem)->Delay < 0;
}

GameEvent GameEventNew(GameEventType type)
{
	GameEvent e;
	memset(&e, 0, sizeof e);
	e.Type = type;
	return e;
}
