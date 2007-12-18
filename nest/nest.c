#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "../gtk-lyricview/lyricview.h"
#include <libmcs/mcs.h>
#define _



static GtkWidget *lyricwin = NULL;
GtkWidget *lyricview = NULL;
static gchar *last_uri = NULL;

static void add_to_widget(gpointer data, gpointer user_data)
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

static gboolean on_timer(gpointer data)
{
	GHashTable *ht = Player_GetInfo();
	
	if (ht)
	{

		gint time = atoi((gchar *) g_hash_table_lookup(ht, (gpointer) "time"));
		gchar *title = (gchar *) g_hash_table_lookup(ht, (gpointer) "title");
		gchar *artist = (gchar *) g_hash_table_lookup(ht, (gpointer) "artist");
		gchar *uri = (gchar *) g_hash_table_lookup(ht, (gpointer) "uri");
	
		printf("%d %s %s %s\n", time, title, artist, uri);
			
		if (uri && (!last_uri || strcmp(last_uri, uri))) // playing another song.
		{
			g_free(last_uri);
			last_uri = uri;

			lyricview_set_meta_info(LYRIC_VIEW(lyricview), uri, title, artist);

			lyricview_set_message(LYRIC_VIEW(lyricview), _("Searching for lyrics..."));

			// clear the old lyric
			lyricview_clear((LyricView *)lyricview);
		
			GetLyricList_async(TRUE, uri, title, artist, on_lyric_list_arrive);
		}

		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	return TRUE;
}

static void on_lyricview_dragdrop( GtkWidget *widget, gint time, gpointer data)
{
	time += 1000 - time % 1000;  // round up
	//audacious_drct_seek(time);
}



static void load_settings(LyricView *lyricview, const char *profile)
{
	printf("about to load %s\n", profile);
	mcs_handle_t *mcs = mcs_new("lyriczilla");

	char *font_desc = "Sans 12";
	
	mcs_get_string(mcs, profile, "font", &font_desc);
	printf("font is %s\n", font_desc);
	
	LyricViewColors colors;

	char *colorstr;

	colorstr = "black";
	mcs_get_string(mcs, profile, "color_background", &colorstr);
	printf("got: %s\n", colorstr);
	gdk_color_parse(colorstr, &colors.background);
	
	colorstr = "blue";
	mcs_get_string(mcs, profile, "color_normal", &colorstr);
	gdk_color_parse(colorstr, &colors.normal);
	
	colorstr = "white";
	mcs_get_string(mcs, profile, "color_current", &colorstr);
	gdk_color_parse(colorstr, &colors.current);
	
	colorstr = "yellow";
	mcs_get_string(mcs, profile, "color_messages", &colorstr);
	gdk_color_parse(colorstr, &colors.messages);
	
	mcs_destroy(mcs);
	lyricview_set_style(lyricview, font_desc, &colors);
}


void save_settings(LyricView *lyricview, const char *profile)
{
	printf("about to save %s\n", profile);
	mcs_handle_t *mcs = mcs_new("lyriczilla");
	
	char *font_desc = pango_font_description_to_string(lyricview->font);
	printf("font is %s\n", font_desc);
	mcs_set_string(mcs, profile, "font", font_desc);
	g_free(font_desc);

	LyricViewColors colors;
	
	const char *colorstr;
	
	colorstr = gdk_color_to_string(&lyricview->colors.background);
	mcs_set_string(mcs, profile, "color_background", colorstr);
	colorstr = gdk_color_to_string(&lyricview->colors.normal);
	mcs_set_string(mcs, profile, "color_normal", colorstr);
	colorstr = gdk_color_to_string(&lyricview->colors.current);
	mcs_set_string(mcs, profile, "color_current", colorstr);
	colorstr = gdk_color_to_string(&lyricview->colors.messages);
	mcs_set_string(mcs, profile, "color_messages", colorstr);
	
	mcs_destroy(mcs);
}

static gchar *profile = "lyriczilla";


static gboolean on_timer_save_settings()
{
	save_settings(lyricview, profile);
	return TRUE;
}

static gint timeout_id;

int main(int argc, char *argv[])
{
	gtk_init (&argc, &argv);
	mcs_init();
	
	if (argc == 2)
		lyricwin = gtk_plug_new(atoi(argv[1]));
	else
		lyricwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

//	gtk_window_set_title((GtkWindow *) lyricwin, "LyricZilla");
	gtk_widget_realize(lyricwin);
	lyricview = lyricview_new();
	g_signal_connect (G_OBJECT (lyricview), "time_change", G_CALLBACK (on_lyricview_dragdrop), NULL);


	load_settings(lyricview, profile);

	gtk_container_add (GTK_CONTAINER(lyricwin), lyricview);
	gtk_widget_modify_bg(lyricview, GTK_STATE_NORMAL, &LYRIC_VIEW(lyricview)->colors.background);

	gtk_widget_show(lyricview);
	
	gtk_widget_show(lyricwin);
	
	timeout_id = g_timeout_add(50, on_timer, 0);
	
	g_signal_connect(G_OBJECT(lyricwin), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	g_timeout_add(2000, on_timer_save_settings, 0);

	gtk_main();
	mcs_fini();
	return 0;
}

