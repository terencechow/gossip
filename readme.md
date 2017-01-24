Gossip style membership

compile with:

`cc -Wall -std=c99 node.c serialize.c -o bin/node`

run the first node with

`bin/node $PORTNO`

run subsequent nodes with

`bin/node $PORTNO 127.0.0.1 $PORTNO_OF_EXISTING`


TODO:
add in Tfail and Tcleanup to handle removing nodes that fail.
