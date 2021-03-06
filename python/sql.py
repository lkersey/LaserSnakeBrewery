import sqlite3
import time
import numpy as np

db = '/home/pi/laser-snake-temp-control/fermentation_log.db'

def get_db(db_file):
    conn = sqlite3.connect(db_file)
    c = conn.cursor()
    return conn, c


def setup_db(db_file):
    conn, c = get_db(db_file)
    c.execute('''CREATE TABLE IF NOT EXISTS fermentation_log
    (timestmp NUMBER PRIMARY KEY,
     vat_temp FLOAT NOT NULL,
     fridge_temp FLOAT NOT NULL,
     set_temp FLOAT NOT NULL,
     phase NUMBER NOT NULL)'''
    )
    conn.commit()


def get_history():
    """
    Select all points in the database that were added within the last
    24 hours.
    """
    conn, c = get_db(db)
    time_frame = time.time() - 86400.0
    try:
        c.execute('''SELECT * FROM fermentation_log WHERE timestmp>?
                ORDER BY timestmp ASC''', (time_frame,))
        ret = np.asarray(c.fetchall())
    except:
        print 'Problem retrieving status'
        pass

    result = []
    for r in ret:
        timestamp = r[0]
        vat_temp = r[1]
        fridge_temp = r[2]
        set_temp = r[3]
        phase = r[4]
        result.append({'timestamp':timestamp, 'vat_temp':vat_temp,
            'fridge_temp':fridge_temp, 'set_temp':set_temp, 'phase':phase})
    return result


def get_status():
    """
    Returns the most recently added entry in the database.
    """
    conn, c = get_db(db)
    try:
        c.execute('''SELECT * FROM fermentation_log ORDER BY timestmp DESC limit 1''')
        ret = np.asarray(c.fetchall())
    except:
        print 'Problem retrieving status'
        pass

    result = []
    for r in ret:
        timestamp = r[0]
        vat_temp = r[1]
        fridge_temp = r[2]
        set_temp = r[3]
        phase = r[4]
        result.append({'timestamp':timestamp, 'vat_temp':vat_temp,
            'fridge_temp':fridge_temp, 'set_temp':set_temp, 'phase':phase})
    print result
    return result


def write_status(timestamp, vat_temp, fridge_temp, set_temp, phase):
    try:
        conn, c = get_db(db)
        c.execute('''INSERT INTO fermentation_log VALUES (?,?,?,?,?)''',
                (timestamp, vat_temp, fridge_temp, set_temp, phase))
        conn.commit()
    except:
        print 'Problem inserting data into' + str(db)




setup_db(db)
