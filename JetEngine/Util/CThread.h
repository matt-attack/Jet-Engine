#ifndef CTHREAD_HEADER
#define CTHREAD_HEADER

#ifndef _WIN32
#include <pthread.h>
#define THREAD_RETURN void*
#define THREAD_INPUT void*
#else
#include <Windows.h>
#define THREAD_RETURN DWORD WINAPI
#define THREAD_INPUT LPVOID
#endif
class CThread
{
#ifndef _WIN32
public:
	pthread_t thread;
	void Start(void *(*function)(void *), void* data)
	{
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		//pthread_attr_setstacksize(&attr, 1024*1024*5);

		pthread_create(&thread, &attr, function, data);
	}

	void Join(unsigned int timeout = 0)
	{
		int ret = pthread_join(thread,0);
		//if (ret != 0)
			//log("issue with pthread_join");
	}
#else
public:
	unsigned long threadid;
	HANDLE thread;
	void Start(LPTHREAD_START_ROUTINE function, void* data)
	{
		this->thread = CreateThread(NULL, 0, function, data, 0, &this->threadid);//pass it the pointer to a data array for the world heightmap and other expensive to generate data
	}

	void Join(unsigned int timeoutms = INFINITE)
	{
		WaitForSingleObject(thread, timeoutms);
	}
#endif
};

#endif
