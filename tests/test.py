import os
from random import random
from time import sleep
#os.system('./client -k 4 -r 1 -t 3 127.0.0.1:2241')

for i in range(10):
    os.system('./client -k 4 -r {} -t {} 127.0.0.{}:2241&'.format(int(random()*1000),1+int(random()*5),i))
    sleep(random()*2)


