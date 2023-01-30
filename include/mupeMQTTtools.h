#ifndef MUPEMQQTTOOLS
#define MUPEMQQTTOOLS
#include <string.h>

const char *byte_to_binary(uint8_t x);


char *getString(uint8_t *payload);

uint32_t getRemainingLength( uint8_t *rx_buffer) ;
uint8_t getSizeOfRemainingLength(uint8_t *rx_buffer);
uint16_t getTopicLenght(uint8_t *buffer);
uint8_t isTopic(char *topic, char *filter);
void getTopicName(uint8_t *buffer,char *topicName ,uint16_t lenght);


#endif
