
# https://www.sqlite.org/json1.html

import sqlite3
import json

db_name = 'yt.db'

def open():
    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()

    sql = 'create table stats (ts varchar(64), video_id varchar(64), data json)'
    try:
        cursor.execute(sql)
        conn.commit()
    except sqlite3.OperationalError:
        # maybe table already exists
        pass
    return conn

def put(conn, now, video_id, stats):
    cursor = conn.cursor()
    sql = 'insert into stats values (?,?,?)'
    cursor.execute(sql, (now, video_id, json.dumps(stats)))
    conn.commit()
