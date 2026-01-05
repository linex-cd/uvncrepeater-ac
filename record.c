
#include "record.h"
#include "buffer.h"

/////////////////////////////////////
//写包

#ifndef __uint32_t_defined  
typedef unsigned int	uint32_t;  
#define __uint32_t_defined  
#endif


RET write_packet(FILE* file_handle, const void *buf, size_t len, struct timeval *tvp)
{
	
	uint32_t timestamp = htonl (1000 * tvp->tv_sec + tvp->tv_usec / 1000);
	uint32_t dlen = htonl (len);
	len = sizeof (uint32_t) * ( ( len + 3 ) / 4 );
	
	fwrite (&dlen, sizeof(uint32_t), 1, (FILE*)file_handle);
	fwrite (buf, 1, len, file_handle);
	fwrite (&timestamp, 4, 1, file_handle);

	return OK;
}




struct buffer g_buf;

//////////////////////////////////////////////////////////


// 录制线程
// 1.把数据流存到缓存队列
// 2.读取队列，如果拿到前3帧的数据
// 3.第三帧，即start+2+16地址的4字节为00 00 00 00,则认证成功，开始写入文件

void* record_thread(void* args)
{
	FILE* filehandle = (FILE*)args;

	size_t len = 0;
	void* data = 0;
	struct timeval timestamp;

	while (1)
	{
		//sleep(1);
		//usleep(1000 * 100); //100毫秒
		len = 0;
		data = 0;
		//int filehandle = -1; //返回任意filehandle
		//printf("while out %d\n", g_buf.length);
		buf_out(&g_buf, 1, &filehandle, &len, 0, 0);

		//必须转为long，否则-1是最大的unsigned
		if((long)len > -1)
		{

			//end sessoin
			if(len == 0)
			{
				
				buf_out(&g_buf, 0, &filehandle, 0, 0, 0);
				
				fclose((FILE*)filehandle);

				buf_uninit(&g_buf);
				break;
				
			}
			//record session
			else
			{
				data = malloc(len);
			
				buf_out(&g_buf, 0, &filehandle, &len, &data, &timestamp);
	
				write_packet((FILE*)filehandle, data, len, &timestamp);

				free(data);
			}


		}

	}
	printf("end recording\n");
	return 0;

}

/////////////////////////////////////

// 启动录制服务 start_record			
// 开始录制会话 begin_session			
// 保存通信数据：dispatch_session  把数据交给录制模块
// 结束录制:end_session 结束录制并保存文件

////////////////////////////////////////////////////////////

//开始录制
//API
RET get_filename(long id, char* filename)
{

	struct timezone tz;
	struct timeval time_start;
	struct timeval time_now;
	//char filename[MAXPATH];

	////////////////////////////////////////
	//创建文件
	gettimeofday (&time_start, &tz);

	time_t timep;
    time(&timep);

    struct tm* tinfo;
    tinfo = gmtime(&timep);

	int yy, mm, dd, hh, ii, ss;

	yy = 1900 + tinfo->tm_year;
	mm = 1 + tinfo->tm_mon;
	dd = tinfo->tm_mday;

	hh = tinfo->tm_hour - (tz.tz_minuteswest/60);
	ii = tinfo->tm_min;
	ss = tinfo->tm_sec;


	char dir[100] = {0};
	sprintf(dir, "%s/%d%02d%02d", ROOT, yy, mm, dd);
	
	mkdir(ROOT, S_IRWXU);
	mkdir(dir, S_IRWXU);

	sprintf(filename, "%s/%ld_%d%02d%02d_%02d%02d%02d.rbf", dir, id, yy, mm, dd, hh, ii, ss);

  
	return OK;

}

//开始录制
//API
RET begin_session(FILE* filehandle)
{
	////////////////////////////////////////
	//写入头
	struct timeval start_time;
	start_time.tv_sec = 0;
	start_time.tv_usec = 0;
	//第一帧 认证类型 2 bytes
	//1:01  有一个安全类型  2:01 不可用  01 无认证  02  使用VNC认证 
	//01  02
	char frame1[2] = {0};
	memcpy(frame1, "\x01\x02", 2);
	write_packet (filehandle, frame1, 2, &start_time);
    //第二帧 DES秘钥  16 bytes
	char frame2[16] = {0};
	memcpy(frame2, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);

	write_packet (filehandle, frame2, 16, &start_time);

	//第三帧 认证结果 4 bytes 
	char frame3[4] = {0};
	memcpy(frame3, "\x00\x00\x00\x00", 4);

	write_packet (filehandle, frame3, 4, &start_time);



	buf_init(&g_buf);

	return OK;

}

//结束录制
//API
RET end_session(FILE* filehandle)
{

	struct timeval time_now;

	gettimeofday(&time_now, NULL);

	buf_in(&g_buf, (int)filehandle, 0, 0, time_now);

    return OK;
}

//队列情况
//API
int get_buf_length(FILE*  filehandle)
{
	
	int length = buf_size(&g_buf, (int)filehandle);

    return length;
}

//传入数据
//API
RET dispatch_session(FILE*  filehandle, size_t len, void* data)
{
	//printf("dispatch_session p_buffer=%x, g_buf_head=%x\n", &g_buf, &(g_buf.p_buf_head));

	struct timeval time_now;

	gettimeofday(&time_now, NULL);

	buf_in(&g_buf, (int)filehandle, len, data, time_now);

	return OK;
}





