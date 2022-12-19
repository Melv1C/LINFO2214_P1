import os
import matplotlib.pyplot as plt
import numpy as np


with open("data.txt","r") as f:
    data = f.readlines()

print(data)

# set width of bar
barWidth = 0.25
fig = plt.subplots(figsize =(12, 8))

# set height of bar
K_SIZE_8 = [int(data[0][:-1].split(",")[1]), int(data[2][:-1].split(",")[1])]
K_SIZE_128 = [int(data[1][:-1].split(",")[1]), int(data[3][:-1].split(",")[1])]

# Set position of bar on X axis
br1 = np.arange(len(K_SIZE_8))
br2 = [x + barWidth for x in br1]

plt.bar(br1, K_SIZE_8, color ='r', width = barWidth,edgecolor ='grey', label ='Key size 8')
plt.bar(br2, K_SIZE_128, color ='g', width = barWidth,edgecolor ='grey', label ='Key size 128')

#plt.ylim(1,3000)
#plt.yscale("log")


# Adding Xticks
plt.xlabel('Server', fontweight ='bold', fontsize = 15)
plt.ylabel('Nbre of request in 10sec', fontweight ='bold', fontsize = 15)
plt.xticks([r + barWidth//2 for r in range(len(K_SIZE_8))],
        ['server-float', 'server-float-avx'])

plt.title("Graph")

plt.legend(loc = "upper left")
plt.savefig('graph.png')
