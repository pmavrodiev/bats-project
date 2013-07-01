#!/usr/bin/python

import MySQLdb

boxes = ["10001","10002","10003","100011","100033","1000111","331","332","3311","33111","661","663","6611","1001","1002","1003"]
db = MySQLdb.connect(host="localhost", # your host, usually localhost
                     user="root", # your username
                     passwd="987654321", # your password
                     db="BATS") # name of the data base


for box in boxes:
    cur=db.cursor()
    cur.execute("SELECT * FROM  `Findings` WHERE fin_box = \""+box+"\";")
    print box;
    cursor = ""; prev_cursor=""; bats_list="";
    # print all the first cell of all the rows
    for row in cur.fetchall() :
        if (cursor == ""):
           prev_cursor = row[2]
        cursor=row[2]
        if (cursor != prev_cursor):
          print prev_cursor,":", bats_list
          prev_cursor = row[2]
          bats_list=""
        bats_list = bats_list + row[1] + ", "


