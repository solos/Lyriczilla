#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "../gtk-lyricview/lyricview.h"

#define _

gint timeout_id;

static GtkWidget *lyricwin = NULL, *lyricview = NULL;
static gchar *last_filename = NULL;

void add_to_widget(gpointer data, gpointer user_data)
{
	GValueArray *valarr = (GValueArray *) data;
	
	int time = g_value_get_int(g_value_array_get_nth(valarr, 0));
	const char *text = g_value_get_string(g_value_array_get_nth(valarr, 1));
	
	lyricview_append_text(LYRIC_VIEW(lyricview), time, text);
}

void on_lyric_arrive(GPtrArray *result)
{
	lyricview_clear(LYRIC_VIEW(lyricview));
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
			GetLyric_async(TRUE, url, on_lyric_arrive);
		else
			lyricview_set_message((LyricView *)lyricview, _("Error while parse query result."));
	}
	else
	{
		lyricview_set_message((LyricView *)lyricview, _("No lyrics found."));
	}
}

gboolean on_timer(gpointer data)
{
	void *playlist = NULL;//Playlist *playlist = aud_playlist_get_active();
	if (lyricwin && playlist)
	{
		gint playlist_pos = aud_playlist_get_position(playlist);
		
		
		gchar *filename = NULL;//(gchar *)aud_playlist_get_filename(playlist, playlist_pos);
		
		if (filename && (!last_filename || strcmp(last_filename, filename))) // playing another song.
		{
			g_free(last_filename);
			last_filename = filename;

			void *out = NULL;//Tuple *out = NULL;//aud_playlist_get_tuple(playlist, playlist_pos);

			char *title = NULL;//(char *) aud_tuple_get_string(out, FIELD_TITLE, NULL);
			char *artist = NULL;//(char *) aud_tuple_get_string(out, FIELD_ARTIST, NULL);
			
			printf("title artist: %s %s\n", title, artist);
			
			lyricview_set_meta_info(LYRIC_VIEW(lyricview), filename, title, artist);

			lyricview_set_message(LYRIC_VIEW(lyricview), _("Searching for lyrics..."));

			// clear the old lyric
			lyricview_clear((LyricView *)lyricview);
		
			GetLyricList_async(TRUE, filename, title, artist, on_lyric_list_arrive);
		}

		gint time = 0;//audacious_drct_get_time();
		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	return TRUE;
}

static void on_lyricview_dragdrop( GtkWidget *widget, gint time, gpointer data)
{
	time += 1000 - time % 1000;  // round up
	//audacious_drct_seek(time);
}

int main(int argc, char *argv[])
{
	gtk_init (&argc, &argv);
	
	if (argc == 2)
		lyricwin = gtk_plug_new(atoi(argv[1]));
	else
		lyricwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title((GtkWindow *) lyricwin, "LyricZilla");
	gtk_widget_realize(lyricwin);
	lyricview = lyricview_new();
	g_signal_connect (G_OBJECT (lyricview), "time_change", G_CALLBACK (on_lyricview_dragdrop), NULL);

	// make default colors
	gdk_color_parse("black", &LYRIC_VIEW(lyricview)->colors.background);
	gdk_color_parse("blue", &LYRIC_VIEW(lyricview)->colors.normal);
	gdk_color_parse("white", &LYRIC_VIEW(lyricview)->colors.current);

	gtk_container_add (GTK_CONTAINER(lyricwin), lyricview);
	gtk_widget_modify_bg(lyricview, GTK_STATE_NORMAL, &LYRIC_VIEW(lyricview)->colors.background);

	gtk_widget_show(lyricview);
	
	gtk_widget_show(lyricwin);
	
	timeout_id = g_timeout_add(50, on_timer, 0);
	
	g_signal_connect(G_OBJECT(lyricwin), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	gtk_main();
	return 0;
}

