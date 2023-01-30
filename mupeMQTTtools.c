
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

#include "mupeMQTTtools.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
static const char *TAG = "mupeMQTTtools";

/**
 * @brief conwert a byte to binary 
 * 
 * @param x 
 * @return const char* 
 */

const char *byte_to_binary(uint8_t x)
{
	ESP_LOGD(TAG, "byte_to_binary %i", x);
	static char b[9];
	b[0] = '\0';
	int z;
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}

char *getString(uint8_t *payload)
{
	ESP_LOGD(TAG, "getString ");
	char *ret;
	ret = (char *)&(payload[2]);
	return ret;
}

uint32_t getRemainingLength(uint8_t *rx_buffer)
{
	uint32_t multiplier = 1;
	uint32_t value = 0;
	uint8_t pos = 0;
	do
	{
		pos++;
		value += (rx_buffer[pos] & 127) * multiplier;
		multiplier = multiplier * 128;
		ESP_LOGV(TAG, "l p : %i %i", rx_buffer[pos], pos);
		if (multiplier > (128 * 128 * 128))
		{
			return 0;
		}
	} while ((rx_buffer[pos] & 128) != 0);
	ESP_LOGV(TAG, "getRemainingLength %lu", value);
	return value;
}

uint8_t getSizeOfRemainingLength(uint8_t *rx_buffer)
{
	uint32_t multiplier = 1;
	uint32_t value = 0;
	uint8_t pos = 0;
	do
	{
		pos++;
		value += (rx_buffer[pos] & 127) * multiplier;
		multiplier = multiplier * 128;
		if (multiplier > (128 * 128 * 128))
		{
			return 0;
		}

	} while ((rx_buffer[pos] & 128) != 0);
	ESP_LOGD(TAG, "getSizeOfRemainingLength %i",pos);
	return pos;
}

uint16_t getTopicLenght(uint8_t *buffer)
{
	ESP_LOGD(TAG, "getTopicLenght **************");
	uint8_t k = getSizeOfRemainingLength(buffer);
	uint16_t l0 = buffer[1 + k] * 256 + buffer[2 + k];
	ESP_LOGV(TAG, "getTopicName länge: %i", l0);
	return l0;
}

uint8_t isTopic(char *topic, char *filter)
{
	ESP_LOGD(TAG, "isTopic %s %s", topic, filter);
	
	if (strcmp(topic, filter) == 0)
	{
		ESP_LOGV(TAG, "isTopic (strcmp(topic, filter) == 0) %i",
				 strcmp(topic, filter) == 0);
		return 1;
	}
	if ((topic[0] == '$') & (filter[0] == '#'))
	{
		ESP_LOGV(TAG, "isTopic ((topic[0] == '$') & (filter[0] == '#')) %i",
				 (topic[0] == '$') & (filter[0] == '#'));
		return 0;
	}
	if (strlen(filter) > strlen(topic))
	{
		ESP_LOGV(TAG, "isTopic (strlen(filter)>strlen(topic) %i",
				 strlen(filter) > strlen(topic));
		return 0;
	}
	uint8_t p = 0;
	for (uint8_t i = 0; i < strlen(topic); ++i)
	{
		if (p > strlen(filter))
		{
			ESP_LOGV(TAG, "isTopic (p > strlen(filter)) %i",
					 p > strlen(filter));
			return 0;
		}
		if (filter[p] == '#')
		{
			ESP_LOGV(TAG, "isTopic (filter[p] == '#') %i", filter[p] == '#');
			return 1;
		}
		if (filter[p] == '+')
		{
			if (topic[i] == '/')
			{
				p++;
				p++;
			}
		}
		else
		{
			if (topic[i] != filter[p])
			{
				ESP_LOGV(TAG, "isTopic (topic[i] != filter[p]) %i",
						 topic[i] != filter[p]);
				return 0;
			}
			p++;
		}
	}
	if (p > strlen(filter))
	{
		ESP_LOGV(TAG, "isTopic (p>sizeof(filter) %i %i", p, strlen(filter));
		return 0;
	}
	ESP_LOGV(TAG, "isTopic else");
	return 1;
}
void getTopicName(uint8_t *buffer, char *topicName, uint16_t lenght)
{
	ESP_LOGD(TAG, "getTopicName");
	uint8_t k = getSizeOfRemainingLength(buffer);

	for (int x1 = 0; x1 < lenght - 1; ++x1)
	{
		topicName[x1] = buffer[x1 + 3 + k];
	}
	topicName[lenght - 1] = '\0';
	ESP_LOGV(TAG, "getTopicName : %s", topicName);

}
