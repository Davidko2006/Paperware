#pragma once
#define _CRT_SECURE_NO_WARNINGS

// Includes
#include "Utilities.h"
#include <fstream>
#include <Psapi.h>

bool FileLog = false;
std::ofstream logFile;

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

void Utilities::OpenConsole(std::string Title)
{
	AllocConsole();
	freopen("AVOZ", "r", stdin);
	freopen("AVOZ", "w", stdout);
	freopen("AVOZ", "w", stderr);

	SetConsoleTitle(Title.c_str());
}


void Utilities::dcewfhewiuofhuweifwwopop()
{
	float pJunkcode = 2459226616;
	pJunkcode = 2890321847;
	if (pJunkcode = 18112261)
		pJunkcode = 989630872;
	pJunkcode = 147674972;
	pJunkcode = 1078318807;
	if (pJunkcode = 50001462);
	pJunkcode = 920930968;
	pJunkcode = 2422547;
	if (pJunkcode = 186882255)
		pJunkcode = 230536916;
	pJunkcode = 179146074;
	pJunkcode = 563015959;
	if (pJunkcode = 891018452);
	pJunkcode = 2423619544;
	pJunkcode = 2480929864;
	if (pJunkcode = 1805625926)
		pJunkcode = 2275210179;
	pJunkcode = 129173844;
	pJunkcode = 69992189;
	if (pJunkcode = 2503811537);
	pJunkcode = 2281712952;
	pJunkcode = 1425113498;

}

void Utilities::CloseConsole()
{
	FreeConsole();
	dcewfhewiuofhuweifwwopop();
}

void Utilities::Log(const char *fmt, ...)
{
	dcewfhewiuofhuweifwwopop();
	if (!fmt) return; //if the passed string is null return
	if (strlen(fmt) < 2) return;

	//Set up va_list and buffer to hold the params 
	va_list va_alist;
	char logBuf[256] = { 0 };

	//Do sprintf with the parameters
	va_start(va_alist, fmt);
	_vsnprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);
	dcewfhewiuofhuweifwwopop();
	//Output to console
	if (logBuf[0] != '\0')
	{
		SetConsoleColor(FOREGROUND_INTENSE_GREEN);
		printf("[%s]", GetTimeString().c_str());
		SetConsoleColor(FOREGROUND_WHITE);
		printf(": %s\n", logBuf);
	}

	if (FileLog)
	{
		logFile << logBuf << std::endl;
	}
}

std::string Utilities::GetTimeString()
{
	dcewfhewiuofhuweifwwopop();
	//Time related variables
	time_t current_time;
	struct tm *time_info;
	static char timeString[10];
	dewuihfuewihfiuwsebasxweddwA();
	//Get current time
	time(&current_time);
	time_info = localtime(&current_time);
	dcewfhewiuofhuweifwwopop();
	//Get current time as string
	strftime(timeString, sizeof(timeString), "%I:%M%p", time_info);
	return timeString;
}

void Utilities::SetConsoleColor(WORD color)
{
	dcewfhewiuofhuweifwwopop();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void Utilities::EnableLogFile(std::string filename)
{
	logFile.open(filename.c_str());
	if (logFile.is_open())
		FileLog = true;
}

DWORD Utilities::Memory::WaitOnModuleHandle(std::string moduleName)
{
	DWORD ModuleHandle = NULL;
	while (!ModuleHandle)
	{
		ModuleHandle = (DWORD)GetModuleHandle(moduleName.c_str());
		if (!ModuleHandle)
			Sleep(50);
	}
	return ModuleHandle;
}

bool bCompare(const BYTE* Data, const BYTE* Mask, const char* szMask)
{
	for (; *szMask; ++szMask, ++Mask, ++Data)
	{
		if (*szMask == 'x' && *Mask != *Data)
		{
			return false;
		}
	}
	return (*szMask) == 0;
}

DWORD Utilities::Memory::FindPattern(std::string moduleName, BYTE* Mask, char* szMask)
{
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;
	for (DWORD c = 0; c < Length; c += 1)
	{
		if (bCompare((BYTE*)(Address + c), Mask, szMask))
		{
			return (DWORD)(Address + c);
		}
	}
	return 0;
}

DWORD Utilities::Memory::FindTextPattern(std::string moduleName, char* string)
{
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;

	int len = strlen(string);
	char* szMask = new char[len + 1];
	for (int i = 0; i < len; i++)
	{
		szMask[i] = 'x';
	}
	szMask[len] = '\0';

	for (DWORD c = 0; c < Length; c += 1)
	{
		if (bCompare((BYTE*)(Address + c), (BYTE*)string, szMask))
		{
			return (DWORD)(Address + c);
		}
	}
	return 0;
}

DWORD Utilities::Memory::FindPatternV2(std::string moduleName, std::string pattern)
{
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	MODULEINFO miModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;
		}
		else
		{
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}

bool	Utilities::Memory::VMTManager::Initialise(DWORD* InstancePointer)
{
	Instance = InstancePointer;
	OriginalTable = (DWORD*)*InstancePointer;
	int VMTSize = MethodCount(InstancePointer);
	size_t TableBytes = VMTSize * 4;

	CustomTable = (DWORD*)malloc(TableBytes + 8);
	if (!CustomTable) return false;
	memcpy((void*)CustomTable, (void*)OriginalTable, VMTSize * 4);

	*InstancePointer = (DWORD)CustomTable;

	initComplete = true;
	return true;
}

int		Utilities::Memory::VMTManager::MethodCount(DWORD* InstancePointer)
{
	DWORD *VMT = (DWORD*)*InstancePointer;
	int Index = 0;
	int Amount = 0;
	while (!IsBadCodePtr((FARPROC)VMT[Index]))
	{
		if (!IsBadCodePtr((FARPROC)VMT[Index]))
		{
			Amount++;
			Index++;
		}
	}

	return Amount;
}

DWORD	Utilities::Memory::VMTManager::HookMethod(DWORD NewFunction, int Index)
{
	if (initComplete)
	{
		CustomTable[Index] = NewFunction;
		return OriginalTable[Index];
	}
	else
		return NULL;
}

void	Utilities::Memory::VMTManager::UnhookMethod(int Index)
{
	if (initComplete)
		CustomTable[Index] = OriginalTable[Index];
	return;
}

void	Utilities::Memory::VMTManager::RestoreOriginal()
{
	if (initComplete)
	{
		*Instance = (DWORD)OriginalTable;
	}
	return;
}

void	Utilities::Memory::VMTManager::RestoreCustom()
{
	if (initComplete)
	{
		*Instance = (DWORD)CustomTable;
	}
	return;
}

DWORD	Utilities::Memory::VMTManager::GetOriginalFunction(int Index)
{
	return OriginalTable[Index];
}


void Utilities::dewuihfuewihfiuwsebasxweddwA()
{
	float pJunkcode = 1968620;
	pJunkcode = 232605693;
	if (pJunkcode = 1814331449)
		pJunkcode = 70124388;
	pJunkcode = 35894595;
	pJunkcode = 322656355;
	if (pJunkcode = 595811177);
	pJunkcode = 274830334;
	pJunkcode = 161783163;
	if (pJunkcode = 345829914)
		pJunkcode = 2057321341;
	pJunkcode = 2744123789;
	pJunkcode = 1869314854;
	if (pJunkcode = 70627138);
	pJunkcode = 920518691;
	pJunkcode = 2290017153;
	if (pJunkcode = 1040930705)
		pJunkcode = 506617375;
	pJunkcode = 171527436;
	pJunkcode = 110915570;
	if (pJunkcode = 1647817377);
	pJunkcode = 392627048;
	pJunkcode = 1038413931;

}