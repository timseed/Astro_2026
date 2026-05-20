#include <cstdlib>
#include <sstream>
using namespace std;

#include <sys/time.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <IOKit/IOKitLib.h>

#include "config.h"

#include "keywords.h"
#include "Options.h"
#include "xpUtil.h"

#include "TimerMacAqua.h"

TimerMacAqua::TimerMacAqua(const int w, const unsigned long h, 
                           const unsigned long i) : Timer(w, h, i)
{
}

TimerMacAqua::~TimerMacAqua()
{
}

// returns false if the program should exit after this sleep
bool
TimerMacAqua::Sleep()
{
    // Sleep until the next update
    gettimeofday(&currentTime_, NULL);
    if (!SleepForTime(nextUpdate_ - currentTime_.tv_sec)) 
        return(false);

    // If the display has not been idle for idlewait_
    // milliseconds, keep sleeping.  Check every second until the
    // display has been idle for long enough.
    if (idlewait_ > 0) 
    {
        unsigned long idle = GetSystemIdleTime();
        Options *options = Options::getInstance();
        if (options->Verbosity() > 0)
        {
            ostringstream msg;
            msg << "Idle time is " << idle/1000 << " second";
            if (idle/1000 != 1) msg << "s";
            msg << endl;
            xpMsg(msg.str(), __FILE__, __LINE__);
        }
        while (idle < idlewait_) 
        {
            gettimeofday(&currentTime_, NULL);
            if (!SleepForTime((idlewait_ - idle) / 1000))
                return(false);
            idle = GetSystemIdleTime();
        }
    }
    
    // If the display has been idle for longer than hibernate_
    // milliseconds, keep sleeping.  Check every second until
    // something happens.
    if (hibernate_ > 0)
    {
        unsigned long idle = GetSystemIdleTime();
        Options *options = Options::getInstance();
        if (options->Verbosity() > 0 && idle > hibernate_)
            xpMsg("Hibernating ...\n", __FILE__, __LINE__);

        while (idle > hibernate_) 
        {
            if (!SleepForTime(1)) 
                return(false);
            idle = GetSystemIdleTime();
        }
    }
    return(true);
}

// return the system idle time in milliseconds This code is based on
// "idler", found at
// http://www.macosxlabs.org/tools_and_scripts/script_archive/script_archive.html
// Their copyright notice follows:

/*****************************************
 * idler.c
 *
 * Uses IOKit to figure out the idle time of the system. The idle time
 * is stored as a property of the IOHIDSystem class; the name is
 * HIDIdleTime. Stored as a 64-bit int, measured in ns. 
 *
 * The program itself just prints to stdout the time that the computer has
 * been idle in seconds.
 *
 * Compile with gcc -Wall -framework IOKit -framework Carbon idler.c -o
 * idler
 *
 * Copyright (c) 2003, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *      
 * Neither the name of Stanford University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
unsigned long
TimerMacAqua::GetSystemIdleTime()
{
    unsigned long idle;
        
    mach_port_t masterPort;
    io_iterator_t iter;
    io_registry_entry_t curObj;
        
    IOMasterPort(MACH_PORT_NULL, &masterPort);
        
    /* Get IOHIDSystem */
    IOServiceGetMatchingServices(masterPort,
                                 IOServiceMatching("IOHIDSystem"),
                                 &iter);
    if (iter == 0)
    {
        xpWarn("Error accessing IOHIDSystem\n", __FILE__, __LINE__);
    }
        
    curObj = IOIteratorNext(iter);
        
    if (curObj == 0)
    {
        xpWarn("Iterator's empty!\n", __FILE__, __LINE__);
    }
        
    CFMutableDictionaryRef properties = 0;
    CFTypeRef obj;
        
    if (IORegistryEntryCreateCFProperties(curObj, &properties,
                                          kCFAllocatorDefault, 0)
        == KERN_SUCCESS && properties != NULL) 
    {
                
        obj = CFDictionaryGetValue(properties, CFSTR("HIDIdleTime"));
        CFRetain(obj);
    } 
    else 
    {
        xpWarn("Couldn't grab properties of system\n", __FILE__, __LINE__);
        obj = NULL;
    }
        
    if (obj) 
    {
        uint64_t tHandle;
                
        CFTypeID type = CFGetTypeID(obj);
                
        if (type == CFDataGetTypeID()) 
        {
            CFDataGetBytes((CFDataRef) obj,
                           CFRangeMake(0, sizeof(tHandle)), 
                           (UInt8*) &tHandle);   
        } 
        else if (type == CFNumberGetTypeID()) 
        {
            CFNumberGetValue((CFNumberRef)obj,
                             kCFNumberSInt64Type,
                             &tHandle);
        } 
        else 
        { 
            ostringstream errMsg;
            errMsg << (int) type << ": unsupported type\n";
            xpWarn(errMsg.str(), __FILE__, __LINE__);
        }
                
        CFRelease(obj);    
                
        // tHandle is in nanoseconds, we want milliseconds
        idle = static_cast<unsigned long> (tHandle / 1e6);
    } 
    else 
    {
        xpWarn("Can't find idle time\n", __FILE__, __LINE__);
    }
        
    /* Release our resources */
    IOObjectRelease(curObj);
    IOObjectRelease(iter);
    CFRelease((CFTypeRef)properties);
        
    return(idle);
}
