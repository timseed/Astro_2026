#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

#include "config.h"

#include "keywords.h"
#include "Options.h"
#include "Timer.h"
#include "xpUtil.h"

Timer::Timer(const int w, const unsigned long h, const unsigned long i) 
    : wait_(w), hibernate_(h), idlewait_(i)
{
}                       

Timer::~Timer()
{
}

void
Timer::Update()
{
    gettimeofday(&currentTime_, NULL);
    nextUpdate_ = currentTime_.tv_sec + wait_;
}

// Sleep for sleep_time seconds.
bool
Timer::SleepForTime(time_t sleep_time)
{
    if (sleep_time <= 0) 
        return(true);

    gettimeofday(&currentTime_, NULL);
    nextUpdate_ = sleep_time + currentTime_.tv_sec;
    
    if (static_cast<int> (sleep_time) != 1)
    {
        Options *options = Options::getInstance();
        if (options->Verbosity() > 0)
        {
            ostringstream msg;
            msg << "sleeping for " << static_cast<int> (sleep_time) 
                << " seconds until " << ctime((time_t *) &nextUpdate_);
            xpMsg(msg.str(), __FILE__, __LINE__);
        }
    }

    // Check every second if we've reached the time for the next
    // update.
    while (currentTime_.tv_sec < nextUpdate_)
    {
        sleep(1);
        gettimeofday(&currentTime_, NULL);
    }
    
    return(true);
}

// returns false if the program should exit after this sleep
bool
Timer::Sleep()
{
    // Sleep until the next update
    gettimeofday(&currentTime_, NULL);
    SleepForTime(nextUpdate_ - currentTime_.tv_sec);

    return(true);
}
