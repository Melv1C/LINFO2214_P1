from http import client
import matplotlib.pyplot as plt
import matplotlib.pyplot as plt2
import numpy as np

fserv  = open("vm_stat_server.txt", "r")
fclient = open("vm_stat_client.txt", "r")
fserv.readline()
fclient.readline()
x = []
y = []
y2 = []
for i in range (3):
    y.append([])
    y2.append([])


#----------------------Graph 1 & 2

i = 0
for line in fserv.readlines():
    z = line.strip().split(",")
    if int(z[0]) == 4:
        y2[2].append(int(z[2]))
    else:
        y2[int(z[0]) - 1].append(int(z[2]))
    if i < 5:
        x.append(z[1])
        i+=1

i = 0
for line in fclient.readlines():
    z = line.strip().split(",")
    y[i//5].append(int(z[5]))
    i+=1

    

#Plot Graph 1


X_axis = np.arange(len(x))
print(x)
print(y)
print(y2)

plt.bar(X_axis-0.3,y[0],0.3,label="1Thread",edgecolor = "black")
plt.bar(X_axis,y[1],0.3,label="2Threads",edgecolor = "black")
plt.bar(X_axis+0.3,y[2],0.3,label="4Threads",edgecolor = "black")

plt.xticks(X_axis, x)
#freq == 500/s pour 3s
plt.xlabel("Size Of Files (Size of key = 16)")
plt.ylabel("Average Response Time")
plt.title("Average Response Time according to Size of Files")

plt.legend(loc = "upper left")
plt.show()

#avgResp by freq of requests

plt2.bar(X_axis-0.3,y2[0],0.3,label="1Thread",edgecolor = "black")
plt2.bar(X_axis,y2[1],0.3,label="2Threads",edgecolor = "black")
plt2.bar(X_axis+0.3,y2[2],0.3,label="4Threads",edgecolor = "black")

plt2.xticks(X_axis, x)
#freq == 500/s pour 3s
plt2.xlabel("Size Of Files (Size of key = 16)")
plt2.ylabel("Maximum BufferSize")
plt2.title("Maximum buffersize according to size of file")

plt2.legend(loc = "upper left")
plt2.show()

"""
plt2.plot(X_axis,y[0],label="1 Thread")
plt2.plot(x,y[1],label="2 Threads")
plt2.plot(x,y[2],label="4 Threads")

plt2.xlabel("Frequency of requests")
plt2.ylabel("Average Response Time")
plt2.title("Average Response Time according to frequency of requests")


plt2.legend(loc = "upper left")
plt2.show()

"""