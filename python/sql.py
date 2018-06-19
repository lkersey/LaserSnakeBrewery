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


def get_vat_temperatures():
    conn, c = get_db(db)
    try:
        c.execute('''SELECT * FROM fermentation_log''')
        ret = np.asarray(c.fetchall())
    except:
        print 'Problem retrieving vat temperatures'
        pass

    temps = list(ret[:, 1])
    times = list(ret[:, 0])

    return {'vat_temp':temps, 'timestamps':times}


def write_status(timestamp, vat_temp, fridge_temp, set_temp, phase):
    try:
        conn, c = get_db(db)
        c.execute('''INSERT INTO fermentation_log VALUES (?,?,?,?,?)''',
                (timestamp, vat_temp, fridge_temp, set_temp, phase))
        conn.commit()
    except:
        print 'Problem inserting data into' + str(db)

setup_db(db)

