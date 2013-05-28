import lyriczilla.player
import dbus


def audacious_get_info():
    try:
        player = dbus.SessionBus().get_object('org.atheme.audacious',
                                              '/Player')

        time = player.PositionGet()
        info = player.GetMetadata()

        return {'title': info['title'],
                'artist': info['artist'],
                'uri': info['URI'],
                'time': time}
    except:
        return None


def audacious_seek(time):
    try:
        player = dbus.SessionBus().get_object('org.atheme.audacious',
                                              '/Player')
        player.PositionSet(time)
    except:
        pass

lyriczilla.player.register('Audacious', audacious_get_info, audacious_seek)
