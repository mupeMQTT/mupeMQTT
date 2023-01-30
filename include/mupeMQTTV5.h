#ifndef MUPEMQTTV5
#define MUPEMQTTV5


typedef struct
{
    uint32_t sessionExpiryInterval;
    char *authenticationMethod;
    char *authenticationData;
    uint8_t requestProblemInformation;
    uint8_t requestResponseInformation;
    uint16_t receiveMaximum;
    uint16_t topicAliasMaximum;
    uint16_t topicAlias;
    char *userPropertyA;
    char *userPropertyB;
    uint32_t maximumPacketSize;
    uint32_t willDelayInterval;
    uint32_t subscriptionIdentifier;
    int8_t payloadFormatIndicator;
    uint32_t messageExpiryInterval;
    char *contentType;
    char *responseTopic;
    char *correlationData;
    char *reasonString;
} PropertiesS;

int getProperties(int pos, uint8_t *buffer, PropertiesS properties);
#endif
