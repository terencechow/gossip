#include "serialize.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define TFAIL 600;

void error(const char *msg) {
  perror(msg);
  exit(1);
}

struct node {
  char *ip;
  short port;
  int heartbeat;
  int local_time;
};

typedef struct node node;

node *create_node(char *ip, short p, int h, int l) {
  node *n = malloc(sizeof(node));
  n->ip = malloc(INET_ADDRSTRLEN);
  memset(n->ip, '\0', INET_ADDRSTRLEN);
  strcpy(n->ip, ip);
  n->port = p;
  n->heartbeat = h;
  n->local_time = l;
  return n;
}

void read_node(node *n) {
  printf("%s:%i | %i | %i\n", n->ip, n->port, n->heartbeat, n->local_time);
}

struct network {
  int count;
  node **nodes;
};

typedef struct network network;

network *create_network(char *ip, short port) {
  network *net = malloc(sizeof(network));
  net->count = 1;
  net->nodes = malloc(sizeof(node *) * net->count);
  node *n = create_node(ip, port, 0, 20);
  net->nodes[0] = n;
  return net;
}

void read_network(network *net) {
  printf("========================\n");
  printf("count is %i\n", net->count);
  printf("ip:port | heartbeat | local_time\n");
  for (int i = 0; i < net->count; i++) {
    read_node(net->nodes[i]);
  }
  printf("========================\n");
}

void increment_heartbeat_and_time(network *net, int h, int l) {
  net->nodes[0]->heartbeat = h;
  net->nodes[0]->local_time = l;
}

void update_network(network *current, network *new, int local_time) {
  // this is n^2 but i'm too lazy to implement a hash table right now
  int current_count = current->count;

  for (int i = 0; i < new->count; i++) {
    for (int j = 0; j < current_count; j++) {
      // if ip & port exists in our current table...
      if (strcmp(new->nodes[i]->ip, current->nodes[j]->ip) == 0 &&
          new->nodes[i]->port == current->nodes[j]->port) {
        if (new->nodes[i]->heartbeat > current->nodes[j]->heartbeat) {
          // if the heartbeat is higher update the network with the current
          // heartbeat
          current->nodes[j]->heartbeat = new->nodes[i]->heartbeat;
          current->nodes[j]->local_time = local_time;
        }

        break;
      } else if (j == current_count - 1) {
        // if we are on the last entry and the ip/ports don't match
        // then the new entry is not in our network and we should add it

        current->count++;

        // int *ptr = realloc(&current->nodes, sizeof(node *) * current->count);
        current->nodes =
            realloc(current->nodes, sizeof(node *) * current->count);
        // if (ptr) {
        // current->nodes = ptr;
        node *new_node = create_node(new->nodes[i]->ip, new->nodes[i]->port,
                                     new->nodes[i]->heartbeat, local_time);
        current->nodes[current->count - 1] = new_node;
        //   free(ptr);
        // } else {
        //   free(current->nodes);
        // }
      }
    }
  }
}

unsigned char *serialize_node(unsigned char *buffer, node *n) {
  buffer = serialize_chars(buffer, n->ip);
  buffer = serialize_short(buffer, n->port);
  buffer = serialize_int(buffer, n->heartbeat);
  buffer = serialize_int(buffer, n->local_time);
  return buffer;
}

unsigned char *serialize_network(unsigned char *buffer, network *net) {
  buffer = serialize_int(buffer, net->count);
  for (int i = 0; i < net->count; i++) {
    buffer = serialize_node(buffer, net->nodes[i]);
  }
  return buffer;
}

network *deserialize_network(unsigned char *buffer) {

  network *net = malloc(sizeof(network));
  net->count = deserialize_int(buffer);
  net->nodes = malloc(sizeof(node *) * net->count);
  char *ip = malloc(INET_ADDRSTRLEN);
  memset(ip, '\0', INET_ADDRSTRLEN);
  for (int i = 0; i < net->count; i++) {

    ip = (char *)(buffer + 4 + 26 * i);
    node *n = create_node(ip, deserialize_short(buffer + 20 + 26 * i),
                          deserialize_int(buffer + 22 + 26 * i),
                          deserialize_int(buffer + 26 + 26 * i));
    net->nodes[i] = n;
  }
  return net;
}

void free_network(network *net) {
  for (int i = 0; i < net->count; i++) {
    free(net->nodes[i]->ip);
  }
  free(net->nodes);
  free(net);
}

void gossip(int sockfd, network *net) {
  struct sockaddr_in send_addr;
  send_addr.sin_family = AF_INET;
  if (net->count != 1) {
    int total_sent = 0;
    int sent_to = 0;
    while (total_sent < 2 && total_sent < net->count - 1) {
      int r = 1 + (rand() % (net->count - 1));
      if (r != sent_to) {
        send_addr.sin_port = htons(net->nodes[r]->port);
        int s = inet_pton(AF_INET, net->nodes[r]->ip, &(send_addr.sin_addr));
        if (s < 0)
          perror("Error with inet_pton");
        unsigned char *serialized =
            malloc(sizeof(int) + sizeof(int) * net->count * 2 +
                   sizeof(char) * INET_ADDRSTRLEN * net->count +
                   sizeof(short) * net->count);

        unsigned char *ptr = serialize_network(serialized, net);

        int res = sendto(sockfd, serialized, ptr - serialized, MSG_DONTWAIT,
                         (struct sockaddr *)&send_addr, sizeof(send_addr));

        if (res < 0)
          error("Error gossiping");

        sent_to = r;
        total_sent++;
      }
    }
  }
}

int main(int argc, char **argv) {

  srand(time(NULL));
  // if no port provided, exit
  if (argc < 2) {
    fprintf(stderr, "Error no port provided\n");
    exit(0);
  }

  // declarations
  // socket file descriptors & a port #
  int sockfd, portno;

  // addresses of server and client
  struct sockaddr_in serv_addr, cli_addr;

  // length of the client address
  socklen_t clilen;

  // open a socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    error("Error opening socket");

  // set the serv_addr buffer to 0
  memset(&serv_addr, 0, sizeof(serv_addr));
  // bzero(&serv_addr, sizeof(serv_addr));

  // define the serv_addr
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;

  // INADDR_ANY is the IP address of the machine this server is running on
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  // bind the socket to the serv_addr (hostname and port) provided above
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("Error on binding");

  clilen = sizeof(cli_addr);

  // instantiate a network
  char *ip = malloc(INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &(serv_addr.sin_addr.s_addr), ip, INET_ADDRSTRLEN);
  network *received;

  network *net = create_network(ip, portno);

  if (argc == 4) {

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(atoi(argv[3]));
    int s = inet_pton(AF_INET, argv[2], &(cli_addr.sin_addr));
    if (s < 0)
      perror("Error with inet_pton");

    unsigned char *serialized =
        malloc(sizeof(int) + sizeof(int) * net->count * 2 +
               sizeof(char) * INET_ADDRSTRLEN * net->count +
               sizeof(short) * net->count);

    printf("Joining to: %s:%s", argv[2], argv[3]);

    unsigned char *ptr = serialize_network(serialized, net);
    int n = sendto(sockfd, serialized, ptr - serialized, MSG_DONTWAIT,
                   (struct sockaddr *)&cli_addr, clilen);

    if (n < 0)
      error("Error sending initial message");
  }

  int local_time = 0;
  int heartbeat = 0;
  int i = 0;
  while (1) {
    i++;
    if (i % 100000 == 0) {
      heartbeat++;
      local_time++;
      i = 0;
      increment_heartbeat_and_time(net, heartbeat, local_time);
      gossip(sockfd, net);
    }

    int n;
    char count_buffer[sizeof(int)];
    memset(count_buffer, 0, sizeof(int));

    // peek the first 32 bytes so we can get the size of the network
    n = recvfrom(sockfd, count_buffer, sizeof(int), MSG_DONTWAIT | MSG_PEEK,
                 (struct sockaddr *)&cli_addr, &clilen);
    if (n < 0) {
      // printf("heartbeat %i\n", heartbeat);
    } else {

      int count = deserialize_int((unsigned char *)count_buffer);

      int buffer_size =
          (sizeof(int) + sizeof(int) * count * 2 +
           sizeof(char) * INET_ADDRSTRLEN * count + sizeof(short) * count);
      unsigned char *buffer = malloc(buffer_size);
      memset(buffer, 0, buffer_size);

      n = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&cli_addr,
                   &clilen);
      if (n < 0)
        error("Error receiving message");

      // int prior_count = network->count;
      received = deserialize_network(buffer);

      update_network(net, received, local_time);

      printf("Received UDP, Updated Network:\n");
      read_network(net);
    }
  }
  // free the received network
  free_network(received);
  return 0;
}
