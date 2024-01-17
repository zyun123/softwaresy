#!/usr/bin/env python

import sys
import socket
import threading
import struct
import time
from scipy import signal
import iir_filter
import rtde.rtde as rtde
import rtde.rtde_config as rtde_config
import json
import zmq
import math

mutex = [threading.Lock(), threading.Lock()]
mutex2 = [threading.Lock(), threading.Lock()]
g_wrench_l = [0, 0, 0, 0, 0, 0]
g_wrench_r = [0, 0, 0, 0, 0, 0]
bias_flag_l = 0
bias_flag_r = 0
sndToPlot = True

def list_to_setp(setp, list):
	for i in range(0,6):
		setp.external_force_torque[i] = list[i]
	return setp

def mean(arr):
    sum = 0
    for eachi in arr:
        sum += eachi
    return sum / len(arr)

def t_rtde(ip, rbindex):
    contttt = zmq.Context()
    sockkk = contttt.socket(zmq.PUB)
    if rbindex == 0:
        sockkk.bind('tcp://192.168.1.100:9871')
    else:
        sockkk.bind('tcp://192.168.1.100:9872')
    ROBOT_HOST = ip
    ROBOT_PORT = 30004
    config_filename = 'set_external_wrench.xml'
    conf = rtde_config.ConfigFile(config_filename)
    state_names, state_types = conf.get_recipe('state')
    setp_names, setp_types = conf.get_recipe('setp')
    watchdog_names, watchdog_types = conf.get_recipe('watchdog')
    frequencyset=500
    con = rtde.RTDE(ROBOT_HOST, ROBOT_PORT)
    con.connect()
    print('UR_connetok')
    con.get_controller_version()
    con.send_output_setup(state_names, state_types,frequency = frequencyset)
    setp = con.send_input_setup(setp_names, setp_types)
    watchdog = con.send_input_setup(watchdog_names, watchdog_types)
    setp.external_force_torque = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    watchdog.input_int_register_0 = 0
    if not con.send_start():
        print('RTDE con fail')
        sys.exit()
    while True:
        state = con.receive()
        if state is None:
            print('RTDE con fail')
            break
        mutex[rbindex].acquire()
        if rbindex == 0:
            global g_wrench_l
            wrench_filtered = g_wrench_l
        else:
            global g_wrench_r
            wrench_filtered = g_wrench_r
        mutex[rbindex].release()
        dataaaa = {'wrench': [wrench_filtered[0], wrench_filtered[1], wrench_filtered[2], wrench_filtered[3], wrench_filtered[4], wrench_filtered[5]]}
        sockkk.send_string(json.dumps(dataaaa))
        list_to_setp(setp, wrench_filtered)
        con.send(setp)
        con.send(watchdog)
	
    con.send_pause()
    print('RTDE pause')
    con.disconnect()
    print('RTDE discon')
    
def t_udp(port, rbindex):
    if sndToPlot == True:
        if port == 8886:
            contttt1 = zmq.Context()
            sockkk1 = contttt1.socket(zmq.PUB)
            sockkk1.bind('tcp://192.168.1.100:9873')
    if sndToPlot == True:
        if port == 8885:
            contttt2 = zmq.Context()
            sockkk2 = contttt2.socket(zmq.PUB)
            sockkk2.bind('tcp://192.168.1.100:9874')
    bias = [0, 0, 0]
    with open('bias.json', 'r') as f:
        obj = json.loads(f.read())
        f.close()
    for i in range(3):
        key = str(rbindex)
        arr = obj[key]
        bias[i] = arr[i]
    print(rbindex, 'ini bias:', bias)
    bias_cache_x = []
    bias_cache_y = []
    bias_cache_z = []
    sos1 = signal.cheby1(4, 0.5, [56.6, 76.6], 'bandstop', output='sos', fs=1000)
    sos2 = signal.cheby1(4, 0.5, 33, 'lowpass', output='sos', fs=1000)
    iir1 = []
    iir2 = []
    for i in range(6):
        iir1.append(iir_filter.IIR_filter(sos1))
        iir2.append(iir_filter.IIR_filter(sos2))

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(('192.168.1.100', port))
    # s.sendto(b'\x51\xaa\x0d\x0a', ('192.168.1.111', 5152))#切换查询发送和持续发送模式
    t1 = time.time()
    rcv_buf = bytearray()
    while True:
        data = s.recv(1024)
        rcv_buf += data
        if len(rcv_buf) < 28:
            continue
        if (len(rcv_buf) == 28 and rcv_buf[0] == 0x48 and rcv_buf[1] == 0xaa and rcv_buf[26] == 0x0d and rcv_buf[27] == 0x0a) == False:
            print('data error:', rbindex, len(rcv_buf), rcv_buf)
            rcv_buf = bytearray()
            continue
        wrench = [0,0,0,0,0,0]
        wrench_filtered = []
        for i in range(6):
            raw = rcv_buf[2+i*4 : 6+i*4]
            raw.reverse()
            wrench[i] = struct.unpack("!f", raw)[0] * 9.8
        if sndToPlot == True:
            if port == 8886:
                data = {'wrench': [wrench[0], wrench[1], wrench[2], wrench[3], wrench[4], wrench[5]]}
                sockkk1.send_string(json.dumps(data))
        if sndToPlot == True:
            if port == 8885:
                data = {'wrench': [wrench[0], wrench[1], wrench[2], wrench[3], wrench[4], wrench[5]]}
                sockkk2.send_string(json.dumps(data))
        for i in range(6):
            wrench_filtered.append(iir2[i].filter(iir1[i].filter(wrench[i])))
            # wrench_filtered.append(wrench[i])
        # if sndToPlot == True:
        #     if port == 8885:
        #         data = {'wrench_filtered': [wrench_filtered[0], wrench_filtered[1], wrench_filtered[2], wrench_filtered[3], wrench_filtered[4], wrench_filtered[5]]}
        #         sockkk.send_string(json.dumps(data))
        mutex2[rbindex].acquire()
        if rbindex == 0:
            global bias_flag_l
            tmp_flag = bias_flag_l
            bias_flag_l = 0
        else:
            global bias_flag_r
            tmp_flag = bias_flag_r
            bias_flag_r = 0
        mutex2[rbindex].release()
        if tmp_flag == 1:
            bias_cache_x.append(-wrench_filtered[0])
            bias_cache_y.append(-wrench_filtered[1])
        if tmp_flag == 2:
            bias_cache_z.append(-wrench_filtered[2])
        if tmp_flag == 3:
            bias[0] = mean(bias_cache_x)
            bias[1] = mean(bias_cache_y)
            bias[2] = mean(bias_cache_z)
            bias_cache_x = []
            bias_cache_y = []
            bias_cache_z = []
            print(rbindex, bias[0], bias[1], bias[2])
            with open('bias.json', 'r') as f:
                obj = json.loads(f.read())
                f.close()
                key = str(rbindex)
                obj[key] = bias
                with open('bias.json', 'w') as f:
                    f.write(json.dumps(obj))
                    f.close()
        wrench_filtered[0] += bias[0]
        wrench_filtered[1] += bias[1]
        wrench_filtered[2] += bias[2]
        # if port == 8885:
        #     print('{:0.3f}'.format(wrench_filtered[0]), '{:0.3f}'.format(wrench_filtered[1]), '{:0.3f}'.format(wrench_filtered[2]))
        # wrench_filtered = [10,10,10,4,5,6]
        mutex[rbindex].acquire()
        if rbindex == 0:
            global g_wrench_l
            g_wrench_l = wrench_filtered
        else:
            global g_wrench_r
            g_wrench_r = wrench_filtered
        mutex[rbindex].release()
        # print((time.time() - t1) * 1000)
        t1 = time.time() 
        rcv_buf = bytearray()
        
def t_tcp_yuli(ip, rbindex):
    if sndToPlot == True:
        if rbindex == 0:
            contttt = zmq.Context()
            sockkk = contttt.socket(zmq.PUB)
            sockkk.bind('tcp://192.168.1.100:9873')
    bias = [0, 0, 0]
    bias_cache_x = []
    bias_cache_y = []
    bias_cache_z = []
    sos1 = signal.cheby1(4, 0.5, [56.6, 76.6], 'bandstop', output='sos', fs=1000)
    sos2 = signal.cheby1(4, 0.5, 33, 'lowpass', output='sos', fs=1000)
    iir1 = []
    iir2 = []
    for i in range(6):
        iir1.append(iir_filter.IIR_filter(sos1))
        iir2.append(iir_filter.IIR_filter(sos2))

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(1)
    try:
        s.connect(('192.168.1.201', 4008))
    except Exception:
        print(b"connect_error")
    s.send(b'AT+GSD=STOP\r\n')
    time.sleep(1)
    s.send(b'AT+SGDM=(A01,A02,A03,A04,A05,A06);E;1;(WMA:1)\r\n')
    time.sleep(1)
    s.send(b'AT+SMPR=1000\r\n')
    time.sleep(1)
    s.send(b'AT+GSD\r\n')
    t1 = time.time()
    while True:
        rcv_buf = bytearray()
        try:
            data = s.recv(1024)
        except socket.timeout:
            print('no data!')
            continue
        rcv_buf += data
        if len(rcv_buf) != 31:
            continue
        wrench = [0,0,0,0,0,0]
        wrench_filtered = [0,0,0,0,0,0]
        wrench_filtered_raw = []
        for i in range(6):
            raw = rcv_buf[6+i*4 : 10+i*4]
            raw.reverse()
            wrench[i] = struct.unpack("!f", raw)[0]
        for i in range(6):
            wrench_filtered_raw.append(iir2[i].filter(iir1[i].filter(wrench[i])))
            # wrench_filtered_raw.append(wrench[i])
        wrench_filtered[0] = -wrench_filtered_raw[0] * math.sqrt(2) / 2 - wrench_filtered_raw[1] * math.sqrt(2) / 2
        wrench_filtered[1] = wrench_filtered_raw[0] * math.sqrt(2) / 2 - wrench_filtered_raw[1] * math.sqrt(2) / 2
        wrench_filtered[2] = wrench_filtered_raw[2]
        wrench_filtered[3] = -wrench_filtered_raw[3] * math.sqrt(2) / 2 - wrench_filtered_raw[4] * math.sqrt(2) / 2
        wrench_filtered[4] = wrench_filtered_raw[3] * math.sqrt(2) / 2 - wrench_filtered_raw[4] * math.sqrt(2) / 2
        wrench_filtered[5] = wrench_filtered_raw[5]
        
        if sndToPlot == True:
            if rbindex == 0:
                jdat = {'wrench': [wrench[0], wrench[1], wrench[2], wrench[3], wrench[4], wrench[5]]}
                sockkk.send_string(json.dumps(jdat))
            
        # if sndToPlot == True:
        #     if rbindex == 0:
        #         jdat = {'wrench_filtered': [wrench_filtered[0], wrench_filtered[1], wrench_filtered[2], wrench_filtered[3], wrench_filtered[4], wrench_filtered[5]]}
        #         sockkk.send_string(json.dumps(jdat))
            
        mutex2[rbindex].acquire()
        if rbindex == 0:
            global bias_flag_l
            tmp_flag = bias_flag_l
            bias_flag_l = 0
        else:
            global bias_flag_r
            tmp_flag = bias_flag_r
            bias_flag_r = 0
        mutex2[rbindex].release()
        if tmp_flag == 1:
            bias_cache_x.append(-wrench_filtered[0])
            bias_cache_y.append(-wrench_filtered[1])
        if tmp_flag == 2:
            bias_cache_z.append(-wrench_filtered[2])
        if tmp_flag == 3:
            bias[0] = mean(bias_cache_x)
            bias[1] = mean(bias_cache_y)
            bias[2] = mean(bias_cache_z)
            bias_cache_x = []
            bias_cache_y = []
            bias_cache_z = []
            print(ip, bias[0], bias[1], bias[2])
        wrench_filtered[0] += bias[0]
        wrench_filtered[1] += bias[1]
        wrench_filtered[2] += bias[2]
        # if ip == '192.168.1.201':
        #     print('{:0.3f}'.format(wrench_filtered[0]), '{:0.3f}'.format(wrench_filtered[1]), '{:0.3f}'.format(wrench_filtered[2]))
        # wrench_filtered = [1,2,3,4,5,6]
        mutex[rbindex].acquire()
        if rbindex == 0:
            global g_wrench_l
            g_wrench_l = wrench_filtered
        else:
            global g_wrench_r
            g_wrench_r = wrench_filtered
        mutex[rbindex].release()
        # print((time.time() - t1) * 1000)
        t1 = time.time()
        
def t_cali_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
    server.bind(('127.0.0.1', 8680))
    server.listen(128)
    while True:
        client, addr = server.accept()
        th = threading.Thread(target=t_cali_subthread, args=(client, addr))
        th.setDaemon(True)
        th.start()
    server.close()
        
def t_cali_subthread(client, addr):
    # rcv_cnt = 0
    while 1:
        raw = client.recv(1024)
        if len(raw) == 0:
            break
        msg = raw.decode()
        # rcv_cnt = rcv_cnt + 1
        if msg == 'cali_xy_l' or msg == 'cali_z_l' or msg == 'cali_over_l':
            mutex2[0].acquire()
            global bias_flag_l
            if msg == 'cali_xy_l':
                bias_flag_l = 1
            if msg == 'cali_z_l':
                bias_flag_l = 2
            if msg == 'cali_over_l':
                bias_flag_l = 3
            mutex2[0].release()
        if msg == 'cali_xy_r' or msg == 'cali_z_r' or msg == 'cali_over_r':
            mutex2[1].acquire()
            global bias_flag_r
            if msg == 'cali_xy_r':
                bias_flag_r = 1
            if msg == 'cali_z_r':
                bias_flag_r = 2
            if msg == 'cali_over_r':
                bias_flag_r = 3
            mutex2[1].release()
        if msg == 'cali_over_l' or msg == 'cali_over_r':
            break
    client.close()
        
tudp0 = threading.Thread(target=t_udp, args=(8886, 0))
tudp0.setDaemon(True)
tudp0.start()

# tudp0 = threading.Thread(target=t_tcp_yuli, args=('192.168.1.201', 0))
# tudp0.setDaemon(True)
# tudp0.start()

tudp1 = threading.Thread(target=t_udp, args=(8885, 1))
tudp1.setDaemon(True)
tudp1.start()

tcali = threading.Thread(target=t_cali_server, args=())
tcali.setDaemon(True)
tcali.start()

trtde = threading.Thread(target=t_rtde, args=('192.168.1.102', 0))
trtde.setDaemon(True)
trtde.start()
t_rtde('192.168.1.101', 1)
