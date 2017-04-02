#include "CTimer.h"

#ifndef _WIN32
#include <sys/time.h>
#include <time.h>

INT64 gettime()//returns time in microseconds
{
	static __time_t start;
	timespec time;
	//gettimeofday(&time, 0);
	clock_gettime(CLOCK_MONOTONIC, &time);
	return (INT64)time.tv_sec*1000000 + time.tv_nsec/1000;
}

/*unsigned gettime()//GetTickCount()
{
        struct timeval tv;
        if(gettimeofday(&tv, NULL) != 0)
                return 0;

        return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}*/
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Summary: Default constructor.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CTimer::CTimer()
{
#ifdef _WIN32
    QueryPerformanceFrequency( (LARGE_INTEGER *)&m_ticksPerSecond );
#else
	m_ticksPerSecond = 1000000;
#endif

    m_currentTime = m_lastTime = m_lastFPSUpdate = 0;
    m_numFrames = 0;
    m_runningTime = m_timeElapsed = m_fps = 0.0f;
    m_FPSUpdateInterval = m_ticksPerSecond >> 1;
    m_timerStopped = true;
}

void CTimer::Reset()
{
	m_runningTime = m_timeElapsed = 0.0f;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Summary: Starts the timer.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CTimer::Start()
{
	if ( !m_timerStopped )
    {
        // Already started
        return;
    }
#ifdef _WIN32
    QueryPerformanceCounter( (LARGE_INTEGER *)&m_lastTime );
#else
	this->m_lastFPSUpdate = gettime();
	m_lastTime = gettime();
#endif
    m_timerStopped = false;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Summary: Stops the timer.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CTimer::Stop()
{
    if ( m_timerStopped )
    {
        // Already stopped
        return;
    }
    INT64 stopTime = 0;
#ifdef _WIN32
    QueryPerformanceCounter( (LARGE_INTEGER *)&stopTime );
#else 
	stopTime = gettime();
#endif
    m_runningTime += (float)(stopTime - m_lastTime) / (float)m_ticksPerSecond;
    m_timerStopped = false;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
Summary: Updates the timer. Calculates the time elapsed since the last Update call.
Updates the frames per second and updates the total running time.     
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CTimer::Update()
{
    if ( m_timerStopped )
    {
        return;
    }

    // Get the current time
#ifdef _WIN32
    QueryPerformanceCounter( (LARGE_INTEGER *)&m_currentTime );
#else	
	m_currentTime = gettime();
#endif
    
    m_timeElapsed = (float)(m_currentTime - m_lastTime) / (float)m_ticksPerSecond;
    m_runningTime += m_timeElapsed;

    // Update FPS
    m_numFrames++;
    if ( m_currentTime - m_lastFPSUpdate >= m_FPSUpdateInterval )
    {
        double currentTime = (double)m_currentTime / (double)m_ticksPerSecond;
		double lastTime = (double)m_lastFPSUpdate / (double)m_ticksPerSecond;
		m_fps = (double)m_numFrames / (currentTime - lastTime);

        m_lastFPSUpdate = m_currentTime;
        m_numFrames = 0;
    }

    m_lastTime = m_currentTime;
}