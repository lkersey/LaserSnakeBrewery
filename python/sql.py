import sqlite3
import time
import numpy as np

db = 'fermentation_log.db'


def get_db(db_file):
    conn = sqlite3.connect(db_file)
    c = conn.cursor()
    return conn, c


def setup_db(db_file):
    conn, c = get_db(db_file)
    c.execute('''CREATE TABLE IF NOT EXISTS fermentation_log
    (timestmp NUMBER PRIMARY KEY,
     vat_temp NUMBER NOT NULL,
     fridge_temp NUMBER NOT NULL,
     set_temp NUMBER NOT NULL,
     phase NUMBER NOT NULL)'''
    )
    conn.commit()

def get_status():
    conn, c = get_db(db)
    try:
        c.execute('''SELECT * FROM fermentation_log''')
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

    #times = list(ret[:, 0])
    #vat_temps = list(ret[:, 1])
    #fridge_temps = list(ret[:, 2])
    #set_temps = list(ret[:, 3])
    #states = list(ret[:, 4])

    #return {'timestamps':times, 'vat_temps':vat_temps,
    #'fridge_temps':fridge_temps, 'set_temps':set_temps, 'states':states}
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
#write_status(time.time(), 12.34, 11.18, 25.00, 1)
#write_status(time.time(), 12.33, 11.20, 25.00, 1)
