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

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif
#ifdef WIN32
#	include <direct.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "getopt/getopt.h"
#endif

#include "libs/graphics/gfx_common.h"
#include "libs/sound/sound.h"
#include "libs/input/input_common.h"
#include "libs/tasklib.h"
#include "file.h"
#include "port.h"
#include "options.h"
#include "uqmversion.h"
#include "comm.h"

#if defined(GFXMODULE_SDL)
#	include <SDL.h>
			// Including this is actually necessary on OSX.
#endif

/* TODO: Remove these (making them local to main() ) once threading is
   gone. */

int snddriver, soundflags;

static int
Check_PC_3DO_opt (const char *value, DWORD mask, const char *opt)
{
	if (value == NULL)
	{
		fprintf (stderr, "option '%s' requires a value!\n", opt);
		return -1;
	}

	if ((mask & OPT_3DO) && strcmp (value, "3do") == 0)
		return OPT_3DO;
	if ((mask & OPT_PC) && strcmp (value, "pc") == 0)
		return OPT_PC;
	fprintf (stderr, "Unknown option '%s %s' found!", opt, value);
	return -1;
}

int
main (int argc, char *argv[])
{
	int gfxdriver = TFB_GFXDRIVER_SDL_PURE;
	int gfxflags = 0;
	int width = 640, height = 480, bpp = 16;
	int vol;
	int val;
	const char *contentDir;
	float gamma = 1.0;
	int gammaset = 0;
	const char **addons;
	int numAddons;

	enum
	{
		CSCAN_OPT = 1000,
		MENU_OPT,
		FONT_OPT,
		SCROLL_OPT,
		SOUND_OPT,
		STEREOSFX_OPT,
		ADDON_OPT
	};

	int option_index = 0, c;
	static struct option long_options[] = 
	{
		{"res", 1, NULL, 'r'},
		{"bpp", 1, NULL, 'd'},
		{"fullscreen", 0, NULL, 'f'},
		{"opengl", 0, NULL, 'o'},
		{"scale", 1, NULL, 'c'},
		{"meleescale", 1, NULL, 'b'},
		{"scanlines", 0, NULL, 's'},
		{"fps", 0, NULL, 'p'},
		{"contentdir", 1, NULL, 'n'},
		{"help", 0, NULL, 'h'},
		{"musicvol", 1, NULL, 'M'},
		{"sfxvol", 1, NULL, 'S'},
		{"speechvol", 1, NULL, 'T'},
		{"audioquality", 1, NULL, 'q'},
		{"nosubtitles", 0, NULL, 'u'},
		{"music", 1, NULL, 'm'},
		{"gamma", 1, NULL, 'g'},
		//  options with no short equivalent
		{"cscan", 1, NULL, CSCAN_OPT},
		{"menu", 1, NULL, MENU_OPT},
		{"font", 1, NULL, FONT_OPT},
		{"scroll", 1, NULL, SCROLL_OPT},
		{"sound", 1, NULL, SOUND_OPT},
		{"stereosfx", 0, NULL, STEREOSFX_OPT},
		{"addon", 1, NULL, ADDON_OPT},
		{0, 0, 0, 0}
	};

	/* TODO: Once threading is gone, these become local variables
	   again.  In the meantime, they must be global so that
	   initAudio (in StarCon2Main) can see them.  initAudio needed
	   to be moved there because calling AssignTask in the main
	   thread doesn't work */
	snddriver = audio_DRIVER_MIXSDL;
	soundflags = audio_QUALITY_MEDIUM;

	fprintf (stderr, "The Ur-Quan Masters v%d.%d%s (compiled %s %s)\n"
	        "This software comes with ABSOLUTELY NO WARRANTY;\n"
			"for details see the included 'COPYING' file.\n\n",
			UQM_MAJOR_VERSION, UQM_MINOR_VERSION, UQM_EXTRA_VERSION,
			__DATE__, __TIME__);

#ifdef CONTENTDIR
	contentDir = CONTENTDIR;
#elif defined (win32)
	contentDir = "../../content";
#else
	contentDir = "content";
#endif


	/* mem_init () uses mutexes.  Mutex creation cannot use
	   the memory system until the memory system is rewritten
	   to rely on a thread-safe allocator.
	*/
	TFB_PreInit ();
	mem_init ();
	InitThreadSystem ();

	addons = HMalloc(1 * sizeof (const char *));
	addons[0] = NULL;
	numAddons = 0;
	
	while ((c = getopt_long(argc, argv, "r:d:foc:b:spn:?hM:S:T:m:q:ug:", long_options, &option_index)) != -1)
	{
		switch (c) {
			case 'r':
				sscanf(optarg, "%dx%d", &width, &height);
			break;
			case 'd':
				sscanf(optarg, "%d", &bpp);
			break;
			case 'f':
				gfxflags |= TFB_GFXFLAGS_FULLSCREEN;
			break;
			case 'o':
				gfxdriver = TFB_GFXDRIVER_SDL_OPENGL;
			break;
			case 'c':
				if (!strcmp (optarg, "bilinear"))
				{
					gfxflags |= TFB_GFXFLAGS_SCALE_BILINEAR;
				}
				else if (!strcmp (optarg, "biadapt"))
				{
					gfxflags |= TFB_GFXFLAGS_SCALE_BIADAPT;
				}
				else if (!strcmp (optarg, "biadv"))
				{
					gfxflags |= TFB_GFXFLAGS_SCALE_BIADAPTADV;
				}
			break;
			case 'b':
				if (!strcmp (optarg, "nearest"))
					optMeleeScale = TFB_SCALE_NEAREST;
				else if (!strcmp (optarg, "trilinear"))
					optMeleeScale = TFB_SCALE_TRILINEAR;
			break;
			case 's':
				gfxflags |= TFB_GFXFLAGS_SCANLINES;
			break;
			case 'p':
				gfxflags |= TFB_GFXFLAGS_SHOWFPS;
			break;
			case 'n':
				contentDir = optarg;
			break;
			case 'M':
				sscanf(optarg, "%d", &vol);
				if (vol < 0)
					vol = 0;
				if (vol > 100)
					vol = 100;
				musicVolumeScale = vol / 100.0f;
			break;
			case 'S':
				sscanf(optarg, "%d", &vol);
				if (vol < 0)
					vol = 0;
				if (vol > 100)
					vol = 100;
				sfxVolumeScale = vol / 100.0f;
			break;
			case 'T':
				sscanf(optarg, "%d", &vol);
				if (vol < 0)
					vol = 0;
				if (vol > 100)
					vol = 100;
				speechVolumeScale = vol / 100.0f;
			break;
			case 'q':
				if (!strcmp (optarg, "high"))
				{
					soundflags &= ~(audio_QUALITY_MEDIUM|audio_QUALITY_LOW);
					soundflags |= audio_QUALITY_HIGH;
				}
				else if (!strcmp (optarg, "medium"))
				{
					soundflags &= ~(audio_QUALITY_HIGH|audio_QUALITY_LOW);
					soundflags |= audio_QUALITY_MEDIUM;
				}
				else if (!strcmp (optarg, "low"))
				{
					soundflags &= ~(audio_QUALITY_HIGH|audio_QUALITY_MEDIUM);
					soundflags |= audio_QUALITY_LOW;
				}
			break;
		    case 'u':
			    optSubtitles = FALSE;
			break;
			case 'm':
				if ((val = Check_PC_3DO_opt (optarg, 
						OPT_PC | OPT_3DO, 
						long_options[option_index].name)) != -1)
					optWhichMusic = val;
			break;
			case 'g':
				sscanf(optarg, "%f", &gamma);
				gammaset = 1;
			break;
			case CSCAN_OPT:
				if ((val = Check_PC_3DO_opt (optarg, 
						OPT_PC | OPT_3DO, 
						long_options[option_index].name)) != -1)
					optWhichCoarseScan = val;
			break;
			case MENU_OPT:
				if ((val = Check_PC_3DO_opt (optarg, 
						OPT_PC | OPT_3DO, 
						long_options[option_index].name)) != -1)
					optWhichMenu = val;
			break;
			case FONT_OPT:
				if ((val = Check_PC_3DO_opt (optarg, 
						OPT_PC | OPT_3DO, 
						long_options[option_index].name)) != -1)
					optWhichFonts = val;
			break;
			case SCROLL_OPT:
				if ((val = Check_PC_3DO_opt (optarg, 
						OPT_PC | OPT_3DO, 
						long_options[option_index].name)) != -1)
					optSmoothScroll = val;
			break;
			case SOUND_OPT:
				if (!strcmp (optarg, "openal"))
				{
					snddriver = audio_DRIVER_OPENAL;
				}
				else if (!strcmp (optarg, "none"))
				{
					snddriver = audio_DRIVER_NOSOUND;
				}
				else if (!strcmp (optarg, "mixsdl"))
				{
					snddriver = audio_DRIVER_MIXSDL;
				}
			break;
			case STEREOSFX_OPT:
				optStereoSFX = TRUE;
			break;
			case ADDON_OPT:
				numAddons++;
				addons = HRealloc ((void*)addons,
						(numAddons + 1) * sizeof (const char *));
				addons[numAddons - 1] = optarg;
				addons[numAddons] = NULL;
				break;
			default:
				printf ("\nOption %s not found!\n", long_options[option_index].name);
			case '?':
			case 'h':
				printf("Options:\n");
				printf("  -r, --res=WIDTHxHEIGHT (default 640x480, bigger "
						"works only with --opengl)\n");
				printf("  -d, --bpp=BITSPERPIXEL (default 16)\n");
				printf("  -f, --fullscreen (default off)\n");
				printf("  -o, --opengl (default off)\n");
				printf("  -c, --scale=MODE (bilinear, biadapt or biadv, "
						"default is none)\n");
				printf("  -b, --meleescale=MODE (nearest or trilinear, "
						"default is trilinear)\n");
				printf("  -s, --scanlines (default off)\n");
				printf("  -p, --fps (default off)\n");
				printf("  -g, --gamma=CORRECTIONVALUE (default 1.0, which "
						"causes no change)\n");
				printf("  -n, --contentdir=CONTENTDIR\n");
				printf("  -M, --musicvol=VOLUME (0-100, default 100)\n");
				printf("  -S, --sfxvol=VOLUME (0-100, default 100)\n");
				printf("  -T, --speechvol=VOLUME (0-100, default 100)\n");
				printf("  -q, --audioquality=QUALITY (high, medium or low, "
						"default medium)\n");
				printf("  -u, --nosubtitles\n");
				printf("  --addon <addon> (using a specific addon; "
						"may be specified multiple times)\n");
				printf("  --sound=DRIVER (openal, mixsdl, none; default "
						"mixsdl)\n");
				printf("  --stereosfx (enables positional sound effects, "
						"currently only for openal)\n");
				printf("The following options can take either '3do' or 'pc' "
						"as an option:\n");
				printf("  -m, --music : Music version (default 3do)\n");
				printf("  --cscan     : coarse-scan display, pc=text, "
						"3do=hieroglyphs (default 3do)\n");
				printf("  --menu      : menu type, pc=text, 3do=graphical "
						"(default 3do)\n");
				printf("  --font      : font types and colors (default pc)\n");
				printf("  --scroll    : ff/frev during comm.  pc=per-page, "
						"3do=smooth (default pc)\n");
				return 0;
			break;
		}
	}
	
	initIO();
	prepareContentDir (contentDir, addons);
	HFree ((void*)addons);
	prepareConfigDir ();
	prepareMeleeDir ();
	prepareSaveDir ();
	initTempDir();
	
	InitTimeSystem ();
	InitTaskSystem ();

	GraphicsLock = CreateMutex ("Graphics", SYNC_CLASS_TOPLEVEL | SYNC_CLASS_VIDEO);
	RenderingCond = CreateCondVar ("DCQ empty", SYNC_CLASS_TOPLEVEL | SYNC_CLASS_VIDEO);

	TFB_InitGraphics (gfxdriver, gfxflags, width, height, bpp);
	init_communication ();
	if (gammaset) 
		TFB_SetGamma (gamma);
	/* TODO: Once threading is gone, restore initAudio here.
	   initAudio calls AssignTask, which currently blocks on
	   ProcessThreadLifecycles... */
	// initAudio (snddriver, soundflags);
	TFB_InitInput (TFB_INPUTDRIVER_SDL, 0);

	StartThread (Starcon2Main, NULL, 1024, "Starcon2Main");

	for (;;)
	{
		TFB_ProcessEvents ();
		ProcessThreadLifecycles ();
		TFB_FlushGraphics ();
	}

	unInitTempDir();
	uninitIO();
	exit(EXIT_SUCCESS);
}

