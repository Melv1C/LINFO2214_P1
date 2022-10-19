
#client
import os
from time import sleep

#--------------------------------------------------------------------
# Run 1.


for i in [50,100,250,500,750,1000]:
    os.system("./client -k 16 -r {} -t 3 127.0.0.1:2241".format(i))
    sleep(5)

