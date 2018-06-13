#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include "error.h"
#include "epoll_event.h"


ep_element_t* ep_element_new(int fd, uint32_t events)
{
    ep_element_t *element = calloc(1, sizeof(ep_element_t));
    if (NULL != element) {
        element->fd = fd;
        element->events = events;
    }
    return element;
}

ep_event_t* ep_event_new(int timeout)
{
    ep_event_t *ep_event = calloc(1, sizeof(ep_event_t));
    if (!ep_event) {
        return NULL;
    }
    ep_event->table = hash_table_new(REF_MODE);
    if (!ep_event->table) {
        free(ep_event);
        return NULL;
    }
    ep_event->timeout = timeout;
    ep_event->epoll_fd = epoll_create(EP_MAX_ELEMENTS);
    return ep_event;
}

void ep_event_delete(ep_event_t* ep_event)
{
    hash_table_delete(ep_event->table);
    close(ep_event->epoll_fd);
    free(ep_event);
}

int ep_event_remove(ep_event_t* event, int fd)
{
    hash_table_remove(event->table, &fd, sizeof(int));
    close(fd);
    epoll_ctl(event->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    return 0;
}

int ep_event_add(ep_event_t *ep_event, int fd, uint32_t flags, ep_element_t **ep_element)
{
    int ret;
    *ep_element = calloc(1, sizeof(ep_element_t));
    if (!(*ep_element)) {
        exit_throw("failed to memory allocation!\n");
    }
    if ((ret = hash_table_add(ep_event->table, &fd, sizeof(int), *ep_element, sizeof(ep_element_t))) < 0) {
        error_display("failed to add element for hashtable, sock id: %d\n", fd);
        return -2;
    }
    (*ep_element)->fd = fd;
    (*ep_element)->events = flags;
    struct epoll_event ee;
    memset(&ee, 0, sizeof(struct epoll_event));
    ee.data.fd = fd;
    ee.events = flags;
    if (ret == 1) {
        /*返回值为1修改模式*/
        return epoll_ctl(ep_event->epoll_fd, EPOLL_CTL_MOD, fd, &ee);
    }
    return epoll_ctl(ep_event->epoll_fd, EPOLL_CTL_ADD, fd, &ee);
}

static int ep_event_process(ep_event_t *ep_event)
{
    struct epoll_event events[EP_MAX_ELEMENTS];
    int fds = epoll_wait(ep_event->epoll_fd, events, EP_MAX_ELEMENTS, ep_event->timeout);
    /*错误*/
    if (-1 == fds) {
        exit_throw("failed to epoll_wait!\n");
    }
    /*超时*/
    if (0 == fds && NULL != ep_event->timeout_cb) {
        ep_event->timeout_cb(ep_event);
        return 1;
    }

    for (int i = 0; i < fds; i++) {
        ep_element_t *element = NULL;
        if (NULL == (element = (ep_element_t*)hash_table_lookup(ep_event->table, &events[i].data.fd, sizeof(int)))) {
            /*没找到，异常*/
            error_display("not found hashtable for sock id: %d\n", events[i].data.fd);
            continue;
        }

        if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI)) {
            if (element->accept_cb) {
                element->accept_cb(ep_event, element, events[i]);
            }
            if (element->read_cb) {
                element->read_cb(ep_event, element, events[i]);
            }
        }

        if ((events[i].events & EPOLLOUT) && NULL != element->write_cb) {
            element->write_cb(ep_event, element, events[i]);
        }

        /*正常关闭*/
        if ((events[i].events & EPOLLRDHUP) && NULL != element->close_cb) {
            element->close_cb(ep_event, element, events[i]);
        }

        /*异常*/
        if (((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) && NULL != element->error_cb) {
            element->error_cb(ep_event, element, events[i]);
        }
    }
    return 1;
}

void ep_event_start(ep_event_t *ep_event)
{
    while (ep_event_process(ep_event));
}