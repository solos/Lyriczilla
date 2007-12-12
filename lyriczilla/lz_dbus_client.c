#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

/*
void func(gpointer data, gpointer user_data)
{
	GHashTable *hash = (GHashTable *)data;
	gchar *title = (gchar *) g_hash_table_lookup(hash, (gpointer) "title");
	if (title)
		printf("%s\n", title);
}
*/

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

GPtrArray *GetLyricList(const gchar *title, const gchar *artist)
{
	DBusGProxy *proxy = lz_dbus_get_proxy();
	GError *error = NULL;
	GPtrArray *arr = NULL;
	
#define DBUS_TYPE_G_HASH_TABLE_ARRAY  (dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_STRING_STRING_HASHTABLE))

	if (!dbus_g_proxy_call (proxy, "GetLyricList", &error, G_TYPE_STRING, title, G_TYPE_STRING, artist, G_TYPE_STRING, "TEST", G_TYPE_INVALID,
				DBUS_TYPE_G_HASH_TABLE_ARRAY, &arr, G_TYPE_INVALID))
	{
		/* Just do demonstrate remote exceptions versus regular GError */
		if (error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION)
			g_printerr ("Caught remote method exception %s: %s",
					dbus_g_error_get_name (error),
					error->message);
		else
			g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
		exit (1);
	}

	return arr;
}


GPtrArray *GetLyric(const gchar *url)
{
	DBusGProxy *proxy = lz_dbus_get_proxy();
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
		exit (1);
	}
	return arr;
}


int main_old (int argc, char **argv)
{
	DBusGConnection *connection;
	GError *error;
	DBusGProxy *proxy;
	char **name_list;
	char **name_list_ptr;

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

	/* Create a proxy object for the "bus driver" (name "org.freedesktop.DBus") */

	printf("%s %s %s\n", DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS);

	proxy = dbus_g_proxy_new_for_name (connection,
			"com.googlecode.lyriczilla",
			"/LyricZilla",
			"com.googlecode.lyriczilla.LyricZilla");

	/* Call ListNames method, wait for reply */
	error = NULL;

	printf("%s\n", DBUS_TYPE_G_STRING_STRING_HASHTABLE);

	GPtrArray *arr;

	//#define DBUS_TYPE_G_STRING_STRING_HASHTABLE (dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_STRING))


#define DBUS_TYPE_G_HASH_TABLE_ARRAY  (dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_STRING_STRING_HASHTABLE))


	if (!dbus_g_proxy_call (proxy, "GetLyricList", &error, G_TYPE_STRING, "TEST", G_TYPE_STRING, "TEST", G_TYPE_STRING, "TEST", G_TYPE_INVALID,
				DBUS_TYPE_G_HASH_TABLE_ARRAY, &arr, G_TYPE_INVALID))
	{
		/* Just do demonstrate remote exceptions versus regular GError */
		if (error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION)
			g_printerr ("Caught remote method exception %s: %s",
					dbus_g_error_get_name (error),
					error->message);
		else
			g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
		exit (1);
	}

	/* Print the results */

	g_print ("Names on the message bus:\n");

	printf("%d\n", arr->len);

//	g_ptr_array_foreach(arr, func, NULL);

	g_ptr_array_free(arr, TRUE);

	g_object_unref (proxy);

	return 0;
}
