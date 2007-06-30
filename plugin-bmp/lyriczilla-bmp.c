#include <bmp/plugin.h>
#include <bmp/titlestring.h>
#include <stdio.h>
#include <glib.h>
#include <libxml/xmlreader.h>
#include <dlfcn.h>

#include "../gtk-lyricview/lyricview.h"

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
void lyric_cleanup();

gint timeout_id;

GeneralPlugin lyric_gp =
{
	NULL,
	NULL,
	0,
	"LyricZilla",
	lyric_init,
	NULL,
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

gboolean gio_one_func(GIOChannel *source, GIOCondition condition, gpointer data)
{
	gchar *str;
	gsize length;
	g_io_channel_read_to_end(source, &str, &length, NULL);

	xmlDocPtr xmldoc = xmlParseMemory(str, length);
	if (!xmldoc)
		return FALSE;
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



gboolean gio_func(GIOChannel *source, GIOCondition condition, gpointer data)
{
	LyricView *lyricview = (LyricView *) data;
	gchar *str;
	gsize length;
	g_io_channel_read_to_end(source, &str, &length, NULL);

	xmlDocPtr xmldoc = xmlParseMemory(str, length);
	
	if (!xmldoc)
		return FALSE;
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

		int pipefd;
		g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &pipefd, NULL, NULL);
		GIOChannel *channel = g_io_channel_unix_new(pipefd);

		lyricview_set_message(lyricview, "Downloading lyric...");

		g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, gio_one_func, NULL);
	
	}
	else
		lyricview_set_message(lyricview, "Not found.");

	return FALSE;
}



gboolean on_timeout(gpointer data)
{
	gint session = lyric_gp.xmms_session;
	gint playlist_pos = xmms_remote_get_playlist_pos(session);
	gchar *filename = (gchar *)xmms_remote_get_playlist_file(session, playlist_pos);
	static gchar *last_filename = NULL;

	if (!last_filename || !filename || strcmp(last_filename, filename)) // currently playing another song.
	{
		g_free(last_filename);
		last_filename = filename;

		gint orig_titlestring_preset = cfg_ptr->titlestring_preset;
		gchar *orig_gentitle_format = cfg_ptr->gentitle_format;
		cfg_ptr->titlestring_preset = 1000; // last one
		char *title, *artist;
		int len_real;

printf("orig = %d %s\n", orig_titlestring_preset, orig_gentitle_format);

		cfg_ptr->gentitle_format = "%t";
		_input_get_song_info(filename, &title, &len_real);
		cfg_ptr->gentitle_format = "%p";
		_input_get_song_info(filename, &artist, &len_real);

		// restore them back
		cfg_ptr->titlestring_preset = orig_titlestring_preset;
		cfg_ptr->gentitle_format = orig_gentitle_format;

		lyricview_set_message((LyricView *)lyricview, "Search for lyric...");

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
		int pipefd;
		g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &pipefd, NULL, NULL);
		GIOChannel *channel = g_io_channel_unix_new(pipefd);
		g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, gio_func, lyricview);

	}

	gint time = xmms_remote_get_output_time(session);
	lyricview_set_current_time((LyricView *)lyricview, time);

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
	g_signal_connect (G_OBJECT (lyricview), "time_change",
		    G_CALLBACK (win), NULL);
	gtk_container_add (GTK_CONTAINER(lyricwin), lyricview);
	gtk_widget_show(lyricview);
	gtk_widget_show(lyricwin);
	timeout_id = g_timeout_add(100, on_timeout, 0);



}

void lyric_cleanup()
{
	g_source_remove(timeout_id);
	gtk_widget_destroy(lyricwin);
}

