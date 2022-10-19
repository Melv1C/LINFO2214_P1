import os
from random import random
from time import sleep

n_thread = 1
size = 128
#for n_thread in range(1,5):
    #for size in [4,8,16,32,64,126,256,512,1024]:

os.system('./server -j {} -s {} -p 2241&'.format(n_thread,size))
sleep(10)

for key_size in [16,32,64,128]:
    if key_size>size:
        break
    else:
        for rate in [1,10,100,200,300,400,500,1000]:
            for time in [1,3,5]:
                os.system('./client -k {} -r {} -t {} 127.0.0.1:2241'.format(key_size,rate,time))
                sleep(5)

os.system("kill `pidof server`")



