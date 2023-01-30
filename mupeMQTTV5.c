
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

#include <string.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "esp_log.h"
#include "mupeMQTTV5.h"
#include "mupeMQTTtools.h"


static const char *TAG = "mupeMQTTV5";

int getProperties(int pos, uint8_t *buffer, PropertiesS properties)
{
    ESP_LOGD(TAG, "CONNECT Properties %i %i", buffer[pos], buffer[pos + 1]);
    uint8_t pLenghtSize = getSizeOfRemainingLength(&buffer[pos - 1]);
    ESP_LOGD(TAG, "getSizeOfRemainingLength Properties %i", pLenghtSize);
    uint32_t pLenght = getRemainingLength(&buffer[pos - 1]);
    ESP_LOGD(TAG, "getRemainingLength Properties %lu", pLenght);
    pos = pos + pLenghtSize;
    uint32_t posx = pos + pLenght;
    do
    {
        ESP_LOGD(TAG, "property Nr.  %i", buffer[pos]);
        switch (buffer[pos])
        {
        case 1:
            properties.payloadFormatIndicator = buffer[pos + 1];
            ESP_LOGD(TAG, "payloadFormatIndicator Properties %i", properties.payloadFormatIndicator);
            pos = pos + 2;

            break;
        case 2:
            properties.messageExpiryInterval = 256 * (256 * (256 * buffer[pos + 1] + buffer[pos + 2]) + buffer[pos + 3]) + buffer[pos + 4];
            pos = pos + 5;
            ESP_LOGD(TAG, "messageExpiryInterval Properties %lu", properties.messageExpiryInterval);
            break;
        case 3:
            properties.contentType = malloc(buffer[pos + 1] * 256 + buffer[pos + 2] + 1);
            memcpy(properties.contentType, &buffer[pos + 3], buffer[pos + 1] * 256 + buffer[pos + 2]);
            properties.contentType[buffer[pos + 1] * 256 + buffer[pos + 2]] = '\0';
            pos = pos + buffer[pos + 1] * 256 + buffer[pos + 2] + 2;
            ESP_LOGD(TAG, "contentType Properties %s", properties.contentType);
            free(properties.contentType);
            pos++;
            break;
        case 8:
            properties.responseTopic = malloc(buffer[pos + 1] * 256 + buffer[pos + 2] + 1);
            memcpy(properties.responseTopic, &buffer[pos + 3], buffer[pos + 1] * 256 + buffer[pos + 2]);
            properties.responseTopic[buffer[pos + 1] * 256 + buffer[pos + 2]] = '\0';
            pos = pos + buffer[pos + 1] * 256 + buffer[pos + 2] + 1;
            ESP_LOGD(TAG, "responseTopic Properties %s", properties.responseTopic);
            free(properties.responseTopic);
            break;
        case 9:
            properties.correlationData = malloc(posx - pos + 1);
            memcpy(properties.correlationData, &buffer[pos], posx - pos);
            properties.correlationData[posx - pos] = '\0';
            pos = posx;
            ESP_LOGD(TAG, "correlationData Properties %s", properties.correlationData);
            free(properties.correlationData);
            break;
        case 11:
            properties.subscriptionIdentifier = 0;
            uint32_t multiplier = 1;
            do
            {
            	pos++;
                properties.subscriptionIdentifier += (buffer[pos] & 127) * multiplier;
                multiplier = multiplier * 128;
                if (multiplier > (128 * 128 * 128))
                {
                    ESP_LOGE(TAG, "multiplier >128*128*128  %lu", multiplier);
                    return 0;
                }
            } while ((buffer[pos] & 128) != 0);
            ESP_LOGD(TAG, "subscriptionIdentifier Properties %lu", properties.subscriptionIdentifier);
            pos++;
            break;
        case 17:
            properties.sessionExpiryInterval = 256 * (256 * (256 * buffer[pos + 1] + buffer[pos + 2]) + buffer[pos + 3]) + buffer[pos + 4];
            ESP_LOGD(TAG, "sessionExpiryInterval Properties %lu", properties.sessionExpiryInterval);
            pos = pos + 5;
            break;
        case 21:
            properties.authenticationMethod = malloc(buffer[pos + 1] * 256 + buffer[pos + 2] + 1);
            memcpy(properties.authenticationMethod, &buffer[pos + 3], buffer[pos + 1] * 256 + buffer[pos + 2]);
            properties.authenticationMethod[buffer[pos + 1] * 256 + buffer[pos + 2]] = '\0';
            pos = pos + buffer[pos + 1] * 256 + buffer[pos + 2] + 1;
            ESP_LOGD(TAG, "authenticationMethod Properties %s", properties.authenticationMethod);
            free(properties.authenticationMethod);
            break;
        case 22:
            properties.authenticationData = malloc(posx - pos + 1);
            memcpy(properties.authenticationData, &buffer[pos], posx - pos);
            properties.authenticationData[posx - pos] = '\0';
            pos = posx;
            ESP_LOGD(TAG, "authenticationData Properties %s", properties.authenticationData);
            free(properties.authenticationData);
            break;
        case 23:
            properties.requestProblemInformation = buffer[pos + 1];
            ESP_LOGD(TAG, "requestProblemInformation Properties %i", properties.requestProblemInformation);
            pos = pos + 2;
            break;
        case 24:
            properties.willDelayInterval = 256 * (256 * (256 * buffer[pos + 1] + buffer[pos + 2]) + buffer[pos + 3]) + buffer[pos + 4];
            pos = pos + 5;
            ESP_LOGD(TAG, "willDelayInterval Properties %lu", properties.willDelayInterval);
            break;
        case 25:
            properties.requestResponseInformation = buffer[pos + 1];
            ESP_LOGD(TAG, "requestResponseInformation Properties %i", properties.requestResponseInformation);
            pos = pos + 2;
            break;
        case 31:
            properties.reasonString = malloc(buffer[pos + 1] * 256 + buffer[pos + 2] + 1);
            memcpy(properties.reasonString, &buffer[pos + 3], buffer[pos + 1] * 256 + buffer[pos + 2]);
            properties.reasonString[buffer[pos + 1] * 256 + buffer[pos + 2]] = '\0';
            pos = pos + buffer[pos + 1] * 256 + buffer[pos + 2] + 1;
            ESP_LOGD(TAG, "reasonString Properties %s", properties.reasonString);
            free(properties.reasonString);
            break;
        case 33:
            properties.receiveMaximum = 256 * buffer[pos + 1] + buffer[pos + 2];
            pos = pos + 3;
            ESP_LOGD(TAG, "receiveMaximum Properties %i", properties.receiveMaximum);
            break;
        case 34:
            properties.topicAliasMaximum = 256 * buffer[pos + 1] + buffer[pos + 2];
            pos = pos + 3;
            ESP_LOGD(TAG, "topicAliasMaximum Properties %i", properties.topicAliasMaximum);
            break;
        case 35:
            properties.topicAlias = 256 * buffer[pos + 1] + buffer[pos + 2];
            pos = pos + 3;
            ESP_LOGD(TAG, "topicAliasMaximum Properties %i", properties.topicAliasMaximum);
            break;
        case 38:
            properties.userPropertyA = malloc(buffer[pos + 1] * 256 + buffer[pos + 2] + 1);
            memcpy(properties.userPropertyA, &buffer[pos + 3], buffer[pos + 1] * 256 + buffer[pos + 2]);
            properties.userPropertyA[buffer[pos + 1] * 256 + buffer[pos + 2]] = '\0';
            pos = pos + buffer[pos + 1] * 256 + buffer[pos + 2] + 2;
            ESP_LOGD(TAG, "userPropertyA Properties %s", properties.userPropertyA);
            free(properties.userPropertyA);
            properties.userPropertyB = malloc(buffer[pos + 1] * 256 + buffer[pos + 2] + 1);
            memcpy(properties.userPropertyB, &buffer[pos + 3], buffer[pos + 1] * 256 + buffer[pos + 2]);
            properties.userPropertyB[buffer[pos + 1] * 256 + buffer[pos + 2]] = '\0';
            pos = pos + buffer[pos + 1] * 256 + buffer[pos + 2] + 2;
            ESP_LOGD(TAG, "userPropertyB Properties %s", properties.userPropertyB);
            free(properties.userPropertyB);
            pos++;
            break;
        case 39:
            properties.maximumPacketSize = 256 * (256 * (256 * buffer[pos + 1] + buffer[pos + 2]) + buffer[pos + 3]) + buffer[pos + 4];
            pos = pos + 5;
            ESP_LOGD(TAG, "maximumPacketSize Properties %lu", properties.maximumPacketSize);
            break;

        default:
            pos = posx;
            break;
        }
    } while (pos < posx);
    return pos;
}
