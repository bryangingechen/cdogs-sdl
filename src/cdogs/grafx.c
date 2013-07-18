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
#include "grafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#include <SDL_events.h>
#include <SDL_mouse.h>

#include "actors.h"
#include "blit.h"
#include "config.h"
#include "defs.h"
#include "draw.h"
#include "mission.h"
#include "pics.h" /* for gPalette */
#include "files.h"
#include "triggers.h"
#include "utils.h"

GFX_Mode gfx_modelist[] = {
	{ 320, 240 },
	{ 400, 300 },
	{ 640, 480 },
	{ 800, 600 }, /* things go strange above this... */
	{ 1024, 768 },
	{ 480, 320 },	// HVGA
	{ 800, 480 },	// WVGA
	{ 960, 540 },	// qHD
	{ 0, 0 },
};
#define MODE_MAX 7


static int mode_idx = 1;

void Gfx_ModePrev(void)
{
	mode_idx--;
	if (mode_idx < 0)
	{
		mode_idx = MODE_MAX;
	}

	gConfig.Graphics.ResolutionWidth = gfx_modelist[mode_idx].w;
	gConfig.Graphics.ResolutionHeight = gfx_modelist[mode_idx].h;
}

void Gfx_ModeNext(void)
{
	mode_idx++;
	if (mode_idx > MODE_MAX)
	{
		mode_idx = 0;
	}

	gConfig.Graphics.ResolutionWidth = gfx_modelist[mode_idx].w;
	gConfig.Graphics.ResolutionHeight = gfx_modelist[mode_idx].h;
}

static int ValidMode(unsigned int w, unsigned int h)
{
	int i;

	for (i = 0; ; i++) {
		unsigned int m_w = gfx_modelist[i].w;
		unsigned int m_h = gfx_modelist[i].h;

		if (m_w == 0)
		{
			break;
		}

		if (m_w == w && m_h == h) {
			mode_idx = i;
			return 1;
		}
	}

	return 0;
}


GraphicsDevice gGraphicsDevice;

int IsRestartRequiredForConfig(GraphicsDevice *device, GraphicsConfig *config)
{
	return
		!device->IsInitialized ||
		device->cachedConfig.Fullscreen != config->Fullscreen ||
		device->cachedConfig.ResolutionWidth != config->ResolutionWidth ||
		device->cachedConfig.ResolutionHeight != config->ResolutionHeight ||
		device->cachedConfig.ScaleFactor != config->ScaleFactor;
}

void GraphicsInit(GraphicsDevice *device)
{
	device->IsInitialized = 0;
	device->IsWindowInitialized = 0;
	device->screen = NULL;
	memset(&device->cachedConfig, 0, sizeof device->cachedConfig);
	device->buf = NULL;
	device->bkg = NULL;
}

void MakeBkg(GraphicsDevice *device, GraphicsConfig *config)
{
	struct Buffer *buffer = NewBuffer(128, 128);
	unsigned char *p;
	int i;
	TranslationTable randomTintTable;

	SetupQuickPlayCampaign(&gCampaign.Setting);
	gCampaign.seed = rand();
	SetupMission(0, 1, &gCampaign);
	SetupMap();
	SetBuffer(1024, 768, buffer, X_TILES);
	DrawBuffer(buffer, 0);
	CFREE(buffer);
	KillAllObjects();
	FreeTriggersAndWatches();
	gCampaign.seed = gConfig.Game.RandomSeed;

	p = device->buf;
	SetPaletteRanges(15, 12, 10, 0);
	BuildTranslationTables();
	SetRandomTintTable(&randomTintTable, 256);
	for (i = 0; i < GraphicsGetMemSize(config); i++)
	{
		p[i] = randomTintTable[p[i] & 0xFF];
	}
}

// Initialises the video subsystem.
// To prevent needless screen flickering, config is compared with cache
// to see if anything changed. If not, don't recreate the screen.
void GraphicsInitialize(GraphicsDevice *device, GraphicsConfig *config, int force)
{
	int sdl_flags = 0;
	unsigned int w, h = 0;
	unsigned int rw, rh;

	if (!IsRestartRequiredForConfig(device, config))
	{
		return;
	}

	device->IsInitialized = 0;

	sdl_flags |= SDL_HWPALETTE;
	sdl_flags |= SDL_SWSURFACE;

	if (config->Fullscreen)
	{
		sdl_flags |= SDL_FULLSCREEN;
	}

	rw = w = config->ResolutionWidth;
	rh = h = config->ResolutionHeight;

	if (config->ScaleFactor > 1)
	{
		rw *= config->ScaleFactor;
		rh *= config->ScaleFactor;
	}

	if (!force)
	{
		if (!ValidMode(w, h))
		{
			printf("!!! Invalid Video Mode %dx%d\n", w, h);
			return;
		}
	}
	else
	{
		printf("\n");
		printf("  BIG FAT WARNING: If this blows up in your face,\n");
		printf("  and mutilates your cat, please don't cry.\n");
		printf("\n");
	}

	printf("Window dimensions:\t%dx%d\n", rw, rh);
	SDL_FreeSurface(device->screen);
	device->screen = SDL_SetVideoMode(rw, rh, 8, sdl_flags);
	if (device->screen == NULL)
	{
		printf("ERROR: InitVideo: %s\n", SDL_GetError());
		return;
	}

	CFREE(device->buf);
	CCALLOC(device->buf, GraphicsGetMemSize(config));
	CFREE(device->bkg);
	CCALLOC(device->bkg, GraphicsGetMemSize(config));

	if (!device->IsWindowInitialized)
	{
		/* only do this the first time */
		char title[32];
		debug(D_NORMAL, "setting caption and icon...\n");
		sprintf(title, "C-Dogs %s [Port %s]", CDOGS_VERSION, CDOGS_SDL_VERSION);
		SDL_WM_SetCaption(title, NULL);
		SDL_WM_SetIcon(SDL_LoadBMP(GetDataFilePath("cdogs_icon.bmp")), NULL);
		SDL_ShowCursor(SDL_DISABLE);
	}
	else
	{
		debug(D_NORMAL, "Changed video mode...\n");
	}

	CDogsSetClip(0, 0, config->ResolutionWidth - 1, config->ResolutionHeight - 1);
	debug(D_NORMAL, "Internal dimensions:\t%dx%d\n",
		config->ResolutionWidth, config->ResolutionHeight);

	device->IsInitialized = 1;
	device->IsWindowInitialized = 1;
	device->cachedConfig = *config;
	device->cachedConfig.ResolutionWidth = w;
	device->cachedConfig.ResolutionHeight = h;
	// Need to make background here since dimensions use cached config
	MakeBkg(device, config);
	CDogsSetPalette(gPalette);
	BlitSetBrightness(config->Brightness);
	memcpy(device->bkg, device->buf, GraphicsGetMemSize(config));
	memset(device->buf, 0, GraphicsGetMemSize(config));
}

void GraphicsTerminate(GraphicsDevice *device)
{
	debug(D_NORMAL, "Shutting down video...\n");
	SDL_FreeSurface(device->screen);
	SDL_VideoQuit();
	CFREE(device->buf);
	CFREE(device->bkg);
}

int GraphicsGetMemSize(GraphicsConfig *config)
{
	return config->ResolutionWidth * config->ResolutionHeight;
}

void GraphicsBlitBkg(GraphicsDevice *device)
{
	memcpy(device->buf, device->bkg, GraphicsGetMemSize(&device->cachedConfig));
}

char *GrafxGetResolutionStr(void)
{
	static char buf[16];
	sprintf(buf, "%dx%d",
		gConfig.Graphics.ResolutionWidth, gConfig.Graphics.ResolutionHeight);
	return buf;
}


void SetColorZero(
	GraphicsDevice *device, unsigned char r, unsigned char g, unsigned char b)
{
	SDL_Color col;
	col.r = r;
	col.g = g;
	col.b = b;
	SDL_SetPalette(device->screen, SDL_PHYSPAL, &col, 0, 1);
	return;
}
