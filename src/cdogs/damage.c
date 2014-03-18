/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin
    Copyright (C) 2003-2007 Lucas Martin-King

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright (c) 2013-2014, Cong Xu
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
#include "damage.h"

#include "actors.h"
#include "game_events.h"


bool CanHitCharacter(int flags, int player, TActor *actor)
{
	// Don't let players hurt themselves
	if (!(flags & FLAGS_HURTALWAYS) && player >= 0 && actor->pData &&
		&gPlayerDatas[player] == actor->pData)
	{
		return false;
	}
	return true;
}

void HitCharacter(
	Vec2i hitVector,
	int power,
	int flags,
	int player,
	TActor *actor,
	special_damage_e special,
	bool hasHitSound)
{
	bool isInvulnerable = ActorIsInvulnerable(
		actor, flags, player, gCampaign.Entry.mode);
	ActorTakeHit(
		actor,
		hitVector,
		power,
		special,
		hasHitSound,
		isInvulnerable,
		Vec2iNew(actor->tileItem.x, actor->tileItem.y));
}

bool CanDamageCharacter(
	int flags, int player, TActor *actor, special_damage_e special)
{
	if (!CanHitCharacter(flags, player, actor))
	{
		return false;
	}
	if (ActorIsImmune(actor, special))
	{
		return false;
	}
	return !ActorIsInvulnerable(actor, flags, player, gCampaign.Entry.mode);
}

static void TrackKills(int player, TActor *victim);
void DamageCharacter(int power, int player, TActor *actor)
{
	InjureActor(actor, power);
	if (actor->health <= 0)
	{
		TrackKills(player, actor);
	}
}
static void TrackKills(int player, TActor *victim)
{
	if (player >= 0)
	{
		if (victim->pData ||
			(victim->flags & (FLAGS_GOOD_GUY | FLAGS_PENALTY)))
		{
			gPlayerDatas[player].friendlies++;
		}
		else
		{
			gPlayerDatas[player].kills++;
		}
	}
}
