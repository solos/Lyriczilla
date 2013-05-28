import lyriczilla.player
import dbus


def quod_get_info():
    try:
        daemon = dbus.SessionBus().get_object('net.sacredchao.QuodLibet',
                                              '/net/sacredchao/QuodLibet')
        info = daemon.CurrentSong()
        return {'title': info['title'],
                'artist': info['artist'],
                'filename': None,
                'time': 0}
    except:
        return None

lyriczilla.player.register('Quod Libet', quod_get_info)
