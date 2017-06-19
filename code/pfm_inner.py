import sys
import os 
import numpy
import atexit
import time 
import psutil
import matplotlib as mpl
mpl.use('Agg') 
import numpy as np
import matplotlib.pyplot as plt
'''

'''
n=5
factor=1024**2

class MemoryStat:
    def __init__(self):
        self.total=0
        self.available=[]
        self.used=[]
        self.free=[]
        self.buffers=[]
        self.cached=[]
        self.shared=[]
        self.color={"available":"cyan","used":"green","free":"red","buffers":"blue","cached":"yellow" ,"shared":"black"  }

    def add(self, virtual_memory):
        self.total=virtual_memory.total/factor
        self.available.append(virtual_memory.available/factor)
        self.used.append(virtual_memory.used/factor)
        self.free.append(virtual_memory.free/factor)
        self.buffers.append(virtual_memory.buffers/factor)
        self.cached.append(virtual_memory.cached/factor)
        self.shared.append(virtual_memory.shared/factor)
    def draw0(self,name,value0,color_name):
        plt.figure(1,figsize=(15,6))
        #value=[] 
        #for i in range(0,len(value0)-1,2):
        #    value.append((value0[i]+value0[i+1])/2)
        x=np.linspace(0,n,n)
        ax = plt.subplot(1,1,1)
        ax.plot(x, value0,'-',linewidth=1,label=name,color=color_name)
        ax.legend(loc="upper right")
        ax.set_title(name)
        ax.set_xlabel("time")
        ax.set_ylabel(name+"(MB)")
        plt.grid(True)
        plt.savefig(name+".png")
        plt.close()
    def draw(self):
        for name,value in vars(self).items():
            if name != "total" and name != "color" and len(value) == n:
                self.draw0(name, value, self.color[name]);
    def showStatistics(self):
        print "total:"+str(self.total)
        for name,value in vars(self).items():
            if name != "total" and name != "color":
                narray=numpy.array(value)                        
                sum1=narray.sum()
                narray2=narray*narray
                sum2=narray2.sum()
                mean=sum1/len(value)
                var=sum2/len(value)-mean**2
                print name+":"+str(value)
                print "mean:"+str(mean)+" var:"+str(var)
    
 

sleepTime=1
count=n
mstat = MemoryStat()
while count>0:
    mstat.add(psutil.virtual_memory())
    count-=1
    time.sleep(sleepTime)
mstat.showStatistics()       
mstat.draw() 

