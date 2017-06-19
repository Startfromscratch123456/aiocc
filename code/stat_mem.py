
import sys
import os
import time


avg= [([0] * 6) for i in range(2)]
mvar=[([0] * 6) for i in range(2)]
while count < 120:
    cmd_free_rs=os.popen('free').readlines()
    count+=1
    #print cmd_free_rs[0]
    title=cmd_free_rs[0].split()
    tmp=cmd_free_rs[1].split()
    for i in range(1, len(tmp)):
        mvar[0][i-1]+=float(tmp[i])**2
        avg[0][i-1]=avg[0][i-1]+(float(tmp[i])-avg[0][i-1])/count
        print title[i-1]+" "+str(avg[0][i-1])+ " "
    tmp=cmd_free_rs[2].split()
    for i in range(1, len(tmp)):
        mvar[1][i-1]+=float(tmp[i])**2
        avg[1][i-1]=avg[1][i-1]+(float(tmp[i])-avg[1][i-1])/count
        print title[i-1]+" "+str(avg[1][i-1]) + " "
    time.sleep(1)

for i in range(0, len(avg[0])):
     mvar[0][i]=mvar[0][i]/count -avg[0][i]**2
     mvar[1][i]=mvar[1][i]/count -avg[1][i]**2


for i in range(0, len(avg[0])):
    print title[i]+" avg:"+str(avg[0][i]) +" var:" +str(mvar[0][i])

for i in range(0,len(avg[1])):
    print title[i]+" avg:"+str(avg[1][i])+" var:" +str(mvar[1][i])

