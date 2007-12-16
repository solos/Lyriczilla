
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "lyricview.h"

#define _(x) (x)

enum {
  TIME_CHANGE_SIGNAL,
  LAST_SIGNAL
};

static void lyricview_class_init          (LyricViewClass *klass);
static void lyricview_init                (LyricView      *ttt);

static gint lyricview_signals[LAST_SIGNAL] = { 0 };

guint
lyricview_get_type ()
{
  static GType ttt_type = 0;

  if (!ttt_type)
    {
      static const GTypeInfo ttt_info =
      {
	sizeof (LyricViewClass),
	NULL, /* base_init */
	NULL, /* base_finalize */
	(GClassInitFunc) lyricview_class_init,
	NULL, /* class_finalize */
	NULL, /* class_data */
	sizeof (LyricView),
	0,    /* n_preallocs */
	(GInstanceInitFunc) lyricview_init,
      };

      ttt_type = g_type_register_static (GTK_TYPE_LAYOUT,
                                         "LyricView",
                                         &ttt_info,
                                         0);
    }

  return ttt_type;
}

static void
lyricview_class_init (LyricViewClass *klass)
{
  lyricview_signals[TIME_CHANGE_SIGNAL] =
    g_signal_new ("time_change",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (LyricViewClass, lyricview),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

void
on_lyricview_size_allocate               (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data)
{
	LyricView *lyricview = (LyricView *) widget;
	int x, y;
	x = GTK_WIDGET(lyricview->message_label)->allocation.x;
	y = GTK_WIDGET(lyricview->message_label)->allocation.y;
	if (lyricview->horizontal)
		gtk_widget_set_size_request(lyricview->vbox, -1, allocation->height);
	else
		gtk_widget_set_size_request(lyricview->vbox, allocation->width, -1);
	//	FIXME: we should use gtk_layout_move() instead.

	gint width, height;
	x = GTK_WIDGET(widget)->allocation.width / 2 - GTK_WIDGET(lyricview->message_label)->allocation.width / 2;
	y = GTK_WIDGET(widget)->allocation.height / 2 - GTK_WIDGET(lyricview->message_label)->allocation.height / 2;

	GTK_WIDGET(lyricview->message_label)->allocation.x = x;
	GTK_WIDGET(lyricview->message_label)->allocation.y = y;
}

GtkWidget *get_widget(char *widgetname);

#define IMPORT_WIDGET(widget) GtkWidget *widget = get_widget(#widget)


void on_menu_search_activate(GtkMenuItem *menuitem,
                                                        gpointer     user_data)
{
	gtk_widget_show(get_widget("searchwin"));
}

void treeview_lyric_add_header()
{
	static gboolean added = FALSE;
	
	if (added)
		return;
	added = TRUE;
	
	GtkWidget *treeview_lyric = get_widget("treeview_lyric");
	
	GtkTreeViewColumn *column;
	

	GtkCellRenderer *cell;
	
	cell = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Title"));
	gtk_tree_view_column_pack_start(column, cell, TRUE);
	gtk_tree_view_column_set_attributes(column, cell, "text", 0, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview_lyric), column);
	
	cell = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(column, _("Artist"));
	gtk_tree_view_column_pack_start(column, cell, TRUE);
	gtk_tree_view_column_set_attributes(column, cell, "text", 1, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview_lyric), column);
}


void on_search_lyric_list_arrive(GPtrArray *result)
{
	IMPORT_WIDGET(label_status);
	IMPORT_WIDGET(treeview_lyric);
	if (!result)
	{
		gtk_label_set_text(GTK_LABEL(label_status), _("Error while querying lyric list."));
	}
	else if (result->len > 0)
	{
		printf("len: %d\n", result->len);
		treeview_lyric_add_header();
		
		GtkListStore* store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_lyric), GTK_TREE_MODEL(store));

		int i;
		GtkTreeIter iter;
			
		for (i = 0; i < result->len; i++)
		{
			GHashTable *hash = (GHashTable *)result->pdata[i];
			
			gchar *title = (gchar *) g_hash_table_lookup(hash, (gpointer) "title");
			gchar *artist = (gchar *) g_hash_table_lookup(hash, (gpointer) "artist");
			gchar *url = (gchar *) g_hash_table_lookup(hash, (gpointer) "url");
			
			printf("append: %s %s %s\n", title, artist, url);
			
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, 0, title, 1, artist, 2, url, -1);
			
		}
		gtk_label_set_text(GTK_LABEL(label_status), "");
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(label_status), _("No results."));
	}	
}


void on_button_find_clicked(GtkButton *button, gpointer user_data)
{
	IMPORT_WIDGET(entry_title);
	IMPORT_WIDGET(entry_artist);
	IMPORT_WIDGET(label_status);
	
	gtk_label_set_text(GTK_LABEL(label_status), _("Searching..."));
	
	const char *title = gtk_entry_get_text(GTK_ENTRY(entry_title));
	const char *artist = gtk_entry_get_text(GTK_ENTRY(entry_artist));
	
	GetLyricList_async(FALSE, NULL, title, artist, on_search_lyric_list_arrive);
}


void add_to_widget(gpointer data, gpointer user_data);


void on_search_lyric_arrive(GPtrArray *result)
{
	IMPORT_WIDGET(label_status);

	on_lyric_arrive(result);
	gtk_label_set_text(GTK_LABEL(label_status), "");	
}


void on_button_ok_clicked(GtkButton *button, gpointer user_data)
{
	IMPORT_WIDGET(label_status);
	IMPORT_WIDGET(treeview_lyric);
	
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_lyric));
	
	int num = gtk_tree_selection_count_selected_rows(sel);
	
	printf("num = %d\n", num);
	if (num == 1)
	{
		GtkTreeStore* store         = NULL;
		GtkTreeIter iter            = {0};
		GValue value                = {0};
	
		gtk_label_set_text(GTK_LABEL(label_status), _("Downloading..."));
	
		gtk_tree_selection_get_selected(sel, (GtkTreeModel**)&store, &iter);

		gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter, 2, &value);

		const char *url = g_value_get_string(&value);
		
		printf("url = %s\n", url);
		GetLyric_async(TRUE, url, on_search_lyric_arrive);
	}
}

void on_button_close_clicked(GtkButton *button, gpointer user_data)
{
	IMPORT_WIDGET(searchwin);
	gtk_widget_hide(searchwin);
}

GtkWidget *get_widget(char *widgetname)
{
	static GladeXML *xml = NULL;
	if (!xml)
	{
		xml = glade_xml_new("/usr/share/lyriczilla/ui.glade", NULL, NULL);
//		glade_xml_signal_autoconnect(xml);
		
		
#define CONNECT(signal_func) glade_xml_signal_connect(xml, #signal_func, (GCallback)(signal_func))
		CONNECT(gtk_widget_hide_on_delete);

		CONNECT(on_menu_search_activate);
		CONNECT(on_button_find_clicked);
		CONNECT(on_button_ok_clicked);
		CONNECT(on_button_close_clicked);
#undef CONNECT		
		
		
//		g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL); 
	}
	return glade_xml_get_widget(xml, widgetname);
}

gboolean
on_lyricview_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	LyricView *lyricview = (LyricView *) widget;
	if (event->button == 1) // left
	{
		GValue value = {0};
		g_value_init(&value, G_TYPE_INT);
		gtk_container_child_get_property((GtkContainer *) widget, lyricview->vbox, "y", &value);
		lyricview->top = g_value_get_int(&value);
		gtk_container_child_get_property((GtkContainer *) widget, lyricview->vbox, "x", &value);        
		lyricview->left = g_value_get_int(&value);
		lyricview->x = event->x;
		lyricview->y = event->y;
		lyricview->dragging = TRUE;
	}
	else
	{
		IMPORT_WIDGET(lyricview_menu);
		gtk_menu_popup(GTK_MENU(lyricview_menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	}
        return FALSE;
}

gboolean
on_lyricview_button_release_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	LyricView *lyricview = (LyricView *) widget;
	lyricview->dragging = FALSE;
        


	GList *list = lyricview->ones;

	GList *previous = lyricview->current;
	GList *current = lyricview->current;

	if (!current)
		current = list;

	int x = widget->allocation.height / 2;
	int y = widget->allocation.height / 2;
	
	if (lyricview->horizontal)
	{
		while (current && current->next && x >= ((LyricItem *)current->next->data)->label->allocation.x)
			current = current->next;
		while (current && current->prev && x < ((LyricItem *)current->data)->label->allocation.x)
			current = current->prev;	
	}
	else
	{
		while (current && current->next && y >= ((LyricItem *)current->next->data)->label->allocation.y)
			current = current->next;
		while (current && current->prev && y < ((LyricItem *)current->data)->label->allocation.y)
			current = current->prev;
	}

	if (previous != current)
	{
		int time = ((LyricItem *)current->data)->time;
		// TODO: count the height of the label.
		
		g_signal_emit (G_OBJECT (widget), lyricview_signals[TIME_CHANGE_SIGNAL], 0, GINT_TO_POINTER(time));
		lyricview_set_current_time((LyricView *)lyricview, time);
	}
	
        return FALSE;
}

gboolean
on_lyricview_motion_notify_event         (GtkWidget       *widget,
                GdkEventMotion  *event,
                gpointer         user_data)
{
	LyricView *lyricview = (LyricView *) widget;
	
	if (lyricview->dragging)
	{
		if (lyricview->horizontal)
		{
			int newx = lyricview->left + event->x - lyricview->x;
			int width = widget->allocation.width;

			if (newx < -lyricview->vbox->allocation.width + width / 2)
				newx = -lyricview->vbox->allocation.width + width / 2;
			if (newx > width / 2)
				newx = width / 2;
			gtk_layout_move(GTK_LAYOUT(widget), lyricview->vbox, newx, 0);
		}
		else
		{
			int newy = lyricview->top + event->y - lyricview->y;
			int height = widget->allocation.height;

			if (newy < - lyricview->vbox->allocation.height + height / 2)
				newy = - lyricview->vbox->allocation.height + height / 2;
			if (newy > height / 2)
				newy = height / 2;
			gtk_layout_move((GtkLayout *) widget, lyricview->vbox, 0, newy);
		}
	}
	return FALSE;
}

gboolean on_lyricview_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	LyricView *lyricview = LYRIC_VIEW(widget);
	if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_LEFT)
		lyricview_overall_adjust_by(lyricview, 300);
	else
		lyricview_overall_adjust_by(lyricview, -300);
}

static void lyricview_init (LyricView *ttt)
{
	ttt->horizontal = 0;
	if (ttt->horizontal)
		ttt->vbox = gtk_hbox_new(FALSE, 5);
	else
		ttt->vbox = gtk_vbox_new(TRUE, 5);
	gtk_widget_show(ttt->vbox);
	gtk_layout_put(GTK_LAYOUT(ttt), ttt->vbox, 0, 0);
	ttt->message_label = gtk_label_new(NULL);
	GdkColor color;
	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(ttt->message_label, GTK_STATE_NORMAL, &color);

	gtk_layout_put(GTK_LAYOUT(ttt), ttt->message_label, 0, 0);

	ttt->ones = NULL;
	ttt->current = NULL;

	ttt->dragging = FALSE;


	g_signal_connect ((gpointer) ttt, "size_allocate",
			G_CALLBACK (on_lyricview_size_allocate),
			NULL);
	g_signal_connect ((gpointer) ttt, "button_press_event",
			G_CALLBACK (on_lyricview_button_press_event),
			NULL);
	g_signal_connect ((gpointer) ttt, "button_release_event",
			G_CALLBACK (on_lyricview_button_release_event),
			NULL);
	g_signal_connect ((gpointer) ttt, "motion_notify_event",
			G_CALLBACK (on_lyricview_motion_notify_event),
			NULL);
	g_signal_connect ((gpointer) ttt, "scroll_event",
			G_CALLBACK (on_lyricview_scroll_event),
			NULL);

	gtk_widget_set_events (GTK_WIDGET(ttt), GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	lyricview_set_message(ttt, "LyricZilla");
}

GtkWidget *lyricview_new ()
{
	return GTK_WIDGET ( gtk_type_new (lyricview_get_type ()));
}

inline gint number_between(gint from, gint to, gdouble rate)
{
	return from + rate * (to - from);
}

void color_between(GdkColor *from, GdkColor *to, gdouble rate, GdkColor *result)
{
	result->pixel = number_between(from->pixel, to->pixel, rate);
	result->red = number_between(from->red, to->red, rate);
	result->green = number_between(from->green, to->green, rate);
	result->blue = number_between(from->blue, to->blue, rate);
}

void lyricview_append_text(LyricView *lyricview, gint time, const gchar *text)
{
	LyricItem *item = g_new(LyricItem, 1);
	item->time = time;
	item->text = g_strdup(text);
	item->label = gtk_label_new(item->text);
	item->process_color = FALSE;
	gtk_widget_modify_fg(item->label, GTK_STATE_NORMAL, &lyricview->colors.normal);

	gtk_widget_show (item->label);
	gtk_box_pack_start (GTK_BOX (lyricview->vbox), item->label, FALSE, FALSE, 0);

	lyricview->ones = g_list_append(lyricview->ones, item);

	gtk_widget_hide(lyricview->message_label);
	gtk_widget_show(lyricview->vbox);
}

void lyricview_set_message(LyricView *lyricview, gchar *message)
{
	gtk_label_set_text(GTK_LABEL(lyricview->message_label), message);
	gtk_widget_queue_draw(GTK_WIDGET(lyricview));
}

void lyricview_set_current_time(LyricView *lyricview, gint time)
{
	GList *list = lyricview->ones;

	GList *previous = lyricview->current;
	GList *current = lyricview->current;
	int the_y = GTK_WIDGET(lyricview)->allocation.height / 2;

	if (!current)
		current = list;

	while (current && current->next && time >= ((LyricItem *)current->next->data)->time)
		current = current->next;
	while (current && current->prev && time < ((LyricItem *)current->data)->time)
		current = current->prev;

	if (previous != current)
		lyricview->current = current;

	// set colors
	int threshold = 500;
	GList *p;
	for (p = list; p; p = p->next)
	{
		gint p_time = ((LyricItem *)(p->data))->time;
		gint pn_time = p->next ? ((LyricItem *)(p->next->data))->time : 0;

		if (time < p_time - threshold || p->next && pn_time + threshold < time)
		{
			if (((LyricItem *)p->data)->process_color)
			{
				((LyricItem *)p->data)->process_color = FALSE;
				gtk_widget_modify_fg(((LyricItem *)p->data)->label, GTK_STATE_NORMAL, &lyricview->colors.normal);
			}
		}
		else if (time < p_time + threshold)
		{
			int delta = (time - p_time + threshold) / 2;
			GdkColor color;
			color_between(&lyricview->colors.normal, &lyricview->colors.current, (gdouble) delta / threshold, &color);
			gtk_widget_modify_fg(((LyricItem *)p->data)->label, GTK_STATE_NORMAL, &color);
			((LyricItem *)p->data)->process_color = TRUE;
		}
		else if (p->next && time < pn_time - threshold)
		{
			gtk_widget_modify_fg(((LyricItem *)p->data)->label, GTK_STATE_NORMAL, &lyricview->colors.current);
			((LyricItem *)p->data)->process_color = TRUE;
		}
		else if (p->next && time < pn_time + threshold)
		{
			int delta = (pn_time + threshold - time) / 2;
			GdkColor color;
			color_between(&lyricview->colors.normal, &lyricview->colors.current, (gdouble) delta / threshold, &color);
			gtk_widget_modify_fg(((LyricItem *)p->data)->label, GTK_STATE_NORMAL, &color);
			((LyricItem *)p->data)->process_color = TRUE;
		}
	}
	
	// make the current line at middle
	if (current && !lyricview->dragging)
	{
		if (lyricview->horizontal)
		{

			int newx = lyricview->vbox->allocation.x - GTK_WIDGET(((LyricItem *)current->data)->label)->allocation.x;
			newx += GTK_WIDGET(lyricview)->allocation.width / 2;
			newx -= GTK_WIDGET(((LyricItem *)current->data)->label)->allocation.width / 2;
			if (current->next)
			{
				int time_current = ((LyricItem *)current->data)->time;
				int time_next = ((LyricItem *)current->next->data)->time;
				int width = GTK_WIDGET(((LyricItem *)current->next->data)->label)->allocation.x -
					GTK_WIDGET(((LyricItem *)current->data)->label)->allocation.x;
				newx -= width * (time - time_current) / (time_next - time_current);
				newx += width / 2;
			}
			gtk_layout_move((GtkLayout *) lyricview, lyricview->vbox, newx, 0);

		}
		else
		{
			int newy = lyricview->vbox->allocation.y - GTK_WIDGET(((LyricItem *)current->data)->label)->allocation.y;
			newy += GTK_WIDGET(lyricview)->allocation.height / 2;
			newy -= GTK_WIDGET(((LyricItem *)current->data)->label)->allocation.height / 2;
			if (current->next)
			{
				int time_current = ((LyricItem *)current->data)->time;
				int time_next = ((LyricItem *)current->next->data)->time;
				int height = GTK_WIDGET(((LyricItem *)current->next->data)->label)->allocation.y -
					GTK_WIDGET(((LyricItem *)current->data)->label)->allocation.y;
				newy -= height * (time - time_current) / (time_next - time_current);
				newy += height / 2;
			}
			gtk_layout_move((GtkLayout *) lyricview, lyricview->vbox, 0, newy);
		}
	}
}

void lyricview_clear(LyricView *lyricview)
{
	GList *list = lyricview->ones;
	while (list)
	{
		LyricItem *item = (LyricItem *) list->data;
		g_free(item->text);
		gtk_widget_destroy(item->label);		
		GList *next = list->next;
		list = next;
	}
	g_list_free(lyricview->ones);
	lyricview->ones = lyricview->current = NULL;
	gtk_widget_hide(lyricview->vbox);
	gtk_widget_show(lyricview->message_label);
	
	// invisible before next time change
	gtk_layout_move((GtkLayout *) lyricview, lyricview->vbox, 100000000, 0);
}


void lyricview_overall_adjust_by(LyricView *widget, gint time)
{
	LyricView *lyricview = LYRIC_VIEW(widget);
	GList *list = lyricview->ones;
	while (list)
	{
		((LyricItem *)list->data)->time += time;
		list = list->next;
	}
}

// TODO: this function should be moved to another file.
void lyricview_set_meta_info(LyricView *widget, const char *filename, const char *title, const char *artist)
{
	IMPORT_WIDGET(entry_title);
	IMPORT_WIDGET(entry_artist);
	
	printf("setting %s %s %s...\n", filename, title, artist);
	gtk_entry_set_text(GTK_ENTRY(entry_title), title);
	gtk_entry_set_text(GTK_ENTRY(entry_artist), artist);

}



