#define _CRT_SECURE_NO_WARNINGS
#define HTTP_MAX_NUM 100

static char *url_array[HTTP_MAX_NUM];
static int CUR = 0;
static int LEN = 0;

typedef struct
{
	char *url;
	char *m_host;
	char *m_object;
	int m_socket;
}Http;

int IsValidUrl(char *url);
int AnalyseUrl(Http *http);
int Connect(Http *http);
char *FetchGet(Http *http);
void CleanUp(Http *http);
void ParseHtml(char *html, Http *http);
void StartCatch(char *init_url);
