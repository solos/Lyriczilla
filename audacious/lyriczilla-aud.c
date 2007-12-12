#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <audacious/plugin.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "../gtk-lyricview/lyricview.h"

#define _

static void lyric_init();
static void lyric_about();
static void lyric_cleanup();

gint timeout_id;


static GeneralPlugin audaciouslyriczilla =
{
 .description = "LyricZilla Plugin",
 .init = lyric_init,
 .about = lyric_about,
 .cleanup = lyric_cleanup
}; 

GeneralPlugin *lyric_gp[] = {&audaciouslyriczilla, NULL};
SIMPLE_GENERAL_PLUGIN(lyriczilla, lyric_gp);

GtkWidget *lyricwin, *lyricview;

pid_t pid = 0;

gchar *last_filename = NULL;


gboolean auto_scroll = TRUE;

gboolean enable_auto_scroll_again(gpointer data)
{
	auto_scroll = TRUE;
	return FALSE;
}

void add_to_widget(gpointer data, gpointer user_data)
{
	GValueArray *valarr = (GValueArray *) data;
	
	int time = g_value_get_int(g_value_array_get_nth(valarr, 0));
	char *text = g_value_get_string(g_value_array_get_nth(valarr, 1));
	
	printf("%d %s\n", time, text);
	
	lyricview_append_text(lyricview, time, text);
	

/*
	GHashTable *hash = (GHashTable *)data;
	gchar *title = (gchar *) g_hash_table_lookup(hash, (gpointer) "title");
	if (title)
		printf("%s\n", title);
		*/
}

gboolean on_timeout(gpointer data)
{
	Playlist *playlist = aud_playlist_get_active();
	if (auto_scroll && playlist)
	{
		gint playlist_pos = aud_playlist_get_position(playlist);
		
		gchar *filename = (gchar *)aud_playlist_get_filename(playlist, playlist_pos);
		
		if (!last_filename || !filename || strcmp(last_filename, filename)) // playing another song.
		{
			g_free(last_filename);
			last_filename = filename;


			Tuple *out = aud_playlist_get_tuple(playlist, playlist_pos);
			
			char *title = (char *) aud_tuple_get_string(out, FIELD_TITLE, NULL);
			char *artist = (char *) aud_tuple_get_string(out, FIELD_ARTIST, NULL);
			
			printf("title artist: %s %s\n", title, artist);

			lyricview_set_message((LyricView *)lyricview, _("Searching for lyrics..."));

			// clear the old lyric
			lyricview_clear((LyricView *)lyricview);
			

			GPtrArray *result = GetLyricList(title, artist);
			
			printf("result = %d\n", result->len);
			
			if (result->len > 0)
			{
				GHashTable *hash = (GHashTable *)result->pdata[0];
				gchar *url = (gchar *) g_hash_table_lookup(hash, (gpointer) "url");
				if (url)
				{
					GPtrArray *arr = GetLyric(url);
					printf(":: %d\n", arr->len);
					
					int time;
					char *text;
					
					g_ptr_array_foreach(arr, add_to_widget, NULL);
					
				}
			}

/*
				// we should load the lyric.
				gchar *argv[6] =
				{
					"lyriczilla",
					"-t",
					title,
					NULL,
					NULL,
					NULL,
				};
				if (artist)
				{
					argv[3] = "-a";
					argv[4] = artist;
				}

				if (pid)
				{
					kill(pid, 9);
					pid = 0;
				}
				int pipefd;
				g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, NULL, &pipefd, NULL, NULL);
				GIOChannel *channel = g_io_channel_unix_new(pipefd);
				g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_1_data, lyricview);
							printf("6!!\n");
*/
			free(title);
			free(artist);

		}

		gint time = audacious_drct_get_time();//(playlist, playlist_pos);
		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	return TRUE;
}

void win( GtkWidget *widget, gint time, gpointer data)
{
	auto_scroll = FALSE;
	audacious_drct_seek(time);
	g_timeout_add(500, enable_auto_scroll_again, 0);
}

static GtkWidget *(*_ui_skinned_window_new)(const gchar *wmclass_name) = NULL;

static void lyric_init()
{
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

//	GtkWidget **mainwin_ptr = dlsym(handle, "mainwin");
	/*	gtk_window_set_transient_for(GTK_WINDOW(lyricwin), GTK_WINDOW(*mainwin_ptr));

		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(lyricwin), TRUE);
		*/
//	g_signal_connect (G_OBJECT (*mainwin_ptr), "hide", G_CALLBACK(lyric_cleanup), NULL);

	gtk_widget_show(lyricview);
	aud_dock_add_window(aud_get_dock_window_list(), (GtkWindow *)lyricwin);
	gtk_widget_show(lyricwin);
	timeout_id = g_timeout_add(50, on_timeout, 0);
}

static void lyric_about()
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

static void lyric_cleanup()
{
	g_source_remove(timeout_id);
	g_free(last_filename);
	last_filename = NULL;
	gtk_widget_destroy(lyricwin);
}

