port=2254
rate=100
key_size=8




rm respond_time.txt
rm service_time.txt
rm arrival_time.txt
rm client_in_queue.txt



kill -9 $(lsof -t -i:${port})

sleep 2

echo "Lancement de server queue"
./server-queue -j 1 -s 1024 -p ${port}&
sleep 1

echo "Lancement du client queue"
./client-queue -k 128 -r ${rate} -t 5 127.0.0.1:${port}

sleep 2

kill -9 $(lsof -t -i:${port})


python3 ./tests/graph.py