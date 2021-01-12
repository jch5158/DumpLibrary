#pragma once


class CCrashDump
{
public:
	CCrashDump(void)
	{
		//mDumpCount = 0;

		_invalid_parameter_handler oldHandler;
		_invalid_parameter_handler newHandler;

		newHandler = myInvalidParamterHandler;

		oldHandler = _set_invalid_parameter_handler(newHandler); // CRT �Լ��� null ������ ���� �־��� ��

		// ��Ÿ�� �� �ִ� �������� �ǵ����̸� �� ����.
		_CrtSetReportMode(_CRT_WARN, 0);						 // CRT ���� �޽��� ǥ�� �ߴ�, �ٷ� ���� ���⵵��
		_CrtSetReportMode(_CRT_ASSERT, 0);						 // CRT ���� �޽��� ǥ�� �ߴ�, �ٷ� ���� ���⵵��
		_CrtSetReportMode(_CRT_ERROR, 0);						 // CRT ���� �޽��� ǥ�� �ߴ�, �ٷ� ���� ���⵵��

		_CrtSetReportHook(_custom_Report_hook);


		// pure virtual function called ���� �ڵ鷯�� ���� ���� �Լ��� ��ȸ��Ų��.
		_set_purecall_handler(mPurecallHandler);

		SetHandlerDump();
	}

	static void Crash(void)
	{
		int* ptr = nullptr;
		*ptr = 0;
	}

	static LONG WINAPI MyExceptionFilter(PEXCEPTION_POINTERS pExceptionPointer)
	{
		int workingMemory = 0;

		SYSTEMTIME stNowTime;

		long DumptCount = InterlockedIncrement(&mDumpCount);

		
		// ���� ���μ����� �޸� ��뷮�� ���´�.
		HANDLE hProcess = NULL;
		PROCESS_MEMORY_COUNTERS pmc;

		hProcess = GetCurrentProcess();
		if (NULL == hProcess)
		{
			return 0;
		}

		// �޸� ���� ũ�� ���ϱ�
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			workingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
		}

		CloseHandle(hProcess);


		// ���� ��¥�� �ð��� �˾ƿ´�.
		WCHAR filename[MAX_PATH];

		GetLocalTime(&stNowTime);

		wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp",
			stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumptCount, workingMemory);

		wprintf_s(L"\n\n\n\n !!! Crash Error !!! %d%02d%02d_%02d.%02d.%02d",
			stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);

		wprintf_s(L"Now Save Dump File...\n");

		// ���� ����
		HANDLE hDumpFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation = { 0, };

			MinidumpExceptionInformation.ThreadId = GetCurrentThreadId();
			MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;

			// �ܺο��� �ش� ���μ����� ������ ���� �� ����ϴ� �÷����� �� ����.
			// �ٵ� true,false�� �ִ� �۵��� ��ȭ�� ����. 
			MinidumpExceptionInformation.ClientPointers = true;

			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hDumpFile,
				MiniDumpWithFullMemory,
				&MinidumpExceptionInformation,
				NULL,
				NULL
			);

			CloseHandle(hDumpFile);

			wprintf_s(L"CrashDump Save Finish ! \n");
		}

		return EXCEPTION_EXECUTE_HANDLER;
	}

	static void SetHandlerDump()
	{
		SetUnhandledExceptionFilter(MyExceptionFilter);
	}

	// Invalid Paramter handler
	static void myInvalidParamterHandler(const WCHAR* expression, const WCHAR* function, const WCHAR* file, unsigned int line, uintptr_t pReserved)
	{
		Crash();
	}

	static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue)
	{
		Crash();
		return true;
	}

	static void mPurecallHandler(void)
	{
		Crash();
	}

	static long mDumpCount;

};
