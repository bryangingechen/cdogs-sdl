/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (c) 2014, Cong Xu
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
#include "handle_game_events.h"

#include <cdogs/damage.h>
#include <cdogs/game_events.h>
#include <cdogs/objs.h>


void HandleGameEvents(
	CArray *store,
	HUD *hud,
	ScreenShake *shake,
	HealthPickups *hp,
	EventHandlers *eventHandlers)
{
	for (int i = 0; i < (int)store->size; i++)
	{
		GameEvent *e = CArrayGet(store, i);
		switch (e->Type)
		{
		case GAME_EVENT_SCORE:
			Score(&gPlayerDatas[e->u.Score.PlayerIndex], e->u.Score.Score);
			HUDAddScoreUpdate(hud, e->u.Score.PlayerIndex, e->u.Score.Score);
			break;
		case GAME_EVENT_SCREEN_SHAKE:
			*shake = ScreenShakeAdd(
				*shake, e->u.ShakeAmount, gConfig.Graphics.ShakeMultiplier);
			break;
		case GAME_EVENT_SET_MESSAGE:
			HUDDisplayMessage(
				hud, e->u.SetMessage.Message, e->u.SetMessage.Ticks);
			break;
		case GAME_EVENT_GAME_START:
			if (eventHandlers->netInput.channel.state ==
				CHANNEL_STATE_CONNECTED)
			{
				NetInputSendMsg(
					&eventHandlers->netInput, SERVER_MSG_GAME_START);
			}
			break;
		case GAME_EVENT_ADD_HEALTH_PICKUP:
			MapPlaceHealth(e->u.AddPos);
			break;
		case GAME_EVENT_TAKE_HEALTH_PICKUP:
			ActorHeal(gPlayers[e->u.PickupPlayer], HEALTH_PICKUP_HEAL_AMOUNT);
			HealthPickupsRemoveOne(hp);
			HUDAddHealthUpdate(
				hud, e->u.PickupPlayer, HEALTH_PICKUP_HEAL_AMOUNT);
			break;
		case GAME_EVENT_MOBILE_OBJECT_REMOVE:
			MobileObjectRemove(&gMobObjList, e->u.MobileObjectRemoveId);
			break;
		case GAME_EVENT_HIT_CHARACTER:
			HitCharacter(
				e->u.HitCharacter.HitV,
				e->u.HitCharacter.Power,
				e->u.HitCharacter.Flags,
				e->u.HitCharacter.PlayerIndex,
				e->u.HitCharacter.Target,
				e->u.HitCharacter.Special,
				e->u.HitCharacter.HasHitSound);
			break;
		case GAME_EVENT_DAMAGE_CHARACTER:
			DamageCharacter(
				e->u.DamageCharacter.Power,
				e->u.DamageCharacter.PlayerIndex,
				e->u.DamageCharacter.Target);
			if (e->u.DamageCharacter.Power != 0)
			{
				HUDAddHealthUpdate(
					hud,
					e->u.DamageCharacter.TargetPlayerIndex,
					-e->u.DamageCharacter.Power);
			}
			break;
		case GAME_EVENT_MISSION_COMPLETE:
			HUDDisplayMessage(hud, "Mission complete", -1);
			hud->showExit = true;
			MapShowExitArea(&gMap);
			break;
		case GAME_EVENT_MISSION_INCOMPLETE:
			gMission.state = MISSION_STATE_PLAY;
			break;
		case GAME_EVENT_MISSION_PICKUP:
			gMission.state = MISSION_STATE_PICKUP;
			gMission.pickupTime = gMission.time;
			SoundPlay(&gSoundDevice, SND_DONE);
			break;
		case GAME_EVENT_MISSION_END:
			gMission.isDone = true;
			break;
		default:
			assert(0 && "unknown game event");
			break;
		}
	}
	GameEventsClear(store);
}
