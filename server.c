#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <curl/curl.h>
#include <curl/easy.h>

#define PORT 80
#define QUEUE 20
#define BUFFER_SIZE 4096
#define MAX_STRING 256
#define XQ_CURL_CLIENT_HEADER "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36"
#define XQ_CURL_COOKIE_HEADER "Cookie: aliyungf_tc=AQAAAKnH+yP03woAiU49O8zEmcY2VNyA; acw_tc=2760823c15764840817188111ef079e879f818c0350366ff06a76c1903fc36; s=ds11kuup8i; xq_a_token=c9d3b00a3bd89b210c0024ce7a2e049f437d4df3; xq_r_token=8712d4cae3deaa2f3a3d130127db7a20adc86fb2; u=471576484068826; device_id=962a2b4385abd60f6c8f81cc5276d089; Hm_lvt_1db88642e346389874251b5a1eded6e3=1576484071; Hm_lpvt_1db88642e346389874251b5a1eded6e3=1576484071"
#define XQ_CURL_HOST_HEADER "Host: xueqiu.com"

struct MSG
{
    char *memory;
    size_t size;
};

size_t writeMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct MSG *mem = (struct MSG *)data;
    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) 
    {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}

LUALIB_API int __sendRequest(lua_State *l)
{
    const char *request = luaL_checkstring(l, -1);
    CURL *curl;
    CURLcode cc;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, request);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        struct curl_slist *header = NULL;
        header = curl_slist_append(header, XQ_CURL_CLIENT_HEADER);
        header = curl_slist_append(header, XQ_CURL_COOKIE_HEADER);
        header = curl_slist_append(header, XQ_CURL_HOST_HEADER);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
        struct MSG msg = { NULL, 0 };
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&msg);
        cc = curl_easy_perform(curl);
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200)
        {
            lua_pushstring(l, msg.memory);
            free(msg.memory);
            msg.memory = NULL;
        }
        curl_slist_free_all(header);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 1;
}

LUALIB_API int __sendHttpResponse(lua_State *l)
{
    const char *buf = luaL_checkstring(l, -1);
    int conn = luaL_checkinteger(l, -2);
    send(conn, buf, strlen(buf), 0);
    return 0;
}

LUALIB_API int __sendFile(lua_State *l)
{
    const char *filename = luaL_checkstring(l, -1);
    int conn = luaL_checkinteger(l, -2);
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long fs = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *d = malloc(fs);
    memset(d, 0, fs);
    fread(d, fs, sizeof(unsigned char), f); 
    char fileHeader[BUFFER_SIZE];
    memset(fileHeader, 0, BUFFER_SIZE);
    static const char *fileHeaderString =
"HTTP/1.1 200 OK\r\n"
"Cache-Control: max-age: 3600\r\n"
"Content-Length: ";
    sprintf(fileHeader, "%s%d\r\n\r\n\0", fileHeaderString, fs);
    send(conn, fileHeader, strlen(fileHeader), 0); 
    send(conn, d, fs, 0); 
    free(d);
    d = NULL;
    fclose(f);
}

void getLogString(char method[BUFFER_SIZE], char headers[BUFFER_SIZE], char path[BUFFER_SIZE], const char *buffer)
{
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    luaopen_table(L);
    luaopen_package(L);
    luaopen_io(L);
    luaopen_os(L);
    luaopen_string(L);
    luaL_openlibs(L);
    lua_pushstring(L, buffer);
    lua_setglobal(L, "__buffer");
    luaL_dofile(L, "./base.lua");
    lua_getglobal(L, "methodString");
    memset(method, 0, BUFFER_SIZE);
    strcpy(method, lua_tostring(L, -1));
    lua_getglobal(L, "headerString");
    memset(headers, 0, BUFFER_SIZE);
    strcpy(headers, lua_tostring(L, -1));
    lua_getglobal(L, "pathString");
    memset(path, 0, BUFFER_SIZE);
    strcpy(path, lua_tostring(L, -1));
    lua_close(L);
}

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(PORT);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1)
    {
        perror("!!  Error");
        return 1;
    }
    if (listen(sockfd, QUEUE) == -1)
    {
        perror("!!  Error");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    struct sockaddr_in clientAddr;
    socklen_t length = sizeof(clientAddr);
    char method[BUFFER_SIZE], headers[BUFFER_SIZE], path[BUFFER_SIZE];
    char datetime[MAX_STRING];
    while (1)
    {
        int conn = accept(sockfd, (struct sockaddr*)&clientAddr, &length);
        if (conn < 0)
        {
            perror("!!  failed to connect.");
            exit(1);
        }
        memset(buffer, 0, sizeof(buffer));
        recv(conn, buffer, sizeof(buffer), 0);

        clock_t t1 = clock();
        lua_State *L = luaL_newstate();
        luaopen_base(L);
        luaopen_table(L);
        luaopen_package(L);
        luaopen_io(L);
        luaopen_os(L);
        luaopen_string(L);
        luaL_openlibs(L);

        lua_pushcfunction(L, __sendRequest);
        lua_setglobal(L, "__sendRequest");
        lua_pushcfunction(L, __sendHttpResponse);
        lua_setglobal(L, "__sendHttpResponse");
        lua_pushcfunction(L, __sendFile);
        lua_setglobal(L, "__sendFile");
        lua_pushstring(L, buffer);
        lua_setglobal(L, "__buffer");
        lua_pushinteger(L, conn);
        lua_setglobal(L, "__conn");

        luaL_dofile(L, "./requests.lua");

        lua_close(L);
        clock_t t2 = clock();

        struct timeval tv;
        gettimeofday(&tv, NULL);
        struct tm *_tm;
        _tm = localtime((time_t*)&tv.tv_sec);
        char dt[MAX_STRING];
        memset(dt, 0, MAX_STRING);
        strftime(dt, MAX_STRING, "%Y-%m-%d %H:%M:%S", _tm);
        char ms[MAX_STRING];
        memset(ms, 0, MAX_STRING);
        sprintf(ms, "'%03d", tv.tv_usec / 1000);
        strcat(dt, ms);
        char *clientIP = inet_ntoa(clientAddr.sin_addr);
        int clientPort = ntohs(clientAddr.sin_port);
        getLogString(method, headers, path, buffer);
        printf("DateTime:[%s] From:[%s:%d] Path:[%s] Method:[%s] %s Takes:[%d]\n", dt, clientIP, clientPort, path, method, headers, (t2 - t1) / 1000);

        close(conn);
    }
    return 0;
}
