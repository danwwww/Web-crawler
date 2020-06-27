#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "header.h"

// Determine if the url is valid
int IsValidUrl(char *url)
{
	// There will be no URLs using protocols other than http
	if (strstr(url, "http://") == NULL)
		return 0;

	// You can ignore URLs containing . and .. path segments
	if (strstr(url, "./") != NULL || strstr(url, "../") != NULL)
		return 0;

	// You can ignore URLs containing % or # or ?
	if (strchr(url, '%') != NULL || strchr(url, '#') != NULL || strchr(url, '?') != NULL || strchr(url, '\n') != NULL)
		return 0;

	// You do not need to parse URLs longer than 1000 bytes
	if (strlen(url) > 1000)
		return 0;

	// There will be no URLs using ports other than 80
	char *port = strstr(url + 7, ":");
	if (port == NULL)
	{
		// default port is 80
	}
	else
	{
		// There is a port, but we need to determine whether the port is 80
		int num = atoi(port + 1);
		if (num != 80)
			return 0;
	}

	return 1;
}

int Init(Http *http)
{
	http->m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (http->m_socket < 0)
		return 0;

	return 1;
}

/*
Parsing URL: Determine if the URL is legal.
Extract host and path if legal.
*/
int AnalyseUrl(Http *http)
{
	char *url = http->url;

	char *p = strchr(&url[7], '/');
	if (p == NULL)
	{
		// not found
		int host_length = strlen(url) - 7;
		char *host = (char*)calloc(host_length + 1, 1);
		strncpy(host, &url[7], host_length);

		int object_length = 1;
		char *object = (char*)calloc(object_length + 1, 1);
		object[0] = '/';

		http->m_host = host;
		http->m_object = object;
	}
	else
	{
		int host_length = strlen(url) - 7 - strlen(p);
		char *host = (char*)calloc(host_length + 1, 1);
		strncpy(host, &url[7], host_length);

		int object_length = strlen(p);
		char *object = (char*)calloc(object_length + 1, 1);
		strncpy(object, p, object_length);

		http->m_host = host;
		http->m_object = object;
	}

	return 1;
}

/*
Connect to a remote server
*/
int Connect(Http *http)
{
	// Parse IP address
	struct hostent* p = gethostbyname(http->m_host);
	if (p == NULL)
		return 0;

	// Connect server
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);	// Converts host byte order to network byte order
	memcpy(&sa.sin_addr, p->h_addr, 4);

	if (connect(http->m_socket, (struct sockaddr*)&sa, sizeof(struct sockaddr)) < 0)
		return 0;

	return 1;
}

// Get the HTML code through a Get request
char *FetchGet(Http *http)
{
	char info[200];
	memset(info, 0, 200);
	sprintf(info, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: danw8\r\nConnection: Close\r\n\r\n", http->m_object, http->m_host);

	send(http->m_socket, info, strlen(info), 0);

	char *buf = (char*)calloc(200000, 1);
	char ch;
	int i = 0;
	while (recv(http->m_socket, &ch, sizeof(ch), 0))
	{
		buf[i] = ch;
		i++;
	}

	return buf;
}

void CleanUp(Http *http)
{
	free(http->m_host);
	free(http->m_object);
	close(http->m_socket);
}


void ParseHtml(char *html, Http *http)
{

	char *temp = html;
	while (1)
	{
		// Find the href in the a tag
		char *start = strstr(temp, "<a ");
		char *end = strstr(temp, "</a>");

		if (start == NULL)
			break;

		char *p = strstr(start, "href");
		//int pos = strlen(temp) - strlen(p);

		char *href_left = strchr(p, '\"');
		char *href_right = strchr(href_left + 1, '\"');

		int href_length = strlen(href_left) - strlen(href_right) - 1;

		char *url = (char*)calloc(href_length + 1, 1);
		memcpy(url, href_left + 1, href_length);

		// Determine if the URL contains host
		if (strstr(url, http->m_host) != NULL)
		{
			if (IsValidUrl(url) == 1)
			{
				//printf("url: %s\n", url);
				// Determine if the url is in the url collection, if not, add it to the url collection
				int tag = 0;
				for (int i = 0; i < LEN; i++)
				{
					if (strcmp(url, url_array[i]) == 0)
					{
						tag = 1;
						break;
					}
				}

				if (tag == 0)
				{
					if (LEN < HTTP_MAX_NUM)
					{
						url_array[LEN] = url;
						LEN++;
					}
				}
			}
		}

		temp = end + 4;
	}
}

void StartCatch(char *init_url)
{
	if (IsValidUrl(init_url))
	{
		int url_length = strlen(init_url);
		url_array[LEN] = (char*)calloc(url_length + 1, 1);
		memcpy(url_array[LEN], init_url, url_length);

		LEN++;
	}
	else
	{
		return;
	}

	while (CUR < LEN && LEN <= HTTP_MAX_NUM)
	{
		// Take a url from the url collection and get the html code corresponding to the url
		Http http;
		http.url = url_array[CUR];

		if (Init(&http) == 0)
		{
			// sleep
			fprintf(stderr, "Init failed\n");
			continue;
		}

		if (AnalyseUrl(&http) == 0)
		{
			// sleep
			fprintf(stderr, "AnalyseUrl failed\n");
			continue;
		}

		if (Connect(&http) == 0)
		{
			// sleep
			fprintf(stderr, "Connect failed\n");
			continue;
		}

		char *html = FetchGet(&http);
		if (html == NULL)
		{
			// sleep
			fprintf(stderr, "FetchGet failed\n");
			continue;
		}

		ParseHtml(html, &http);

		if (CUR != 0)
		{
			//printf("%s %d\n", http.url, LEN);
			fprintf(stdout, "%s\n", http.url);
		}

		CUR++;

		// free allocated space
		free(html);
		CleanUp(&http);
	}

	for (int i = 0; i < LEN; i++)
		free(url_array[i]);
}
