#!/bin/bash

./server -j 4 -s 16 -p 2241&

sleep 2

# On démarre les clients

for (( i = 1; i < 101; i++ )); do
  echo "LANCEMENT DU CLIENT $i"
  if ! (./client -k 4 -r 1 -t 3 127.0.0.$i:2241&) ; then
    echo "Crash du client!"
  fi
done

sleep 10 # On attend 5 seconde que le receiver finisse

