
#include <gtk/gtk.h>
#include "lyricview.h"

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
	gtk_widget_set_size_request(lyricview->vbox, allocation->width, -1);
//	FIXME: we should use gtk_layout_move() instead.

	gint width, height;
	x = GTK_WIDGET(widget)->allocation.width / 2 - GTK_WIDGET(lyricview->message_label)->allocation.width / 2;
	y = GTK_WIDGET(widget)->allocation.height / 2 - GTK_WIDGET(lyricview->message_label)->allocation.height / 2;

	GTK_WIDGET(lyricview->message_label)->allocation.x = x;
	GTK_WIDGET(lyricview->message_label)->allocation.y = y;
}


gboolean
on_lyricview_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	LyricView *lyricview = (LyricView *) widget;
        GValue value = {0};
        g_value_init(&value, G_TYPE_INT);
        gtk_container_child_get_property((GtkContainer *) widget, lyricview->vbox, "y", &value);
        lyricview->top = g_value_get_int(&value);
        lyricview->y = event->y;
	
	lyricview->dragging = TRUE;
        return FALSE;
}

gboolean
on_lyricview_button_release_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	LyricView *lyricview = (LyricView *) widget;
	lyricview->dragging = FALSE;
        
	int the_y = GTK_WIDGET(lyricview)->allocation.height / 2;


	GList *list = lyricview->ones;

	GList *previous = lyricview->current;
	GList *current = lyricview->current;

	if (!current)
		current = list;

	while (current && current->next && the_y >= ((LyricItem *)current->next->data)->label->allocation.y)
		current = current->next;
	while (current && current->prev && the_y < ((LyricItem *)current->data)->label->allocation.y)
		current = current->prev;

	if (previous != current)
		g_signal_emit (G_OBJECT (widget), lyricview_signals[TIME_CHANGE_SIGNAL], 0, GINT_TO_POINTER(((LyricItem *)current->data)->time));
	
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
	        int cury = event->y;
	        int newy = lyricview->top + cury - lyricview->y;

	        int height;

	        height = widget->allocation.height;

		int vbox_height;
	        if (newy < - lyricview->vbox->allocation.height + height / 2)
	                newy = - lyricview->vbox->allocation.height + height / 2;
	        if (newy > height / 2)
	                newy = height / 2;
	        gtk_layout_move((GtkLayout *) widget, lyricview->vbox, 0, newy);
	}
        return FALSE;
}


static void
lyricview_init (LyricView *ttt)
{
	ttt->vbox = gtk_vbox_new (TRUE, 5);
	gtk_widget_show(ttt->vbox);
	gtk_layout_put(GTK_LAYOUT(ttt), ttt->vbox, 0, 66);
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


	gtk_widget_set_events (GTK_WIDGET(ttt), GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

}

GtkWidget *lyricview_new ()
{
	return GTK_WIDGET ( gtk_type_new (lyricview_get_type ()));
}

void lyricview_append_text(LyricView *lyricview, gint time, const gchar *text)
{
	LyricItem *item = g_new(LyricItem, 1);
	item->time = time;
	item->text = g_strdup(text);
	item->label = gtk_label_new(item->text);
	GdkColor color;
	gdk_color_parse("darkblue", &color);
	gtk_widget_modify_fg(item->label, GTK_STATE_NORMAL, &color);

	gtk_widget_show (item->label);
	gtk_box_pack_start (GTK_BOX (lyricview->vbox), item->label, FALSE, FALSE, 0);
	lyricview->ones = g_list_append(lyricview->ones, item);

	gtk_widget_hide(lyricview->message_label);
	gtk_widget_show(lyricview->vbox);
}

void lyricview_set_message(LyricView *lyricview, gchar *message)
{
	gtk_label_set_text(GTK_LABEL(lyricview->message_label), message);
}

void lyricview_set_current_time(LyricView *lyricview, gint time)
{
	GList *list = lyricview->ones;

	GList *previous = lyricview->current;
	GList *current = lyricview->current;

	if (!current)
		current = list;

	while (current && current->next && time >= ((LyricItem *)current->next->data)->time)
		current = current->next;
	while (current && current->prev && time < ((LyricItem *)current->data)->time)
		current = current->prev;

	if (previous != current)
	{
		GdkColor color;
		gdk_color_parse("white", &color);
		gtk_widget_modify_fg(((LyricItem *)current->data)->label, GTK_STATE_NORMAL, &color);

		if (previous)
		{
			GdkColor color;
			gdk_color_parse("darkblue", &color);
			gtk_widget_modify_fg(((LyricItem *)previous->data)->label, GTK_STATE_NORMAL, &color);
		}

		lyricview->current = current;
	}

	// make the current line at middle
	if (current && !lyricview->dragging)
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
}

