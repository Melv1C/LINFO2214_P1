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


#----------------------Graph 3 & 4


for line in fserv.readlines():
    z = line.strip().split(",")
    if int(z[0]) == 4:
        y2[2].append(int(z[2]))
    else:
        y2[int(z[0]) - 1].append(int(z[2]))
j=0
i = 0
for line in fclient.readlines():
    z = line.strip().split(",")
    y[i//6].append(int(z[5]))
    i+=1
    if j < 6:
        x.append(z[0])
        j+=1

    

#Plot Graph 1

print(x)
print(y)
print(y2)

plt.plot(x,y[0],label="1 Thread")
plt.plot(x,y[1],label="2 Threads")
plt.plot(x,y[2],label="4 Threads")

plt.xlabel("Frequency of requests for 3secs")
plt.ylabel("Average Response Time")
plt.title("Average Response Time according to frequency of requests")


plt.legend(loc = "upper left")
plt.show()


plt2.plot(x,y2[0],label="1 Thread")
plt2.plot(x,y2[1],label="2 Threads")
plt2.plot(x,y2[2],label="4 Threads")

plt2.xlabel("Frequency of requests")
plt2.ylabel("Max buffer size")
plt2.title("Maximum buffer size according to frequency of requests")


plt2.legend(loc = "upper left")
plt2.show()
