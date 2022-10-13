import os
from random import random
from time import sleep
#os.system('./client -k 4 -r 1 -t 3 127.0.0.1:2241')

for i in range(100):
    "os.system('./client -k 4 -r {} -t {} 127.0.0.{}:2241&'.format(int(random()*1000),1+int(random()*5),i))"
    os.system('./client -k 2 -r {} -t {} 127.0.0.{}:2241&'.format(1000,1,i))
    sleep(0.5)


