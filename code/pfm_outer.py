import sys
import os 
import numpy
import atexit
import time 
import matplotlib as mpl
mpl.use('Agg') 
import numpy as np
import matplotlib.pyplot as plt
n=300
factor=1024**2
values={"memory-unused":[],"memory-available":[],"memory-rss":[]}
files_name=os.listdir(".")
for file_name in files_name:
    if "memory-unused" in file_name or "memory-available" in file_name or "memory-total" in file_name or "memory-rss" in file_name:
        rs=os.popen("cat "+str(file_name)+" | tail -"+str(n)).readlines()
        value=[]
        for line in rs:
            value.append( float(line.split(",")[1])/factor )
        narray=numpy.array(value)                        
        sum1=narray.sum()
        narray2=narray*narray
        sum2=narray2.sum()
        mean=sum1/len(value)
        var=sum2/len(value)-mean**2
        if "memory-unused" in file_name :
            plt.figure(1,figsize=(15,6))
            x=np.linspace(0,n,n)
            ax = plt.subplot(1,1,1)
            data_outer = ax.plot(x, value,'-',linewidth=1,label="memory-unused-collectd")
            ax.legend(loc="upper right")
            ax.set_title("memory-unused")
            ax.set_xlabel("time")
            ax.set_ylabel("memory-unused(MB)")
            plt.grid(True)
            plt.savefig("memory-unused.png")
            plt.close()
    
            
            
            #values["memory-unused"].append( float(line.split(",")[1])/factor ) 
            print "memory-unused"
        elif "memory-available" in file_name:
            #values["memory-available"].append( float(line.split(",")[1])/factor ) 
            print "memory-available"
        elif "memory-total" in file_name:
            print "memory-total"
        elif "memory-rss" in file_name:
            #values["memory-rss"].append( float(line.split(",")[1])/factor ) 
            print "memory-rss"
        print str(value)
        print "mean:"+str(mean)+" var:"+str(var)
     
                
            
