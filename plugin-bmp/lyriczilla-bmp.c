#ifdef AUDACIOUS
#include <audacious/plugin.h>
#include <audacious/input.h>
#include <audacious/titlestring.h>
#endif
#ifdef BEEP_MEDIA_PLAYER
#include <bmp/plugin.h>
#include <bmp/titlestring.h>
#endif
#include <stdio.h>
#include <glib.h>
#include <libxml/xmlreader.h>
#include <dlfcn.h>
#include <string.h>

#include "../gtk-lyricview/lyricview.h"
#include "../version.h"
/*

   gpointer handle;
   gchar *filename;
   gint xmms_session;
   gchar *description;
   void (*init) (void);
   void (*about) (void);
   void (*configure) (void);
   void (*cleanup) (void);

*/

#define _

#ifdef AUDACIOUS
	#define BMPCONFIG_OFFSET 364
#endif
#ifdef BEEP_MEDIA_PLAYER
	#define BMPCONFIG_OFFSET 356
#endif

struct _BmpConfig {
#ifdef BMPCONFIG_OFFSET
	char we_just_want_to_use_the_following_two_fields[BMPCONFIG_OFFSET];
#else
#error "Please define your player!"
#endif
	gint titlestring_preset;
	gchar *gentitle_format;
}; typedef struct _BmpConfig BmpConfig;


void lyric_init();
void lyric_about();
void lyric_cleanup();

gint timeout_id;

GeneralPlugin lyric_gp =
{
	NULL,
	NULL,
	0,
	"LyricZilla",
	lyric_init,
	lyric_about,
	NULL,
	lyric_cleanup,
};

GeneralPlugin *get_gplugin_info(void)
{
	return &lyric_gp;
}

static void (*_input_get_song_info) (char *filename, char **title_real, int *len_real) = NULL;
BmpConfig *cfg_ptr;

GtkWidget *lyricwin, *lyricview;

pid_t pid = 0;

gboolean on_stage_2_data(GIOChannel *source, GIOCondition condition, gpointer data)
{
	gchar *str;
	gsize length;
	g_io_channel_read_to_end(source, &str, &length, NULL);

	kill(pid, 9);
	pid = 0;
	xmlDocPtr xmldoc = xmlParseMemory(str, length);
	if (!xmldoc)
	{
		lyricview_set_message(LYRIC_VIEW(lyricview), "Error while parsing lyric.");
		return FALSE;
	}
	xmlNodePtr song = xmldoc->children;
	char *id = NULL;
	while (song)
	{
		xmlNodePtr one = song->children;
		while (one)
		{
			if (xmlStrcmp(one->name, (xmlChar *) "one") == 0)
			{
				gint time = atoi(xmlGetProp(one, "time"));
				gchar *text = one->children ? (gchar *) one->children->content : "";
				
				lyricview_append_text((LyricView *)lyricview, time, text);
			}
			one = one->next;
		}
		song = song->next;
	}

	return FALSE;
}

gboolean on_stage_1_data(GIOChannel *source, GIOCondition condition, gpointer data)
{
	LyricView *lyricview = (LyricView *) data;
	gchar *str;
	gsize length;
	g_io_channel_read_to_end(source, &str, &length, NULL);

	xmlDocPtr xmldoc = xmlParseMemory(str, length);
	
	if (!xmldoc)
	{
		lyricview_set_message(LYRIC_VIEW(lyricview), "Error while searching.");
		return FALSE;
	}
	xmlNodePtr lyrics = xmldoc->children;
	char *id = NULL;
	while (lyrics)
	{
		xmlNodePtr lyric = lyrics->children;
		while (lyric)
		{
			if (xmlStrcmp(lyric->name, (xmlChar *) "lyric") == 0)
			{
				id = xmlGetProp(lyric, "id");
				break;
			}
			if (id)
				break;
			lyric = lyric->next;
		}
		if (id)
			break;
		lyrics = lyrics->next;
	}

	if (id)
	{
		gchar *argv[3] =
		{
			"lyriczilla",
			id,
			NULL,
		};
		kill(pid, 9);
		pid = 0;
		int pipefd;
		g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, NULL, &pipefd, NULL, NULL);
		GIOChannel *channel = g_io_channel_unix_new(pipefd);

		lyricview_set_message(lyricview, "Downloading lyric...");

		g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_2_data, NULL);
	
	}
	else
		lyricview_set_message(lyricview, "Not found.");

	return FALSE;
}

gboolean load_local_lrc(const gchar *filename)
{
	size_t pos = strlen((const char *) filename);

	while (pos > 0 && filename[pos] != '.')
		pos--;

	gchar *file = g_strndup(filename, pos);
	gchar *lrc_filename = g_strconcat(file, ".lrc", NULL);
	g_free(file);

	if (!g_file_test(lrc_filename, G_FILE_TEST_IS_REGULAR))
		return FALSE;

	lyricview_clear((LyricView *)lyricview);
	gchar *argv[4] =
	{
		"lyriczilla",
		"-l",
		lrc_filename,
		NULL,
	};

	if (pid)
	{
		kill(pid, 9);
		pid = 0;
	}
	int pipefd;
	g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, NULL, &pipefd, NULL, NULL);
	GIOChannel *channel = g_io_channel_unix_new(pipefd);
	lyricview_set_message((LyricView *) lyricview, "Loading local lyric...");
	g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_2_data, NULL);


	return TRUE;

}

gchar *last_filename = NULL;

gboolean on_timeout(gpointer data)
{
	gint session = lyric_gp.xmms_session;
	if (xmms_remote_get_playlist_length(session))
	{
		gint playlist_pos = xmms_remote_get_playlist_pos(session);
		gchar *filename = (gchar *)xmms_remote_get_playlist_file(session, playlist_pos);
		if (!last_filename || !filename || strcmp(last_filename, filename)) // currently playing another song.
		{
			g_free(last_filename);
			last_filename = filename;

			if (!load_local_lrc(filename))
			{
				char *title, *artist;
			
#ifdef AUDACIOUS
				TitleInput *input = input_get_song_tuple(filename);
				if (!input)
					return TRUE;
				title = strdup(input->track_name);
				artist = strdup(input->performer);
				bmp_title_input_free(input);
#endif
#ifdef BEEP_MEDIA_PLAYER
				gint orig_titlestring_preset = cfg_ptr->titlestring_preset;
				gchar *orig_gentitle_format = cfg_ptr->gentitle_format;
				cfg_ptr->titlestring_preset = 1000; // last one
				int len_real;

				cfg_ptr->gentitle_format = "%t";
				_input_get_song_info(filename, &title, &len_real);
				cfg_ptr->gentitle_format = "%p";
				_input_get_song_info(filename, &artist, &len_real);

				// restore them back
				cfg_ptr->titlestring_preset = orig_titlestring_preset;
				cfg_ptr->gentitle_format = orig_gentitle_format;
#endif

				lyricview_set_message((LyricView *)lyricview, "Searching for lyric...");

				// clear the old lyric
				lyricview_clear((LyricView *)lyricview);

				// we should load the lyric.
				gchar *argv[6] =
				{
					"lyriczilla",
					"-t",
					title,
					"-a",
					artist,
					NULL,
				};

				if (pid)
				{
					kill(pid, 9);
					pid = 0;
				}
				int pipefd;
				g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, NULL, &pipefd, NULL, NULL);
				GIOChannel *channel = g_io_channel_unix_new(pipefd);
				g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_1_data, lyricview);
				
				free(title);
				free(artist);
			}
		}

		gint time = xmms_remote_get_output_time(session);
		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	return TRUE;
}

void win( GtkWidget *widget, gint time, gpointer data)
{
	gint session = lyric_gp.xmms_session;
	xmms_remote_jump_to_time(session, time+1000);
}

void lyric_init()
{
	void *handle = dlopen(NULL, RTLD_LAZY);
	cfg_ptr = dlsym(handle, "cfg");
	_input_get_song_info = dlsym(handle, "input_get_song_info");
	if (!_input_get_song_info)
		return ;
	lyricwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title((GtkWindow *) lyricwin, "LyricZilla");
	gtk_widget_realize(lyricwin);
	lyricview = lyricview_new();
	g_signal_connect (G_OBJECT (lyricview), "time_change", G_CALLBACK (win), NULL);

	// make default colors
	gdk_color_parse("black", &LYRIC_VIEW(lyricview)->colors.background);
	gdk_color_parse("darkblue", &LYRIC_VIEW(lyricview)->colors.normal);
	gdk_color_parse("green", &LYRIC_VIEW(lyricview)->colors.current);

	gtk_container_add (GTK_CONTAINER(lyricwin), lyricview);
	gtk_widget_modify_bg(lyricview, GTK_STATE_NORMAL, &LYRIC_VIEW(lyricview)->colors.background);

	GtkWidget **mainwin_ptr = dlsym(handle, "mainwin");
	/*	gtk_window_set_transient_for(GTK_WINDOW(lyricwin), GTK_WINDOW(*mainwin_ptr));

		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(lyricwin), TRUE);
		*/
	g_signal_connect (G_OBJECT (*mainwin_ptr), "hide", G_CALLBACK(lyric_cleanup), NULL);

	gtk_widget_show(lyricview);
	gtk_widget_show(lyricwin);
	timeout_id = g_timeout_add(50, on_timeout, 0);
}

void lyric_about()
{
	const char *copyright =	"Copyright \xc2\xa9 2007 Liu Qishuai";
	const char *authors[] = {"Liu Qishuai <lqs.buaa@gmail.com>", "Yuki Lee <ykstars@gmail.com>", NULL};
	const gchar *license[] = {
		"LyricZilla is free software; you can redistribute it and/or modify "
			"it under the terms of the GNU General Public License as published by "
			"the Free Software Foundation; either version 2 of the License, or "
			"(at your option) any later version.",
		"LyricZilla is distributed in the hope that it will be useful, "
			"but WITHOUT ANY WARRANTY; without even the implied warranty of "
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
			"GNU General Public License for more details.",
		"You should have received a copy of the GNU General Public License "
			"along with LyricZilla; if not, write to the Free Software Foundation, Inc., "
			"51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA"
	};
	gchar *license_text = g_strjoin ("\n\n",
			_(license[0]), _(license[1]), _(license[2]), NULL);

	gtk_show_about_dialog (NULL,
			"name", "LyricZilla",
			//			"copyright", copyright,
			"comments", "Lyric",
			"version", VERSION,
			"authors", authors,
			"license", license_text,
			"wrap-license", TRUE,
			"translator-credits", _("translator-credits"),
			"logo-icon-name", "lyriczilla",
			NULL);
	g_free(license_text);
}

void lyric_cleanup()
{
	g_source_remove(timeout_id);
	g_free(last_filename);
	last_filename = NULL;
	gtk_widget_destroy(lyricwin);
}

