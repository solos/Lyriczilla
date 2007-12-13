#include <string.h>
#include <panel-applet.h>
#include <gtk/gtklabel.h>
#include <audacious/plugin.h>
#include <audacious/input.h>
#include <audacious/titlestring.h>
#include <libxml/xmlreader.h>

#include "../gtk-lyricview/lyricview.h"

gint timeout_id;
gchar *last_filename;
pid_t pid;



gboolean on_stage_2_data(GIOChannel *source, GIOCondition condition, gpointer data)
{
	LyricView *lyricview = (LyricView *) data;
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

		g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_2_data, lyricview);
	
	}
	else
		lyricview_set_message(lyricview, "Not found.");

	return FALSE;
}

gboolean load_local_lrc(GtkWidget *widget, const gchar *filename)
{
	LyricView *lyricview = LYRIC_VIEW(widget);
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
	g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_2_data, lyricview);


	return TRUE;

}


gboolean on_timeout(gpointer data)
{
	LyricView *lyricview = LYRIC_VIEW(data);
	if (xmms_remote_is_running(0))
	{
		gint playlist_pos = xmms_remote_get_playlist_pos(0);
		gchar *filename = (gchar *)xmms_remote_get_playlist_file(0, playlist_pos);
		printf("%s\n", filename);
		if (!last_filename || !filename || strcmp(last_filename, filename)) // currently playing another song.
		{
			g_free(last_filename);
			last_filename = filename;

			if (!load_local_lrc(lyricview, filename))
			{
				gchar *title, *artist;

				TitleInput *input = input_get_song_tuple(filename);
				title = strdup(input->track_name);
				artist = strdup(input->performer);
				bmp_title_input_free(input);

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

				printf("title artist: %s %s\n", title, artist);
				if (pid)
				{
					kill(pid, 9);
					pid = 0;
				}
				int pipefd;
				g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, NULL, &pipefd, NULL, NULL);
				GIOChannel *channel = g_io_channel_unix_new(pipefd);
				g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, on_stage_1_data, lyricview);
			}
		}

		gint time = xmms_remote_get_output_time(0);
		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	return TRUE;
}


void lyric_init(GtkWidget *widget)
{
	LyricView *lyricview = LYRIC_VIEW(widget);
//	g_signal_connect (G_OBJECT (lyricview), "time_change", G_CALLBACK (win), NULL);

	// make default colors
	gdk_color_parse("black", &LYRIC_VIEW(lyricview)->colors.background);
	gdk_color_parse("darkblue", &LYRIC_VIEW(lyricview)->colors.normal);
	gdk_color_parse("green", &LYRIC_VIEW(lyricview)->colors.current);

	gtk_widget_modify_bg(lyricview, GTK_STATE_NORMAL, &LYRIC_VIEW(lyricview)->colors.background);

	gtk_widget_show(lyricview);
	timeout_id = g_timeout_add(50, on_timeout, lyricview);
}

static const char Context_menu_xml [] =
   "<popup name=\"button3\">\n"
   "   <menuitem name=\"Properties Item\" "
   "             verb=\"ExampleProperties\" "
   "           _label=\"_Preferences...\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gtk-properties\"/>\n"
   "   <menuitem name=\"About Item\" "
   "             verb=\"ExampleAbout\" "
   "           _label=\"_About...\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gnome-stock-about\"/>\n"
   "</popup>\n";
   
static const BonoboUIVerb myexample_menu_verbs [] = {
//        BONOBO_UI_VERB ("ExampleProperties", display_properties_dialog),
//        BONOBO_UI_VERB ("ExampleAbout", display_about_dialog),
        BONOBO_UI_VERB_END
};

static gboolean
lyriczilla_applet_fill (PanelApplet *applet,
		   const gchar *iid,
		   gpointer     data)
{
        GtkWidget *widget;

        if (strcmp (iid, "OAFIID:LyricZilla_Applet") != 0)
		return FALSE;

        widget = lyricview_new();
        gtk_widget_set_size_request(applet, 200, 20);
        gtk_widget_show(widget);
	gtk_container_add (GTK_CONTAINER (applet), widget);
        gtk_widget_show(widget);

	gtk_widget_show_all (GTK_WIDGET (applet));
        gtk_widget_show(widget);
        lyric_init(widget);

	panel_applet_setup_menu (PANEL_APPLET (applet),
	                         Context_menu_xml,
	                         myexample_menu_verbs,
	                         NULL);



        return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:LyricZilla_Applet_Factory",
                             PANEL_TYPE_APPLET,
                             "LyricZillaApplet",
                             "0",
                             lyriczilla_applet_fill,
                             NULL);

