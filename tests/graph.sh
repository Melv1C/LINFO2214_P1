rm data.txt
kill -9 $(lsof -t -i:2100)
kill -9 $(lsof -t -i:2101)

sleep 2

echo "Lancement de server float"
./server-float -j 1 -s 1024 -p 2100&

echo "Lancement du client (size key 8)"
./client-graph -k 8 127.0.0.1:2100

echo "Lancement du client (size key 128)"
./client-graph -k 128 127.0.0.1:2100

echo "Lancement de server float avx"
./server-float-avx -j 1 -s 1024 -p 2101&


echo "Lancement du client (size key 8)"
./client-graph -k 8 127.0.0.1:2101

echo "Lancement du client (size key 128)"
./client-graph -k 128 127.0.0.1:2101

python3 ./tests/graph.py