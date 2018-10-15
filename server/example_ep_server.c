#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "epoll_event.h"
#include "error.h"


#define UNUSED(x)   (void)x

/*server端口号*/
#define SERVER_PORT            8080

/*epoll超时时间，单位ms*/
#define EPOLL_TIMEOUT       10000

static unsigned long long count = 0;

static time_t t1, t2;

void read_callback(ep_event_t *event, ep_element_t *element, struct epoll_event ee)
{
    UNUSED(event);
    UNUSED(ee);

    char buf[1024] = {0};
    int val = read(element->fd, buf, 1024);

    if (val <= 0)
        return;

    printf("received data : %s\n", buf);
    return;

    if (count == 0)
        t1 = time(NULL);

    count++;
    if (count % (1024 * 1024) == 0) {
        t2 = time(NULL);
        printf("received data %llu times, and used %lus\n", count, t2 - t1);
    }
}

void write_callback(ep_event_t *event, ep_element_t *element, struct epoll_event ee)
{
    UNUSED(event);
    UNUSED(element);
    UNUSED(ee);
}

void close_callback(ep_event_t *event, ep_element_t *element, struct epoll_event ee)
{
    UNUSED(event);
    UNUSED(ee);

    printf("client closed, sd : %d\n", element->fd);

    ep_event_remove(event, element->fd);

    /*自定义数据自己定义自己处理*/
    if (!element->custom_data) {
        free(element->custom_data);
        element->custom_data = NULL;

    }
}

/*示例，使用自定义custom_data简单写入字符串*/
void error_callback(ep_event_t *event, ep_element_t *element, struct epoll_event ee)
{
    UNUSED(event);
    UNUSED(ee);

    /*自定义数据自己定义自己处理*/
    if (!element->custom_data) {
        element->custom_data = calloc(1, 32);
        if (!element->custom_data) return;
    }

    sprintf(element->custom_data, "error fd: %d\n", element->fd);
    printf("%s\n", (char*)element->custom_data);
}

void accept_callback(ep_event_t *event, ep_element_t *element, struct epoll_event ee)
{
    UNUSED(ee);
    struct sockaddr_in sock_addr;
    socklen_t sock_len = sizeof(sock_addr);
    int listenfd = accept(element->fd, (struct sockaddr*) &sock_addr, &sock_len);
    fcntl(listenfd, F_SETFL, O_NONBLOCK);
    fprintf(stderr, "got the socket %d\n", listenfd);
    uint32_t flags = EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    ep_element_t *ac_element;
    ep_event_add(event, listenfd, flags, &ac_element);
    ac_element->read_cb = read_callback;
    ac_element->write_cb = write_callback;
    ac_element->close_cb = close_callback;
    ac_element->error_cb = error_callback;
}

/*示例，使用自定义event_data简单+1打印*/
int ep_timeout_callback(ep_event_t *event)
{
    /*自定义数据自己定义自己处理*/
    if (!event->event_data) {
        event->event_data = calloc(1, sizeof(int));
        if (!event->event_data) {
            return -1;
        }
    }

    (*(unsigned int*)event->event_data)++;
    printf("timeout %d\n", *(unsigned int*)event->event_data);

    return 0;
}

int main()
{
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0 , sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    listen(sd, 16);

    fcntl(sd, F_SETFL, O_NONBLOCK);

    ep_event_t *ee = ep_event_new(EPOLL_TIMEOUT);

    ee->timeout_cb = ep_timeout_callback;

    ep_element_t *element;
    ep_event_add(ee, sd, EPOLLIN, &element);
    element->read_cb = read_callback;
    element->accept_cb = accept_callback;
    element->close_cb = close_callback;

    ep_event_start(ee);

    return 0;
}