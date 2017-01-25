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
