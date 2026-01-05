//source file for buffer list

#include "buffer.h"
#include <pthread.h>


//互斥锁
pthread_mutex_t _mutex;

int buf_init(struct buffer* p_buffer)
{
	
	p_buffer = (struct buffer*)malloc(sizeof(struct buffer));

	p_buffer->p_buf_head = 0;
	p_buffer->p_buf_tail = 0;
	p_buffer->length = 0;

	pthread_mutex_init(&_mutex, NULL);

	return 1;
}

int buf_uninit(struct buffer* p_buffer)
{

	buf_clear(p_buffer);

	pthread_mutex_destroy(&_mutex);
	return 1;
} 


//缓冲队列
//handle使用-1以上的值，-1在出队时入参表示任意
int buf_in(struct buffer* p_buffer, int handle, size_t len, void* data, struct timeval timestamp)
{
	pthread_mutex_lock(&_mutex); 

	struct buf_node* node = malloc(sizeof(struct buf_node));

	node->next = 0;
	node->handle = handle;

	node->len = len;
	node->data = 0;

	if(len > 0)
	{

		node->data = malloc(len);
		memcpy(node->data, data, len);
	}
	
	node->timestamp = timestamp;
	
	if (p_buffer->p_buf_tail)
	{
		p_buffer->p_buf_tail->next = node;
	}

	p_buffer->p_buf_tail = node;
	p_buffer->length = p_buffer->length + 1;

	if (p_buffer->p_buf_head == 0)
	{
		p_buffer->p_buf_head = p_buffer->p_buf_tail;
	}
	//printf("buf_in ok p_buffer->length=%d\n", p_buffer->length);

	pthread_mutex_unlock(&_mutex); 

	return 1;
} 

//handle == 0 返回任意
//handle !=0 && *handle == -1 返回任意
//handle !=0 && *handle != -1 返回指定
int buf_out(struct buffer* p_buffer, int is_peek, int* handle, size_t* len, void** data, struct timeval* timestamp)
{	
	
	pthread_mutex_lock(&_mutex); 

	//默认找不到的话是-1
	if(len)
	{
		*len = -1;
	}


	struct buf_node* tmp_node = p_buffer->p_buf_head;
	struct buf_node* last_node = p_buffer->p_buf_head;
	//printf("buf out seek=%d, p_buffer->p_buf_head=%x  tmp_node=%x\n", is_peek, p_buffer->p_buf_head, tmp_node);
	while (tmp_node)
	{
		//printf("handle=%x *handle=%d tmp_node->handle=%x \n", handle, *handle, tmp_node->handle);
		//handle == 0 返回任意
		//handle !=0 && *handle == -1 返回任意
		//handle !=0 && *handle != -1 返回指定
		if (handle == 0 || ( handle && *handle == -1) || (handle && *handle != -1 && tmp_node->handle == *handle))
		{
			//printf("found node=%x\n", tmp_node);
			if(handle)
			{
				*handle = tmp_node->handle;
			}
			

			if(len)
			{
				*len = tmp_node->len;
			}
			
			if(data)
			{
				if(tmp_node->len > 0)
				{
					memcpy(*data, tmp_node->data, tmp_node->len);
				}
				
			}
			

			if(timestamp)
			{
				*timestamp = tmp_node->timestamp;
			}
			
			//非PEEK，就移除节点
			if (is_peek == 0)
			{
				//释放节点数据
				if(tmp_node->len > 0)
				{
					free(tmp_node->data);
				}
				
				//释放节点
				struct buf_node* to_free = tmp_node;
				tmp_node = tmp_node->next;
				free(to_free);
				
				//printf("last_node=%x tmp_node=%x, \n", last_node, tmp_node);
				//上一个系节点的指针指向下一个节点
				last_node->next = tmp_node;

				//下一个节点如果是空的话，列尾置空
				if(tmp_node == 0)
				{
					p_buffer->p_buf_tail = 0;
				}

				//如果释放的是头节点，列头更新
				if(p_buffer->p_buf_head == to_free)
				{
					p_buffer->p_buf_head = tmp_node;
				}

				p_buffer->length = p_buffer->length - 1;
			}

			break;
			

		}


		last_node = tmp_node;
		tmp_node = tmp_node->next;
		
	}
	
	//printf("buf out length=%d\n", p_buffer->length);
	pthread_mutex_unlock(&_mutex); 

	return 1;

} 


int buf_clear(struct buffer* p_buffer)
{
	pthread_mutex_lock(&_mutex); 

	struct buf_node* to_free = 0;
	while (p_buffer->p_buf_head)
	{
		
		to_free = p_buffer->p_buf_head;
		p_buffer->p_buf_head = p_buffer->p_buf_head->next;

		free(to_free);

		
	}
	p_buffer->p_buf_head = 0;
	p_buffer->p_buf_tail = 0;
	p_buffer->length = 0;

	pthread_mutex_unlock(&_mutex); 
	return 1;
} 

int buf_seek(struct buffer* p_buffer, int handle, size_t from, size_t to, size_t* len, void** data)
{
	pthread_mutex_lock(&_mutex);

	size_t index = 0;
	size_t buf_len = 0;
	struct buf_node* tmp_node = p_buffer->p_buf_head;

	while (tmp_node)
	{
		if(tmp_node->handle == handle && index < to)
		{
			if (index >= from)
			{
				if(data)
				{
					memcpy((*data + buf_len), tmp_node->data, tmp_node->len);			
				}

				buf_len = buf_len + tmp_node->len;
				if(len)
				{
					*len = buf_len;
				}


			}
			index = index + 1;
		}
		
		tmp_node = tmp_node->next;
	
	}

	pthread_mutex_unlock(&_mutex); 
	return 1;
}

int buf_size(struct buffer* p_buffer, int handle)
{

	pthread_mutex_lock(&_mutex);

	size_t buf_len = 0;
	struct buf_node* tmp_node = p_buffer->p_buf_head;

	while (tmp_node)
	{

		if(handle == 0 || tmp_node->handle == handle)
		{
			
			buf_len = buf_len + 1;
		}
		
		tmp_node = tmp_node->next;
	
	}

	pthread_mutex_unlock(&_mutex); 
	return buf_len;
}
