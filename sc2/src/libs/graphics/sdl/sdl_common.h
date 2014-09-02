//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#include "port.h"
#include <SDL.h>
#include <SDL_endian.h>
#include <SDL_image.h>

#include "../gfxintrn.h"
#include "libs/graphics/tfb_draw.h"
#include "libs/graphics/gfx_common.h"

// The Graphics Backend vtable
typedef struct _tfb_graphics_backend {
	void (*preprocess) (int force_redraw, int transition_amount, int fade_amount);
	void (*postprocess) (void);
	void (*screen) (SCREEN screen, Uint8 alpha, SDL_Rect *rect);
	void (*color) (Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Rect *rect);
} TFB_GRAPHICS_BACKEND;

extern TFB_GRAPHICS_BACKEND *graphics_backend;

extern SDL_Surface *SDL_Video;
extern SDL_Surface *SDL_Screen;
extern SDL_Surface *TransitionScreen;

extern SDL_Surface *SDL_Screens[TFB_GFX_NUMSCREENS];

extern SDL_Surface *format_conv_surf;

SDL_Surface* TFB_DisplayFormatAlpha (SDL_Surface *surface);
void TBF_DrawCanvas_GetColorkeyAlphamask (SDL_Surface *surface, Uint32* mask, Uint32* colorkey);

SDL_Surface* Create_Screen (SDL_Surface *templat, int w, int h);
int ReInit_Screen (SDL_Surface **screen, SDL_Surface *templat, int w, int h);
void UnInit_Screen (SDL_Surface **screen);

// SDL2 compatibility, so we don't need to plaster #ifs all over
#if !SDL_VERSION_ATLEAST(1,3,0)
# define SDL_WINDOW_FULLSCREEN SDL_FULLSCREEN
# define SDL_WINDOW_OPENGL SDL_OPENGL
#else
// When calling SDL_SetColorKey(), you should pass SDL_TRUE instead of SDL_SRCCOLORKEY.
# define SDL_SRCCOLORKEY SDL_TRUE
#endif

#endif
