/**
 * @file
 * System logging facility for daemons.
 */

/******************************************************************************
 *
 *
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <qcc/platform.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#if defined(QCC_OS_GROUP_WINDOWS) || defined(QCC_OS_GROUP_WINRT)
#elif defined(QCC_OS_ANDROID)
#include <android/log.h>
#else
#include <syslog.h>
#endif

#include <qcc/Debug.h>
#include <qcc/Logger.h>
#include <qcc/Util.h>

using namespace qcc;


#if defined(QCC_OS_ANDROID)
static const int androidPriorityMap[] = {
    ANDROID_LOG_FATAL,      // LOG_EMERG
    ANDROID_LOG_ERROR,      // LOG_ALERT
    ANDROID_LOG_ERROR,      // LOG_CRIT
    ANDROID_LOG_ERROR,      // LOG_ERR
    ANDROID_LOG_WARN,       // LOG_WARNING
    ANDROID_LOG_DEFAULT,    // LOG_NOTICE
    ANDROID_LOG_INFO,       // LOG_INFO
    ANDROID_LOG_DEBUG       // LOG_DEBUG
};

#endif


void qcc::Log(int priority, const char* format, ...)
{
    LoggerSetting* loggerSettings = LoggerSetting::GetLoggerSetting();
    va_list ap;


    loggerSettings->lock.Lock();

#if !defined(QCC_OS_GROUP_WINDOWS) && !defined(QCC_OS_GROUP_WINRT)
    if (loggerSettings->UseSyslog()) {

#if defined(QCC_OS_ANDROID)
        if (priority <= loggerSettings->GetLevel()) {
            va_start(ap, format);
            __android_log_vprint(androidPriorityMap[priority], loggerSettings->name, format, ap);
            va_end(ap);
        }
#else  // QCC_OS_LINUX || QCC_OS_DARWIN

        va_start(ap, format);
        vsyslog(priority, format, ap);
        va_end(ap);

#endif

    }
#endif

    if (loggerSettings->UseStdio()) {
        if (priority <= loggerSettings->GetLevel()) {
            va_start(ap, format);
            vfprintf(loggerSettings->GetFile(), format, ap);
            va_end(ap);
            fflush(loggerSettings->GetFile());
        }
    }

    loggerSettings->lock.Unlock();
}



static void Output(DbgMsgType type,
                   const char* module,
                   const char* msg,
                   void* context)
{
    const static int priorityMap[] = {
        LOG_ERR,        // Local error messages
        LOG_WARNING,    // Remote error messages
        LOG_NOTICE,     // High level debug messages
        LOG_INFO,       // Normal debug messages
        LOG_DEBUG,      // API trace messages
        LOG_DEBUG,      // Remote data messages
        LOG_DEBUG       // Local data messages
    };

    Log(priorityMap[type], "%s", msg);
}

LoggerSetting* LoggerSetting::singleton = NULL;


void LoggerSetting::SetSyslog(bool enable)
{
#if !defined(QCC_OS_GROUP_WINDOWS) && !defined(QCC_OS_GROUP_WINRT)
    lock.Lock();
#if !defined(QCC_OS_ANDROID)
    if (enable) {
        if (!useSyslog) {
            if (name) {
                openlog(name, 0, LOG_DAEMON);
            } else {
                enable = false;
            }
        }
    } else {
        if (useSyslog) {
            closelog();
        }
    }
#endif
    useSyslog = enable;
    lock.Unlock();
#endif
}


void LoggerSetting::SetFile(FILE* file)
{
    lock.Lock();
    if (UseStdio()) {
        fflush(this->file);
    }
    this->file = file;
    lock.Unlock();
}


void LoggerSetting::SetLevel(int level)
{
    lock.Lock();
    this->level = level;

#if !defined(QCC_OS_GROUP_WINDOWS) && !defined(QCC_OS_GROUP_WINRT) && !defined(QCC_OS_ANDROID)
    if (UseSyslog()) {
        setlogmask(LOG_UPTO(level));
    }

#endif

    lock.Unlock();
}


void LoggerSetting::SetName(const char* name)
{
    lock.Lock();
    this->name = name;
    lock.Unlock();
}


LoggerSetting::LoggerSetting() :
    name(NULL), level(LOG_WARNING), useSyslog(false), file(NULL)
{
    singleton = this;
}


LoggerSetting::LoggerSetting(const char* name, int level, bool useSyslog, FILE* file) :
    name(name), level(level), useSyslog(useSyslog), file(file)
{
#if !defined(QCC_OS_GROUP_WINDOWS) && !defined(QCC_OS_GROUP_WINRT) && !defined(QCC_OS_ANDROID)
    if (useSyslog) {
        openlog(name, 0, LOG_DAEMON);
    }
#endif
    singleton = this;
}


LoggerSetting::~LoggerSetting()
{
#if !defined(QCC_OS_GROUP_WINDOWS) && !defined(QCC_OS_GROUP_WINRT) && !defined(QCC_OS_ANDROID)
    if (useSyslog) {
        closelog();
    }
#endif
}


LoggerSetting* LoggerSetting::GetLoggerSetting(const char* name, int level,
                                               bool useSyslog, FILE* file) {
    if (!singleton) {
        singleton = new LoggerSetting(name, level, useSyslog, file);
    } else {
        singleton->lock.Lock();
        singleton->SetName(name);
        singleton->SetLevel(level);
        singleton->SetSyslog(useSyslog);
        singleton->SetFile(file);
        singleton->lock.Unlock();
    }
    return singleton;
}
