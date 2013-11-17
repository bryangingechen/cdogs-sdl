/*
	C-Dogs SDL
	A port of the legendary (and fun) action/arcade cdogs.

	Copyright (c) 2013, Cong Xu
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
#include "menu_utils.h"

#include <cdogs/draw.h>
#include <cdogs/text.h>


void MenuDisplayPlayer(GraphicsDevice *g, Vec2i pos, Vec2i size, void *data)
{
	MenuDisplayPlayerData *d = data;
	Vec2i playerPos;
	Vec2i namePos;
	pos.x -= size.x;	// move to left half of screen
	playerPos = Vec2iNew(
		pos.x + size.x * 3 / 4 - 12 / 2, CENTER_Y(pos, size, 0));
	namePos = Vec2iAdd(playerPos, Vec2iNew(-20, -36));

	UNUSED(g);

	if (strcmp((*d->currentMenu)->name, "Name") == 0)
	{
		char s[22];
		sprintf(s, "%c%s%c", '\020', d->pData->name, '\021');
		CDogsTextStringAt(namePos.x, namePos.y, s);
	}
	else
	{
		CDogsTextStringAt(namePos.x, namePos.y, d->pData->name);
	}

	DrawCharacterSimple(
		d->c, playerPos,
		DIRECTION_DOWN, STATE_IDLE, -1, GUNSTATE_READY, &d->c->table);
}

void MenuDisplayPlayerControls(
	GraphicsDevice *g, Vec2i pos, Vec2i size, void *data)
{
	char s[256];
	KeyConfig *config = data;
	Vec2i textPos = Vec2iNew(0, pos.y + size.y - (size.y / 6));
	int textWidth = 0;

	if (config->Device == INPUT_DEVICE_KEYBOARD)
	{
		sprintf(s, "(%s, %s, %s, %s, %s and %s)",
			SDL_GetKeyName(config->Keys.left),
			SDL_GetKeyName(config->Keys.right),
			SDL_GetKeyName(config->Keys.up),
			SDL_GetKeyName(config->Keys.down),
			SDL_GetKeyName(config->Keys.button1),
			SDL_GetKeyName(config->Keys.button2));
		textWidth = TextGetStringWidth(s);
		if (textWidth < 125)
		{
			textPos.x = pos.x - textWidth / 2;
			DrawTextString(s, g, textPos);
		}
		else
		{
			sprintf(s, "(%s, %s, %s,",
				SDL_GetKeyName(config->Keys.left),
				SDL_GetKeyName(config->Keys.right),
				SDL_GetKeyName(config->Keys.up));
			textWidth = TextGetStringWidth(s);
			textPos.x = pos.x - textWidth / 2;
			textPos.y -= 10;
			DrawTextString(s, g, textPos);
			sprintf(s, "%s, %s and %s)",
				SDL_GetKeyName(config->Keys.down),
				SDL_GetKeyName(config->Keys.button1),
				SDL_GetKeyName(config->Keys.button2));
			textWidth = TextGetStringWidth(s);
			textPos.x = pos.x - textWidth / 2;
			textPos.y += 10;
			DrawTextString(s, g, textPos);
		}
	}
	else if (config->Device == INPUT_DEVICE_MOUSE)
	{
		sprintf(s, "(mouse wheel to scroll, left and right click)");
		textWidth = TextGetStringWidth(s);
		textPos.x = pos.x - textWidth / 2;
		DrawTextString(s, g, textPos);
	}
	else
	{
		sprintf(s, "(%s)", InputDeviceName(config->Device));
		textWidth = TextGetStringWidth(s);
		textPos.x = pos.x - textWidth / 2;
		DrawTextString(s, g, textPos);
	}
}
