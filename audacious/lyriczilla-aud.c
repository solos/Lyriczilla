#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <audacious/plugin.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <glade/glade.h>
#include "../gtk-lyricview/lyricview.h"

#include <dlfcn.h>

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
gchar *last_filename = NULL;

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

int state = 0;

void on_lyric_arrive(GPtrArray *result)
{
	g_ptr_array_foreach(result, add_to_widget, NULL);
}


void on_lyric_list_arrive(GPtrArray *result)
{
	if (!result)
	{
		lyricview_set_message((LyricView *)lyricview, _("Error while querying lyric list."));
	}
	else if (result->len > 0)
	{
		GHashTable *hash = (GHashTable *)result->pdata[0];
		gchar *url = (gchar *) g_hash_table_lookup(hash, (gpointer) "url");
		if (url)
			GetLyric_async(url, on_lyric_arrive);
		else
			lyricview_set_message((LyricView *)lyricview, _("Error while parse query result."));

		state = 0;
	}
	else
	{
		lyricview_set_message((LyricView *)lyricview, _("Cannot found any lyric matching this song."));
		state = 0;
	}	
}

gboolean on_timer(gpointer data)
{
	Playlist *playlist = aud_playlist_get_active();
	if (playlist)
	{
		gint playlist_pos = aud_playlist_get_position(playlist);
		
		
		gchar *filename = (gchar *)aud_playlist_get_filename(playlist, playlist_pos);
		
		if (filename && (!last_filename || strcmp(last_filename, filename))) // playing another song.
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
		
			state = 1;
			
			GetLyricList_async(title, artist, on_lyric_list_arrive);
		}

		gint time = audacious_drct_get_time();
		printf("time is %d\n", time);
		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	return TRUE;
}

static void on_lyricview_dragdrop( GtkWidget *widget, gint time, gpointer data)
{
	time += 1000 - time % 1000;  // round up
	audacious_drct_seek(time);
}

static GtkWidget *(*_ui_skinned_window_new)(const gchar *wmclass_name) = NULL;

static void lyric_init()
{
	lyricwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title((GtkWindow *) lyricwin, "LyricZilla");
	gtk_widget_realize(lyricwin);
	lyricview = lyricview_new();
	g_signal_connect (G_OBJECT (lyricview), "time_change", G_CALLBACK (on_lyricview_dragdrop), NULL);

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
	
	gtk_window_set_keep_above(GTK_WINDOW(lyricwin), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(lyricwin), TRUE);	
//	gtk_window_set_decorated(GTK_WINDOW(lyricwin), FALSE);


	gtk_widget_show(lyricwin);
	timeout_id = g_timeout_add(50, on_timer, 0);
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

