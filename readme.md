# Gossip style membership

A distributed system based on a gossip style membership. Each node chooses 2 nodes at random from their membership list and gossips their membership list. The entire network receives the membership after Olog(N) gossips.

This repo was mostly for me to practice C and to play around with sockets programming.

compile with:

`cc -Wall -std=c99 node.c serialize.c -o bin/node`

run the first node with

`bin/node $PORTNO`

run subsequent nodes with

`bin/node $PORTNO 127.0.0.1 $PORTNO_OF_EXISTING`

Memberships will be printed to the terminal. If you kill a process, after a short while membership lists will remove the failed process.

TODO:
- use hash maps
- make it work for distributed network instead of just locally
- handle membership lists when the list gets too large for the max udp size
