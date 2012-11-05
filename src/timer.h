#ifndef TIMER_H
#define TIMER_H

#include <cstdlib>
#include <sys/time.h>

enum TimeIds {
	TimeTotal,
	TimeLoad,
	TimeFind,
	TimeNone
};

/**
@brief
Calculate time
*/

class Timer {
public:
	static inline void start(TimeIds id = TimeTotal)
	{
		if (!enabled) return;
		data[id] -= microtime();
	}
	static inline void stop(TimeIds id = TimeTotal)
	{
		if (!enabled) return;
		data[id] += microtime();
	}
	static inline float secs(TimeIds id = TimeTotal)
	{
		return data[id] * 1e-6;
	}

private:
	static inline long unsigned int microtime()
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return (long unsigned int) (1e6*tv.tv_sec + tv.tv_usec);
	}

	static bool enabled;
	static int data[TimeNone];
};

#endif 
