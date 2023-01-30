#include "mupeMQTTdata.h"
#include "mupeMQTTtools.h"
#include "mupeMQTTflow.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "mupeMQTTnvs.h"
//#include "mupeMdnsNtp.h"

static const char *TAG = "mupeMQTTData";

typedef struct TopiclistS TopiclistS;
struct TopiclistS
{
	char *topic;
	int sock;
	uint8_t protocolLevel;
	uint8_t qos;
	TopiclistS *next;
};

TopiclistS *topiclistP = NULL;
TopiclistS *topiclistget = NULL;



void topicListAdd(char *topic, int sock, uint8_t protocolLevel, uint8_t qos)
{

	ESP_LOGD(TAG, "addTopicList : sock %i topic %s protocolLevel %i qos %i", sock, topic, protocolLevel, qos);
	TopiclistS *topiclist = NULL;
	TopiclistS *topiclistL = NULL;
	topiclistL = topiclistP;

	while (topiclistL != NULL)
	{
		ESP_LOGV(TAG, "TopicListAdd : %i %s", topiclistL->sock,
				 topiclistL->topic);
		if (sock == topiclistL->sock)
		{
			if (strcmp(topiclistL->topic, topic) == 0)
				return;
		}
		if (topiclistL->next != NULL)
		{
			if (topiclistL->next->sock > sock)
			{
				break;
			}
		}
		topiclistL = topiclistL->next;
	}
	topiclist = malloc(sizeof(TopiclistS));
	topiclist->topic = strdup(topic);
	topiclist->sock = sock;
	topiclist->protocolLevel = protocolLevel;
	topiclist->qos = qos;
	topiclist->next = NULL;

	if (topiclistP == NULL)
	{
		topiclistP = topiclist;
		topiclistget = topiclistP;
	}
	else if (topiclistL != NULL)
	{
		topiclist->next = topiclistL->next;
		topiclistL->next = topiclist;
	}
	else
	{
		topiclistL = topiclistP;
		while (topiclistL->next != NULL)
		{
			topiclistL = topiclistL->next;
		}
		topiclistL->next = topiclist;
	}
}

void topicListRemoveSock(int sock)
{
	ESP_LOGD(TAG, "removeTopicList : %i", sock);
	TopiclistS *current = topiclistP;
	TopiclistS *previous = NULL;
	while (true)
	{
		current = topiclistP;
		previous = NULL;
		if (current == NULL)
		{
			return;
		}

		while (current->sock != sock)
		{
			ESP_LOGV(TAG, "TopicList : %i %s", current->sock, current->topic);
			// if it is last node
			if (current->next == NULL)
			{
				return;
			}
			else
			{
				// store reference to current link
				previous = current;
				// move to next link
				current = current->next;
			}
		}
		// found a match, update the link
		if (current == topiclistP)
		{
			// change first to point to next link
			topiclistP = topiclistP->next;
		}
		else
		{
			// bypass the current link
			previous->next = current->next;
		}
	}
}

void topicListRemove(char *topic, int sock)
{
	ESP_LOGD(TAG, "removeTopicList : %i %s", sock, topic);
	TopiclistS *current = topiclistP;
	TopiclistS *previous = NULL;
	if (current == NULL)
	{
		return;
	}

	while (!((strcmp(current->topic, topic) == 0) & (current->sock == sock)))
	{
		ESP_LOGV(TAG, "TopicList : %i %s", current->sock, current->topic);
		// if it is last node
		if (current->next == NULL)
		{
			return;
		}
		else
		{
			// store reference to current link
			previous = current;
			// move to next link
			current = current->next;
		}
	}
	// found a match, update the link
	if (current == topiclistP)
	{
		// change first to point to next link
		topiclistP = topiclistP->next;
	}
	else
	{
		// bypass the current link
		previous->next = current->next;
	}
}

int topicListGetSock(char *topic, uint8_t *protocolLevel)
{
	int ret = -1;
	ESP_LOGD(TAG, "getTopicListSock :  %s", topic);
	while (topiclistget != NULL)
	{
		ESP_LOGV(TAG, "getTopicListSock : %i %s", topiclistget->sock,
				 topiclistget->topic);

		if (isTopic(topic, topiclistget->topic))
		{
			ESP_LOGV(TAG, "getTopicListSock return :  %i", topiclistget->sock);
			ret = topiclistget->sock;
			*protocolLevel = topiclistget->protocolLevel;

			topiclistget = topiclistget->next;
			return ret;
		}
		topiclistget = topiclistget->next;
	}
	topiclistget = topiclistP;
	ESP_LOGV(TAG, "getTopicListSock : Ende Liste");
	return ret;
}

void topicListGet(httpd_req_t *req)
{
	ESP_LOGD(TAG, "getTopcList");
	char a[50];
	sprintf(a, "sock topic protocolLevel qos  <br>");
	httpd_resp_send_chunk(req, a, strlen(a));
	while (topiclistget != NULL)
	{
		ESP_LOGV(TAG, "getTopcList : %i %s", topiclistget->sock,
				 topiclistget->topic);
		char a[50];
		sprintf(a, " %i  %s  %i  %i <br>", topiclistget->sock, topiclistget->topic, topiclistget->protocolLevel, topiclistget->qos);
		httpd_resp_send_chunk(req, a, strlen(a));
		topiclistget = topiclistget->next;
	}
	topiclistget = topiclistP;
}


