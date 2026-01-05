//header for a buffer list

#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

struct buf_node
{
    int handle;
    size_t len;
    void* data;
    struct timeval timestamp;
	struct buf_node* next;
};


struct buffer
{
	size_t length;
	struct buf_node* p_buf_head;
	struct buf_node* p_buf_tail;
};


int buf_init(struct buffer* p_buffer);
int buf_uninit(struct buffer* p_buffer);
int buf_in(struct buffer* p_buffer, int handle, size_t len, void* data, struct timeval timestamp);
int buf_out(struct buffer* p_buffer, int is_peek, int* handle, size_t* len, void** data, struct timeval* timestamp);
int buf_seek(struct buffer* p_buffer, int handle, size_t from, size_t to, size_t* len, void** data);
int buf_clear(struct buffer* p_buffer);
int buf_size(struct buffer* p_buffer, int handle);

#endif