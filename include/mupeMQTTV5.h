
// Copyright Peter Müller mupe
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

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
