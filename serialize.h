unsigned char *serialize_int(unsigned char *buffer, int value);
unsigned char *serialize_short(unsigned char *buffer, short value);
unsigned char *serialize_chars(unsigned char *buffer, char *value);

int deserialize_int(unsigned char *buffer);
short deserialize_short(unsigned char *buffer);
unsigned char *deserialize_chars(unsigned char *buffer);
