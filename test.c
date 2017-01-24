#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
  char *ip;
  short port;
  int heartbeat;
  int local_time;
};

typedef struct node node;

int main(int argc, char **argv) {

  node *n = malloc(sizeof(node));
  n->ip = malloc(16);
  memset(n->ip, '\0', 16);

  strcpy(n->ip, "127.123.456.789\0");
  n->port = 22;
  n->heartbeat = 12;
  printf("before local time is set: node->ip is %s\n", n->ip);
  n->local_time = 11;
  printf("after local_time is set: node->ip is %s\n", n->ip);
  return 0;
}
