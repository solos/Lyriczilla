# I will rewrite this file soon.

import os
import re
from re import findall
from string import replace, lower
import dbus
import threading
import cPickle


def dbus_name_running(name):
    try:
        session = dbus.SessionBus().get_object('org.freedesktop.DBus', '/')
        names = session.ListNames()
        return name in names
    except:
        return False


def async_call(func, args, callback, exception):
    def run(func, args, callback, exception):
        res = None
        try:
            res = func(*args)
        except Exception, e:
            exception(e)
        else:
            callback(res)
    thread = threading.Thread(target=run,
                              args=(func, args, callback, exception))
    thread.start()
    return thread


def lrctolist(lrc):
    #pattern = '\[(?P<type>.*?):(?P<value>.*?)\](?P<tail>.*)'
    pattern = '\[.*\](?P<tail>.?)'
    pt_value = '\[(?P<type>.+?):(?P<value>.+?)\]'
    rpl_tail = '\g<tail>'

    lrc = lrc.splitlines()
    #title = ''
    #artist = ''
    #album = ''
    #editor = ''
    offset = 0
    arrays = []
    for line in lrc:
        line = replace(line, "\n", "")
        line = replace(line, "\r", "")
        if line == "":
            continue
        if line[0] == '[':  # if a CH char start with [ may crash
            value = findall(pt_value, line)
            ly = re.sub(pattern, rpl_tail, line)
            if value != []:
                name = lower(value[0][0])
                namev = value[0][1]

                if name == "ti":
                    #title = namev
                    continue
                if name == 'ar':
                    #artist = namev
                    continue
                if name == 'al':
                    #album = namev
                    continue
                if name == 'by':
                    #editor = namev
                    continue
                if name == 'offset':
                    offset = int(namev)
                    continue
                if name == 'la':
                    continue
                if name == 'encoding':
                    continue
                for j in range(len(value)):
                    time = offset  # offset using here
                    if '.' in value[j][1]:
                        time += int(value[j][0])*60*1000
                        time += int(value[j][1].split('.', 2)[0])*1000 + \
                            int(value[j][1].split('.', 2)[1])
                    elif ':' in value[j][1]:
                        time += int(value[j][0])*60*1000
                        time += int(value[j][1].split(':', 2)[0])*1000 + \
                            int(value[j][1].split(':', 2)[1])*100/60
                    else:
                        time += int(value[j][0])*60*1000
                        time += int(value[j][1])*1000
                    if time < 0:
                        time = 0
                    arrays.append((time, ly))

    arrays.sort()
    return arrays


def makedirs_for_file(filename):
    try:
        os.makedirs(os.path.dirname(filename))
    except Exception, e:
        print e


def dump_object(obj, filename):
    try:
        os.makedirs(os.path.dirname(filename))
    except:
        pass
    try:
        cPickle.dump(obj, open(filename, 'w'))
    except:
        pass


def load_object(objtype, filename):
    try:
        return objtype(cPickle.load(open(filename)))
    except:
        return None


def dump_text(text, filename):
    try:
        os.makedirs(os.path.dirname(filename))
    except:
        pass
    try:
        open(filename, 'w').write(text.encode('utf-8'))
    except Exception:
        pass


def load_text(filename):
    try:
        return open(filename).read()
    except:
        return None
