/**
* Copyright 2016 IBM Corp. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <list>
#include <stdio.h>
#include <time.h>

#include "Log.h"

void ConsoleReactor::Process(const LogRecord & a_Record)
{
	if (a_Record.m_Level >= m_MinLevel)
	{
#ifdef _WIN32
		HANDLE h = NULL;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (a_Record.m_Level >= LL_ERROR)
		{
			h = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleScreenBufferInfo(h, &csbi);
			SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY );
		}
#endif
		printf("[%s][%s][%s] %s\n",
			a_Record.m_Time.c_str(),
			Log::LevelText(a_Record.m_Level),
			a_Record.m_SubSystem.c_str(),
			a_Record.m_Message.c_str());

#ifdef _WIN32
		if (h)
			SetConsoleTextAttribute(h, csbi.wAttributes );
#endif
	}
}

FileReactor::FileReactor(const char * a_pLogFile, LogLevel a_MinLevel /*= DEBUG*/) :
	m_LogFile(a_pLogFile),
	m_MinLevel(a_MinLevel)
{
	// truncate log file..
	FILE * f = fopen(m_LogFile.c_str(), "w");
	fclose(f);
}

void FileReactor::Process(const LogRecord & a_Record)
{
	if (a_Record.m_Level >= m_MinLevel)
	{
		FILE * f = fopen(m_LogFile.c_str(), "a+");
		if (f != NULL)
		{
			fprintf(f, "[%s][%s][%s] %s\n", a_Record.m_Time.c_str(), Log::LevelText( a_Record.m_Level ), a_Record.m_SubSystem.c_str(), a_Record.m_Message.c_str());
			fclose(f);
		}
	}
}

Log::ReactorList & Log::GetReactorList()
{
	static ReactorList reactors;
	return reactors;
}

tthread::recursive_mutex & Log::GetReactorLock()
{
	static tthread::recursive_mutex lock;
	return lock;
}

void Log::RegisterReactor(ILogReactor * a_pReactor)
{
	tthread::lock_guard<tthread::recursive_mutex> lock( GetReactorLock() );
	GetReactorList().push_back(a_pReactor);
}

void Log::RemoveReactor(ILogReactor * a_pReactor)
{
	tthread::lock_guard<tthread::recursive_mutex> lock( GetReactorLock() );
	GetReactorList().remove(a_pReactor);
}

void Log::DoLog(LogLevel a_Level, const char * a_pSub, const char * a_pFormat, va_list args )
{
	char buffer[1024 * 32];

	LogRecord rec;
	rec.m_Level = a_Level;
	rec.m_SubSystem = a_pSub;

	time_t now;
	time(&now);

	strftime(buffer, sizeof(buffer), "%x %X", localtime(&now));
	rec.m_Time = buffer;

	vsnprintf(buffer, sizeof(buffer) - 1, a_pFormat, args);
	rec.m_Message = buffer;

	ProcessRecord(rec);
}

void Log::ProcessRecord(const LogRecord & rec)
{
	tthread::lock_guard<tthread::recursive_mutex> lock( GetReactorLock() );
	for (ReactorList::iterator iReactor = GetReactorList().begin();
		iReactor != GetReactorList().end(); ++iReactor)
	{
		(*iReactor)->Process(rec);
	}
}

void Log::DebugLow(const char * a_pSub, const char * a_pFormat, ...)
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog(LL_DEBUG_LOW, a_pSub, a_pFormat, args);
	va_end(args);
}

void Log::DebugMed(const char * a_pSub, const char * a_pFormat, ...)
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog(LL_DEBUG_MED, a_pSub, a_pFormat, args);
	va_end(args);
}

void Log::DebugHigh(const char * a_pSub, const char * a_pFormat, ...)
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog(LL_DEBUG_HIGH, a_pSub, a_pFormat, args);
	va_end(args);
}

void Log::Debug( const char * a_pSub, const char * a_pFormat, ... )
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog( LL_DEBUG, a_pSub, a_pFormat, args );
	va_end(args);
}

void Log::Status( const char * a_pSub, const char * a_pFormat, ... )
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog( LL_STATUS, a_pSub, a_pFormat, args );
	va_end(args);
}

void Log::Warning( const char * a_pSub, const char * a_pFormat, ... )
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog( LL_WARNING, a_pSub, a_pFormat, args );
	va_end(args);
}

void Log::Error( const char * a_pSub, const char * a_pFormat, ... )
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog( LL_ERROR, a_pSub, a_pFormat, args );
	va_end(args);
}

void Log::Critical( const char * a_pSub, const char * a_pFormat, ... )
{
	va_list args;
	va_start(args, a_pFormat);
	DoLog( LL_CRITICAL, a_pSub, a_pFormat, args );
	va_end(args);
}

const char * Log::LevelText( LogLevel a_Level )
{
	static const char * LEVELS[] = 
	{
		"DEBL",
		"DEBM",
		"DEBH",
		"STAT",
		"WARN",
		"ERRO",
		"CRIT"
	};

	return LEVELS[ a_Level ];
}