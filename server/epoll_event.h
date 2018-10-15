#ifndef _EP_SERVER_H_
#define _EP_SERVER_H_

#include <sys/epoll.h>
#include "hashtable.h"


#define EP_MAX_ELEMENTS		1024

typedef struct ep_event ep_event_t;

typedef struct ep_element ep_element_t;

typedef void (*EP_CALLBACK)(ep_event_t *, ep_element_t *, struct epoll_event);


typedef struct ep_element
{
    int fd;

    uint32_t events;

    /*ep_element内自定义数据存放*/
    void *custom_data;

    /*回调函数*/
    EP_CALLBACK accept_cb;
    EP_CALLBACK read_cb;
    EP_CALLBACK write_cb;
    EP_CALLBACK close_cb;
    EP_CALLBACK error_cb;

} ep_element_t;



typedef struct ep_event
{
    hash_table_t *table;

    int epoll_fd;

    /*ep_event内自定义数据存放*/
    void *event_data;

    /*超时时间*/
    size_t timeout;

    /*超时处理函数，返回为0会忽略继续epoll事件循环*/
    int (*timeout_cb)(ep_event_t *);

} ep_event_t;


ep_event_t* ep_event_new(int );

int ep_event_add(ep_event_t *, int , uint32_t , ep_element_t **);

int ep_event_remove(ep_event_t* , int );

void ep_event_start(ep_event_t* );


#endif	/*!_EP_SERVER_H_*/