#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

DBusGProxy *lz_dbus_get_proxy()
{
	DBusGConnection *connection;
	GError *error;
	static DBusGProxy *proxy = NULL;

	if (proxy == NULL)
	{
		g_type_init ();

		error = NULL;
		connection = dbus_g_bus_get (DBUS_BUS_SESSION,
				&error);
		if (connection == NULL)
		{
			g_printerr ("Failed to open connection to bus: %s\n",
					error->message);
			g_error_free (error);
			exit (1);
		}

		/* Create a proxy object for the "bus driver" */
		proxy = dbus_g_proxy_new_for_name (connection,
				"com.googlecode.lyriczilla",
				"/LyricZilla",
				"com.googlecode.lyriczilla.LyricZilla");
	}
	return proxy;
}

DBusGProxyCall *pending_call = NULL;


static void GetLyricList_async_callback(DBusGProxy *proxy, DBusGProxyCall *call, void *(*callback) (GPtrArray *))
{
	GError *error = NULL;

	pending_call = NULL;
	
#define DBUS_TYPE_G_HASH_TABLE_ARRAY  (dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_STRING_STRING_HASHTABLE))
	
	GPtrArray *arr = NULL;
	if (!dbus_g_proxy_end_call(proxy, call, &error, DBUS_TYPE_G_HASH_TABLE_ARRAY, &arr, G_TYPE_INVALID))
	{
		/* Just do demonstrate remote exceptions versus regular GError */
		if (error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION)
			g_printerr ("Caught remote method exception %s: %s",
					dbus_g_error_get_name (error),
					error->message);
		else
			g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
//		exit (1);
	}
	callback(arr);
}


void GetLyricList_async(const gchar *title, const gchar *artist, void *(*callback) (GPtrArray *))
{
	DBusGProxy *proxy = lz_dbus_get_proxy();
	
	if (pending_call)
		dbus_g_proxy_cancel_call(proxy, pending_call);
	
	GError *error = NULL;
	pending_call = dbus_g_proxy_begin_call (proxy, "GetLyricList", GetLyricList_async_callback, callback, NULL, G_TYPE_STRING, title, G_TYPE_STRING, artist, G_TYPE_STRING, "TEST", G_TYPE_INVALID);
}


GPtrArray *GetLyric(const gchar *url)
{
	DBusGProxy *proxy = lz_dbus_get_proxy();
	
	if (pending_call)
		dbus_g_proxy_cancel_call(proxy, pending_call);
		
	GError *error = NULL;
	GPtrArray *arr = NULL;
	
	GType inner = dbus_g_type_get_struct ("GValueArray", G_TYPE_INT, G_TYPE_STRING, G_TYPE_INVALID);
	GType outer = dbus_g_type_get_collection("GPtrArray", inner);
	
	if (!dbus_g_proxy_call (proxy, "GetLyric", &error, G_TYPE_STRING, url, G_TYPE_STRING, "TEST", G_TYPE_INVALID,
				outer, &arr, G_TYPE_INVALID))
	{
		/* Just do demonstrate remote exceptions versus regular GError */
		if (error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION)
			g_printerr ("Caught remote method exception %s: %s",
					dbus_g_error_get_name (error),
					error->message);
		else
			g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
//		exit (1);
	}
	return arr;
}


