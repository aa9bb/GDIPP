#pragma once

template <typename T>
class singleton
{
	singleton() {};                              // Private constructor
	singleton(const singleton&);                 // Prevent copy-construction
	singleton& operator=(const singleton&);      // Prevent assignment

public:
	static T &instance()
	{
		static T _instance;
		return _instance;
	}
};

class critical_section
{
	static CRITICAL_SECTION _cs;

public:
	critical_section()
	{
		EnterCriticalSection(&_cs);
	}

	~critical_section()
	{
		LeaveCriticalSection(&_cs);
	}

	static void initialize()
	{
		InitializeCriticalSection(&_cs);
	}

	static void release()
	{
		DeleteCriticalSection(&_cs);
	}
};

/*
GDIPP_SERVICE: gdimm.dll monitors the injector process, and unload itself once the injector process is terminated
GDIPP_LOADER: gdimm.dll does not care about the injector
*/
enum INJECTOR_TYPE
{
	GDIPP_SERVICE,
	GDIPP_LOADER
};

void get_dir_file_path(WCHAR source_path[MAX_PATH], const WCHAR *file_name, HMODULE h_module = NULL);

void debug_output_process_name();
void debug_output(const WCHAR *str = L"");
void debug_output(const WCHAR *str, unsigned int c);
void debug_output(const void *ptr, unsigned int size);
void debug_output(long num);