#ifndef CTIMER_H
#define CTIMER_H

#ifndef _WIN32
typedef long long INT64;
typedef unsigned int UINT;
#else
#include <Windows.h>
#endif

class CTimer
{
public:
    CTimer();
    void Start();
    void Stop();
    void Update();
	void Reset();

    bool IsStopped() { return m_timerStopped; }
    float GetFPS() { return m_fps; }
    float GetRunningTime() { return m_runningTime; }
    float GetElapsedTime() { return m_timerStopped ? 0.0f : m_timeElapsed; }
private:
    INT64 m_ticksPerSecond;
    INT64 m_currentTime;
    INT64 m_lastTime;
    INT64 m_lastFPSUpdate;
    INT64 m_FPSUpdateInterval;
    UINT m_numFrames;
    float m_runningTime;
    float m_timeElapsed;
    float m_fps;
    bool m_timerStopped;
};
#endif