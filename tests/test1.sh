#!/bin/bash

kill -9 2241

./server -j 4 -s 32 -p 2241 &

sleep 1

cleanup()
{
   kill -9 $receiver_pid
   kill -9 $link_pid
   exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre les clients

for i in {1..10} ; do
    if ! (./client -k 4 -r 10 -t 2 127.0.0.1:2241 &) ; then
      echo "Crash du client!"
    fi
done

sleep 10 # On attend 5 seconde que le receiver finisse

