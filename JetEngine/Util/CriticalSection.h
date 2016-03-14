#ifndef CRITICAL_SECTION_HEADER
#define CRITICAL_SECTION_HEADER

#ifndef ANDROID
#include <Windows.h>
#else
#include <pthread.h>
#endif

class CriticalSection
{
#ifdef ANDROID
	pthread_mutex_t lock;
#else
	CRITICAL_SECTION lock;
#endif
public:
	CriticalSection()
	{
#ifdef ANDROID
		pthread_mutex_init(&lock, NULL);
#else
		InitializeCriticalSection(&lock);
#endif
	}

	~CriticalSection()
	{
#ifdef ANDROID
		pthread_mutex_destroy(&lock);
#else
		DeleteCriticalSection(&lock);
#endif
	}

	void Enter()
	{
#ifdef ANDROID
		pthread_mutex_lock(&lock);
#else
		EnterCriticalSection(&lock);
#endif
	};

	bool TryEnter()
	{
#ifdef ANDROID
		int res = pthread_mutex_trylock(&lock);
		if (res == EBUSY)
			return false;
		else
			return true;
#else
		return TryEnterCriticalSection(&lock);
#endif
	}

	void Leave()
	{
#ifdef ANDROID
		pthread_mutex_unlock(&lock);
#else
		LeaveCriticalSection(&lock);
#endif
	}
};

#endif