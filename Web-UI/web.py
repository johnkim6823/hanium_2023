#sudo lsof -i :5000
#sudo kill -9 3125 3127

#sudo mysql -u root
#USE hanium
#SHOW TABLES;
#SELECT * FROM 2022_1009;
#DESC 2022_1009;
#SELECT HASH,Verified FROM 2022_1009 ORDER BY CID LIMIT 1;
from datetime import datetime
from crypt import methods
import socket
from flask import Flask,render_template, redirect, request, url_for,session,jsonify
from flask import abort
import pymysql
import sysv_ipc
import numpy as np
import struct
import os

from type_definitions import *



app = Flask("__name__")

    
#Go to start screen
@app.route('/')
def main():
    return render_template("cfg.html")

# Message queue delivery when CFG value is cif,30,YUV420
@app.route('/config_hd')
def config_cif():
     msg_size = HD
     msg_fps = 10
     msg_format = YUV420

     try:
        mq = sysv_ipc.MessageQueue(CAMERA_CFG_MQ, sysv_ipc.IPC_CREAT)
        
        # by HEADER
        header = bytearray(CAMERA_CONFIG_HEADER_SIZE)
        fmt_str = "<BBi" 
        struct.pack_into(fmt_str, header, 0, msg_size, msg_format, msg_fps) 
        print (header)
        print("cfg_hd sent")
        mq.send(header, True, type=TYPE_CAMERA_CFG)


     except sysv_ipc.ExistentialError:
        print("ERROR: message queue creation failed")
     return jsonify('1')
     
# Message queue delivery when CFG value is vga,30,YUV420
@app.route('/config_vga')
def config_vga():
     msg_size = VGA
     msg_fps = 10
     msg_format = YUV420

     try:
        mq = sysv_ipc.MessageQueue(CAMERA_CFG_MQ, sysv_ipc.IPC_CREAT)
        
        # by HEADER
        header = bytearray(CAMERA_CONFIG_HEADER_SIZE)
        fmt_str = "<BBi" 
        struct.pack_into(fmt_str, header, 0, msg_size, msg_format, msg_fps) 
        print (header)
        print("cfg_vga message sent")
        mq.send(header, True, type=TYPE_CAMERA_CFG)


     except sysv_ipc.ExistentialError:
        print("ERROR: message queue creation failed")
     return jsonify("1")

# If LOGGER responds to MQ, call recvMq
@app.route('/recvMq')
def recvMq():
    try:
        mq = sysv_ipc.MessageQueue(CAMERA_CFG_RES_MQ, sysv_ipc.IPC_CREAT)

        while True:
            message, mtype = mq.receive(block=False)
            print("*** New message received ***")
            print(f"Raw message: {message}")
            
            if mtype == TYPE_CAMERA_CFG_RES:
                camera_cfg_res = struct.unpack("<BBBB", message)[0]
                print(f"Logger Confirmed CFG. {camera_cfg_res}")
                return jsonify(camera_cfg_res)

    except sysv_ipc.BusyError as e:
        e="sysv_ipc error"
        print(e)
        

    except sysv_ipc.ExistentialError:
        print("ERROR: message queue creation failed")
        return 1

@app.route('/reqData')
def reqData():
    try:
        mq = sysv_ipc.MessageQueue(IMAGE_HASH_REQ_MQ, sysv_ipc.IPC_CREAT)
        
        # by HEADER
        header = bytearray(IMAGE_HASH_REQ_BUFF_SIZE)
        fmt_str = "<B" 
        struct.pack_into(fmt_str, header, 0, IMAGE_HASH_REQ) 
        print (header)
        print("Request message sent")
        mq.send(header, True, type=TYPE_IMAGE_HASH_REQ)
        return jsonify("1")


    except sysv_ipc.ExistentialError:
        print("ERROR: message queue creation failed")
    return jsonify("1")


@app.route('/resData')
def resData():
        try:
            mq = sysv_ipc.MessageQueue(IMAGE_HASH_RES_MQ, sysv_ipc.IPC_CREAT)

            while True:
                message, mtype = mq.receive(block=False)
            
                if mtype == TYPE_IMAGE_HASH_RES:
                    str_msg = message.decode('UTF-8', 'ignore')
                    return jsonify(str_msg)
                    

        except sysv_ipc.BusyError as e:
            e="sysv_ipc error"
            print(e)
            return render_template('BusyError.html')

        except sysv_ipc.ExistentialError:
            print("ERROR: message queue creation failed")

@app.route('/recvHash2')
def recvHash2():
        try:
            mq = sysv_ipc.MessageQueue(IMAGE_HASH_MQ, sysv_ipc.IPC_CREAT)

            while True:
                message, mtype = mq.receive()
            
                if mtype == TYPE_IMAGE_HASH:
                    str_msg = message.decode('UTF-8', 'ignore')
                    result=str_msg[0:63]
                    print(str_msg)
                    return render_template('vgaTab.html',hashmsg=result)

        except sysv_ipc.BusyError as e:
            e="sysv_ipc error"
            print(e)
            return render_template('BusyError.html')

        except sysv_ipc.ExistentialError:
            print("ERROR: message queue creation failed")

@app.route('/confirmHash')
def sndHash():
    try:
        mq = sysv_ipc.MessageQueue(IMAGE_HASH_RES_MQ , sysv_ipc.IPC_CREAT)
        
        # by HEADER
        header = bytearray(1)
        fmt_str = "<B" 
        struct.pack_into(fmt_str, header, 0, IMAGE_HASH_RES) 
        print (header)
        print("Confirmed Hashcode")
        mq.send(header, True, type=TYPE_IMAGE_HASH_RES)
        return jsonify('1')


    except sysv_ipc.ExistentialError:
        print("ERROR: message queue creation failed")
        



@app.route('/hdTab')
def hdTab():
    cid_db = pymysql.connect(
    user='hanium', 
    passwd='1234', 
    host='127.0.0.1', 
    db='hanium', 
    charset='utf8'
    )
    now = datetime.now()          #현재 시간
    a=now.strftime('%Y-%m%d')
    b = a.replace('-', '_'  )
    cursor = cid_db.cursor()
    #sql = "SEECT CID,HASH,Verified FROM 2022_1009 ORDER BY CID LIMIT 1;"
    #sql = "SELECT CID,HASH,Verified FROM 2022_1130 ORDER BY CID LIMIT 1;"
    sql = "SELECT CID,HASH,Verified FROM {} ORDER BY CID DESC LIMIT 1;".format(b)
    cursor.execute(sql)
    data_list = cursor.fetchall()
    cid_db.close()
    return render_template('hdTab.html',data_list=data_list)

@app.route('/vgaTab')
def vgaTab():
    cid_db = pymysql.connect(
    user='hanium', 
    passwd='1234', 
    host='127.0.0.1', 
    db='hanium', 
    charset='utf8'
    )
    now = datetime.now()          #현재 시간
    a=now.strftime('%Y-%m%d')
    b = a.replace('-', '_'  )
    cursor = cid_db.cursor()
    #sql = "SEECT CID,HASH,Verified FROM 2022_1009 ORDER BY CID LIMIT 1;"
    #sql = "SELECT CID,HASH,Verified FROM 2022_1130 ORDER BY CID LIMIT 1;"
    sql = "SELECT CID,HASH,Verified FROM {} ORDER BY CID DESC LIMIT 1;".format(b)
    cursor.execute(sql)
    data_list = cursor.fetchall()
    cid_db.close()
    return render_template('vgaTab.html',data_list=data_list)


@app.route("/BusyError")
def BusyError():
    return render_template('BusyError.html')

    

if __name__ == '__main__':     
    app.run(debug=True, host='0.0.0.0',port=8080)
