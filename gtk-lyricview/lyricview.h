#ifndef __LYRICVIEW_H__
#define __LYRICVIEW_H__

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LYRICVIEW(obj)          GTK_CHECK_CAST (obj, lyricview_get_type (), LyricView)
#define LYRICVIEW_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, lyricview_get_type (), LyricViewClass)
#define IS_LYRICVIEW(obj)       GTK_CHECK_TYPE (obj, lyricview_get_type ())

typedef struct _LyricView       LyricView;
typedef struct _LyricViewClass  LyricViewClass;


typedef struct _LyricItem 	LyricItem;

struct _LyricItem
{
	gint time;
	gchar *text;
	GtkWidget *label;
};


struct _LyricView
{
	GtkLayout layout;
	GtkWidget *vbox;
	GList *ones;
	GList *current;
	gint top, y;
	gboolean dragging;
};

struct _LyricViewClass
{
  GtkLayoutClass parent_class;
  void (* lyricview) (LyricView *ttt);
};

guint          lyricview_get_type        (void);
GtkWidget*     lyricview_new             (void);

void lyricview_set_lyric(gchar *xmltext);
void lyricview_set_message(gchar *message);
void lyricview_append_text(LyricView *, gint time, const gchar *text);
void lyricview_set_current_time(LyricView *, gint time);
void lyricview_clear(LyricView *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LYRICVIEW_H__ */

