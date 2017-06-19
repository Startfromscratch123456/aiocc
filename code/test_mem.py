
import sys
import os

count=0

data= [([0] * 6) for i in range(2)]
while count < 10:
    cmd_free_rs=os.popen('free').readlines()
    count+=1
    #print cmd_free_rs[0]
    title=cmd_free_rs[0].split()
    tmp=cmd_free_rs[1].split()
    for i in range(1, len(tmp)):
        data[0][i-1]=data[0][i-1]+float(float(tmp[i])-data[0][i-1])/count
        print title[i-1]+" "+str(data[0][i-1])+ " "
    tmp=cmd_free_rs[2].split()
    for i in range(1, len(tmp)):
        data[1][i-1]=data[1][i-1]+float(float(tmp[i])-data[1][i-1])/count
        print title[i-1]+" "+str(data[1][i-1]) + " "
    #time.sleep(60)

for i in range(0, len(data[0])):
    print str(data[0][i])+" "

for i in range(0,len(data[1])):
    print str(data[1][i])+" "

