SUBDIRS = .

if SERVER
SUBDIRS += server
endif

if TOOLS
SUBDIRS += tools
endif

bin_PROGRAMS = wesnoth

if EDITOR
bin_PROGRAMS += wesnoth_editor
endif

if WZIP
SUBDIRS += zip
endif

pkgdatadir=$(datadir)/@DATADIR@

#############################################################################
#    Wesnoth                                                                #
#############################################################################

wesnoth_SOURCES = about.cpp \
                  actions.cpp \
		  ai.cpp \
		  ai_attack.cpp \
		  ai_move.cpp \
		  builder.cpp \
		  cavegen.cpp \
		  clipboard.cpp \
		  config.cpp \
		  cursor.cpp \
		  dialogs.cpp \
		  display.cpp \
		  events.cpp \
		  filesystem.cpp \
		  font.cpp \
		  game.cpp \
		  game_config.cpp \
		  game_events.cpp \
		  gamestatus.cpp \
		  halo.cpp \
          help.cpp \
		  hotkeys.cpp \
		  image.cpp \
		  intro.cpp \
		  key.cpp \
		  language.cpp \
		  log.cpp \
		  map.cpp \
		  map_label.cpp \
		  mapgen.cpp \
		  mapgen_dialog.cpp \
		  mouse.cpp \
		  multiplayer.cpp \
		  multiplayer_client.cpp \
		  multiplayer_connect.cpp \
		  multiplayer_lobby.cpp \
		  network.cpp \
		  pathfind.cpp \
		  playlevel.cpp \
		  playturn.cpp \
		  preferences.cpp \
		  race.cpp \
		  replay.cpp \
		  reports.cpp \
		  sdl_utils.cpp \
		  show_dialog.cpp \
		  sound.cpp \
		  statistics.cpp \
		  team.cpp \
		  terrain.cpp \
		  theme.cpp \
		  titlescreen.cpp \
		  tooltips.cpp \
		  unit.cpp \
		  unit_display.cpp \
		  unit_types.cpp \
		  video.cpp \
		  widgets/button.cpp \
		  widgets/file_chooser.cpp \
		  widgets/combo.cpp \
		  widgets/menu.cpp \
		  widgets/progressbar.cpp \
		  widgets/scrollbar.cpp \
		  widgets/slider.cpp \
		  widgets/textbox.cpp \
		  widgets/widget.cpp \
		  about.hpp \
		  actions.hpp \
		  ai.hpp \
		  ai_attack.hpp \
		  ai_move.hpp \
		  array.hpp \
		  builder.hpp \
		  cavegen.hpp \
		  clipboard.hpp \
		  cursor.hpp \
		  config.hpp \
		  dialogs.hpp \
		  display.hpp \
		  events.hpp \
		  filesystem.hpp \
		  font.hpp \
		  game.hpp \
		  game_config.hpp \
		  game_events.hpp \
		  gamestatus.hpp \
		  halo.hpp \
          help.hpp \
		  hotkeys.hpp \
		  image.hpp \
		  intro.hpp \
		  key.hpp \
		  language.hpp \
		  log.hpp \
		  map.hpp \
		  map_label.hpp \
		  mapgen.hpp \
		  mapgen_dialog.hpp \
		  mouse.hpp \
		  multiplayer.hpp \
		  multiplayer_client.hpp \
		  multiplayer_connect.hpp \
		  multiplayer_lobby.hpp \
		  network.hpp \
		  pathfind.hpp \
		  playlevel.hpp \
		  playturn.hpp \
		  preferences.hpp \
		  race.hpp \
		  replay.hpp \
		  reports.hpp \
		  scoped_resource.hpp \
		  sdl_utils.hpp \
		  show_dialog.hpp \
		  sound.hpp \
		  statistics.hpp \
		  team.hpp \
		  terrain.hpp \
		  theme.hpp \
		  titlescreen.hpp \
		  tooltips.hpp \
		  unit.hpp \
		  unit_display.hpp \
		  unit_types.hpp \
		  util.hpp \
		  video.hpp \
		  widgets/button.hpp \
		  widgets/file_chooser.hpp \
		  widgets/combo.hpp \
		  widgets/menu.hpp \
		  widgets/progressbar.hpp \
		  widgets/scrollbar.hpp \
		  widgets/slider.hpp \
		  widgets/textbox.hpp \
		  widgets/widget.hpp

#############################################################################
#    Editor                                                                 #
#############################################################################

wesnoth_editor_SOURCES = editor/editor.cpp \
			 editor/editor_layout.cpp \
			 editor/map_manip.cpp \
			 editor/editor_palettes.cpp \
			 editor/editor_main.cpp \
			 editor/editor_dialogs.cpp \
			 editor/editor_undo.cpp \
			 about.cpp \
			 actions.cpp \
			 ai.cpp \
			 ai_attack.cpp \
			 ai_move.cpp \
		         builder.cpp \
		 	 cavegen.cpp \
			 clipboard.cpp \
			 config.cpp \
			 cursor.cpp \
			 dialogs.cpp \
			 display.cpp \
			 events.cpp \
			 filesystem.cpp \
			 font.cpp \
			 game_config.cpp \
			 game_events.cpp \
			 gamestatus.cpp \
			 halo.cpp \
             help.cpp \
			 hotkeys.cpp \
			 image.cpp \
			 intro.cpp \
			 key.cpp \
			 language.cpp \
			 log.cpp \
		         map_label.cpp \
			 map.cpp \
			 mapgen.cpp \
			 mapgen_dialog.cpp \
			 mouse.cpp \
			 network.cpp \
			 pathfind.cpp \
			 playlevel.cpp \
			 playturn.cpp \
			 preferences.cpp \
			 race.cpp \
			 replay.cpp \
			 reports.cpp \
			 sdl_utils.cpp \
			 show_dialog.cpp \
			 sound.cpp \
			 statistics.cpp \
			 team.cpp \
			 terrain.cpp \
			 theme.cpp \
			 tooltips.cpp \
			 unit.cpp \
		         unit_display.cpp \
			 unit_types.cpp \
			 video.cpp \
			 widgets/button.cpp \
			 widgets/file_chooser.cpp \
			 widgets/menu.cpp \
			 widgets/progressbar.cpp \
			 widgets/textbox.cpp \
			 widgets/scrollbar.cpp \
			 widgets/slider.cpp \
			 widgets/widget.cpp \
			 editor/editor.hpp \
			 editor/editor_layout.hpp \
			 editor/map_manip.hpp \
			 editor/editor_palettes.hpp \
			 editor/editor_dialogs.hpp \
			 editor/editor_undo.hpp \
			 about.hpp \
			 actions.hpp \
			 ai.hpp \
			 ai_attack.hpp \
			 ai_move.hpp \
			 array.hpp \
		         builder.hpp \
		 	 cavegen.hpp \
			 clipboard.hpp \
			 config.hpp \
			 cursor.hpp \
			 dialogs.hpp \
			 display.hpp \
			 events.hpp \
			 filesystem.hpp \
			 font.hpp \
			 game_config.hpp \
			 game_events.hpp \
			 gamestatus.hpp \
			 halo.hpp \
             help.hpp \
			 hotkeys.hpp \
			 image.hpp \
			 intro.hpp \
			 key.hpp \
			 language.hpp \
			 log.hpp \
			 map_label.hpp \
			 map.hpp \
			 mapgen.hpp \
			 mapgen_dialog.hpp \
			 mouse.hpp \
			 network.hpp \
			 pathfind.hpp \
			 playlevel.hpp \
			 playturn.hpp \
			 preferences.hpp \
			 race.hpp \
			 replay.hpp \
			 reports.hpp \
			 sdl_utils.hpp \
			 show_dialog.hpp \
			 sound.hpp \
			 statistics.hpp \
			 team.hpp \
			 terrain.hpp \
			 theme.hpp \
			 tooltips.hpp \
			 unit.hpp \
		         unit_display.hpp \
			 unit_types.hpp \
			 video.hpp \
			 widgets/button.hpp \
			 widgets/file_chooser.hpp \
			 widgets/menu.hpp \
			 widgets/progressbar.hpp \
			 widgets/textbox.hpp \
			 widgets/scrollbar.hpp \
			 widgets/slider.hpp \
			 widgets/widget.hpp

AM_CXXFLAGS = @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\"

if X11
    CXXFLAGS += -D_X11 @X_CFLAGS@
endif
if GCC
    CXXFLAGS += -fno-omit-frame-pointer
endif

AM_LDFLAGS = @SDL_LIBS@ @SDL_IMAGE_LIBS@ @SDL_MIXER_LIBS@ @SDL_NET_LIBS@ \
	@SDL_TTF_LIBS@

if STATIC
LIBS += -static -L/usr/lib -Wl,-rpath,/usr/lib -L/usr/X11R6/lib -lSDL_image \
-lSDL_mixer -lSDL_net -lSDL_ttf -lSDL -lpthread -ldl -laudio -lXt -lX11 -lXext \
-lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lstdc++ -lm -lSM -lICE -ljpeg \
-lpng -lz -lvorbisfile -lvorbis -logg -lsmpeg -lfreetype
endif
