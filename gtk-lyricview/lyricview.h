#ifndef __LYRICVIEW_H__
#define __LYRICVIEW_H__

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LYRIC_VIEW(obj)          GTK_CHECK_CAST (obj, lyricview_get_type (), LyricView)
#define LYRICVIEW_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, lyricview_get_type (), LyricViewClass)
#define IS_LYRICVIEW(obj)       GTK_CHECK_TYPE (obj, lyricview_get_type ())

typedef struct _LyricView       LyricView;
typedef struct _LyricViewClass  LyricViewClass;
typedef struct _LyricViewColors LyricViewColors;

typedef struct _LyricItem 	LyricItem;

struct _LyricItem
{
	gint time;
	gchar *text;
	GtkWidget *label;
	gboolean process_color;
};

struct _LyricViewColors
{
	GdkColor background;
	GdkColor normal;
	GdkColor current;
	GdkColor drag;
	GdkColor messages;
	GdkColor errors;
};

struct _LyricView
{
	GtkLayout layout;
	GtkWidget *vbox, *message_label;
	GList *ones;
	GList *current;
	gint top, left, x, y;
	gboolean dragging;
	LyricViewColors colors;
	gboolean horizontal;
};

struct _LyricViewClass
{
  GtkLayoutClass parent_class;
  void (* lyricview) (LyricView *ttt);
};

guint          lyricview_get_type        (void);
GtkWidget*     lyricview_new             (void);

void lyricview_set_lyric(gchar *xmltext);
void lyricview_set_message(LyricView *lyricview, gchar *message);
void lyricview_append_text(LyricView *, gint time, const gchar *text);
void lyricview_set_current_time(LyricView *, gint time);
void lyricview_clear(LyricView *);
void lyricview_overall_adjust_by(LyricView *, gint);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LYRICVIEW_H__ */

