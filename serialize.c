#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct network {
  int count;
  char **ips;
  short *ports;
  int *heartbeat;
  int *local_time;
};
typedef struct network network;

unsigned char *serialize_int(unsigned char *buffer, int value) {
  /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char.
   */

  buffer[0] = value >> 24;
  buffer[1] = value >> 16;
  buffer[2] = value >> 8;
  buffer[3] = value;
  return buffer + 4;
}

unsigned char *serialize_short(unsigned char *buffer, short value) {
  buffer[0] = value >> 8;
  buffer[1] = value;
  return buffer + 2;
}

unsigned char *serialize_chars(unsigned char *buffer, char *value) {
  memcpy(buffer, value, 16);
  // strcpy((char *)buffer, value);
  return buffer + 16;
}

int deserialize_int(unsigned char *buffer) {
  return buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
}

short deserialize_short(unsigned char *buffer) {
  return buffer[0] << 8 | buffer[1];
}
unsigned char *deserialize_chars(unsigned char *buffer) { return buffer; }

// unsigned char *serialize_network(unsigned char *buffer, network *network) {
//   buffer = serialize_int(buffer, network->count);
//   for (int i = 0; i < network->count; i++) {
//     buffer = serialize_chars(buffer, network->ips[i]);
//     buffer = serialize_short(buffer, network->ports[i]);
//     buffer = serialize_int(buffer, network->heartbeat[i]);
//     buffer = serialize_int(buffer, network->local_time[i]);
//   }
//   return buffer;
// }
//
// network *deserialize_network(unsigned char *buffer) {
//
//   network *n = malloc(sizeof(network));
//   n->count = deserialize_int(buffer);
//   n->ips = NULL;
//   n->ports = NULL;
//   n->heartbeat = NULL;
//   n->local_time = NULL;
//
//   n->ips = realloc(n->ips, sizeof(char *) * n->count);
//   n->ports = realloc(n->ports, sizeof(short) * n->count);
//   n->heartbeat = realloc(n->heartbeat, sizeof(int) * n->count);
//   n->local_time = realloc(n->local_time, sizeof(int) * n->count);
//
//   for (int i = 0; i < n->count; i++) {
//     n->ips[i] = malloc(16);
//     strcpy(n->ips[i], (char *)deserialize_chars(buffer + 4 + 26 * i));
//     // n->ips[i] = (char *)deserialize_chars(buffer + 4 + 30 * i);
//     n->ports[i] = deserialize_short(buffer + 20 + 26 * i);
//     n->heartbeat[i] = deserialize_int(buffer + 22 + 26 * i);
//     n->local_time[i] = deserialize_int(buffer + 26 + 26 * i);
//   }
//   return n;
// }
//
// int main(int argc, char **argv) {
//
//   network *n = malloc(sizeof(network));
//   n->count = 2;
//   n->ips = NULL;
//   n->ports = NULL;
//   n->heartbeat = NULL;
//   n->local_time = NULL;
//
//   n->ips = realloc(n->ips, sizeof(char *) * 2);
//   n->ips[0] = malloc(16);
//   strcpy(n->ips[0], "127.123.221.224");
//   n->ips[1] = malloc(16);
//   strcpy(n->ips[1], "256.333.111.777");
//
//   n->ports = realloc(n->ports, sizeof(short) * 2);
//   n->ports[0] = 8080;
//   n->ports[1] = 9090;
//
//   n->heartbeat = realloc(n->heartbeat, sizeof(int) * 2);
//   n->heartbeat[0] = 2340;
//   n->heartbeat[1] = 6732;
//
//   n->local_time = realloc(n->local_time, sizeof(int) * 2);
//   n->local_time[0] = 12345;
//   n->local_time[1] = 6820;
//
//   unsigned char *buffer =
//       malloc(sizeof(int) + sizeof(int) * n->count * 2 +
//              sizeof(char *) * n->count + sizeof(short) * n->count);
//
//   serialize_network(buffer, n);
//   network *res = deserialize_network(buffer);
//   printf("res->count %i\n", res->count);
//   printf("res->ips[0] %s\n", res->ips[0]);
//   printf("res->ips[1] %s\n", res->ips[1]);
//   printf("res->ports[0] %i\n", res->ports[0]);
//   printf("res->ports[1] %i\n", res->ports[1]);
//   printf("res->heartbeat[0] %i\n", res->heartbeat[0]);
//   printf("res->heartbeat[1] %i\n", res->heartbeat[1]);
//   printf("res->local_time[0] %i\n", res->local_time[0]);
//   printf("res->local_time[1] %i\n", res->local_time[1]);
//
//   return 0;
// }
