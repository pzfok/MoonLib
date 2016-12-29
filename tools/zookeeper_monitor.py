#!/usr/bin/env python
#coding=utf-8

#这个脚本用来监控本机ZooKeeper是否正常运行。
#用于本机时，可以监控以下场景：
#   服务是否正常运行
#   服务反复拉起监控
#

import sys
import time
import commands
import traceback
import os

LAST_FILE = "/tmp/last_vmstat"
CURR_FILE = "/proc/vmstat"
CURR_FILE_BAK = "/tmp/vmstat_tmp"

#反复重启检测，在REBOOT_CHECK_TIME秒内，达到REBOOT_CHECK_COUNT次重启，则告警
REBOOT_CHECK_TIME = 600    
REBOOT_CHECK_COUNT = 3      #重启时间范围检测
CURR_TIME = int(time.time())

REBOOT_TIMES_KEY = "reboot_times"
CURR_TIME_KEY = "curr_time"
PID_KEY = "pid"

#发送公安经
def SendMsg(msg):
    print >> sys.stderr,"告警:" + str(msg)

#获得上一次的数据，解析内容：
#zk_version 3.4.8-1--1, built on Fri, 26 Feb 2016 14:51:43 +0100
#zk_avg_latency 0
#zk_max_latency 38
#zk_min_latency 0
#zk_packets_received 48846
#zk_packets_sent 48846
#zk_num_alive_connections 16
#zk_outstanding_requests 0
#zk_server_state standalone
#zk_znode_count 142
#zk_watch_count 12
#zk_ephemerals_count 13
#zk_approximate_data_size 3121
#zk_open_file_descriptor_count 59
#zk_max_file_descriptor_count 65536
#pid 12345
def GetDatas(data_str):
    datas = {}

    split_result = data_str.split("\n")
    for line in split_result:
        kv = line.split("\t")
        if len(kv) != 2:
            continue
        datas[kv[0]] = kv[1]

    #尝试获得死机时间，并且按照空格分割
    if not datas.has_key(REBOOT_TIMES_KEY):
        datas[REBOOT_TIMES_KEY] = ""
    datas[REBOOT_TIMES_KEY] = filter(None, datas[REBOOT_TIMES_KEY].split())

    #将当前的时间戳放进去
    datas[CURR_TIME_KEY] = CURR_TIME

    if not datas.has_key(PID_KEY):
        datas[PID_KEY] = "0"

    return datas

#写回文件
def SetDatas(datas,file_path):
    #将重启时间恢复成字符串
    datas[REBOOT_TIMES_KEY] = " ".join(map(str,datas[REBOOT_TIMES_KEY]))

    #逐行写入文件
    file = open(file_path,'w')
    for k in datas:
        file.write(k + "\t" + str(datas[k]) + "\n")
    file.close()

#检查
def Check(last_datas,curr_datas):
    #判断是否重启过
    if last_datas[PID_KEY] != curr_datas[PID_KEY]:
        #记录重启的时间
        curr_datas[REBOOT_TIMES_KEY].append(CURR_TIME)
        
    #过滤重启时间，距离当前时间超过REBOOT_CHECK_TIME，则删除记录
    delete_list = []
    for time in curr_datas[REBOOT_TIMES_KEY]:
        if CURR_TIME - int(time) > REBOOT_CHECK_TIME or CURR_TIME < int(time):
            delete_list.append(time)

    for time in delete_list:
        curr_datas[REBOOT_TIMES_KEY].remove(time)

    #判断是否反复重启
    if len(curr_datas[REBOOT_TIMES_KEY]) >= REBOOT_CHECK_COUNT:
        raise Exception("ZooKeeper服务[%s:%d]%d秒内重启%d次>=%d次" % (host,port,REBOOT_CHECK_TIME,
                                                               len(curr_datas[REBOOT_TIMES_KEY]),REBOOT_CHECK_COUNT))

##主函数操作
try:
    if(len(sys.argv) < 4):
        print >> sys.stderr,("Usage:")
        print >> sys.stderr,("python " + sys.argv[0] + " zookeeper_host_ip zookeeper_port pid_file")
        sys.exit(0)

    host = sys.argv[1]
    port = int(sys.argv[2])
    pid_file_path = sys.argv[3]
    last_stat_file = "/tmp/zk_monitor_%s_%d.txt" % (host,port)
    
    #检查服务是否正常
    ret,curr_data_str = commands.getstatusoutput("echo mntr|nc %s %d" % (host,port))
    if ret != 0:
        raise Exception("ZooKeeper服务[%s:%d]无响应:%s" % (host,port,curr_data_str))

    if curr_data_str.find("not currently") != -1:
        raise Exception("ZooKeeper服务[%s:%d]服务异常:%s" % (host,port,curr_data_str))

    #检查当前PID文件
    if not os.path.isfile(pid_file_path):
        raise Exception("ZooKeeper服务[%s:%d]PID文件无效" % (host,port))

    pid_file = open(pid_file_path)
    pid = int(pid_file.readline())
    pid_file.close()
    if pid == 0:
        raise Exception("ZooKeeper服务[%s:%d]PID为0" % (host,port))

    ret,zk_ps_count = commands.getstatusoutput("ps aux|grep %d|grep zookeeper|grep -v grep|wc -l" % pid)
    if ret != 0:
        raise Exception("ZooKeeper服务[%s:%d]进程检测失败,ret[%s]:%s" % (host,port,ret,zk_ps_count))

    if int(zk_ps_count) != 1:
        raise Exception("ZooKeeper服务[%s:%d]进程已找不到,进程数%s" % (host,port,zk_ps_count))

    #获得上次的结果，获取不到的话，后面处理返回0，不影响
    last_stat = ""
    if os.path.isfile(last_stat_file):
        last_file = open(last_stat_file)
        last_stat = last_file.read()
        last_file.close()
    last_datas = GetDatas(last_stat)

    #获得当前的数据，到这里，服务正常了
    curr_datas = GetDatas(curr_data_str)

    #把pid放进去
    curr_datas[PID_KEY] = str(pid)
    curr_datas[REBOOT_TIMES_KEY] = last_datas[REBOOT_TIMES_KEY]

    #检查
    Check(last_datas,curr_datas)

    #写回文件
    SetDatas(curr_datas,last_stat_file)
    sys.exit(0)

except Exception, arg:
    SendMsg(arg)
    print traceback.format_exc()

    #写回文件
    if curr_datas != None:
        SetDatas(curr_datas,last_stat_file)

    sys.exit(-1)
