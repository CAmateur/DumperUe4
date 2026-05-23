#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "offsets.h"
using namespace std;
class Dump
{
public:
	Dump();
	~Dump();

	int Init();
	size_t GetName(UINT32 Id, char* Buffer, size_t Size);

	VOID BeginDump();

	VOID DumpClass(UINT64 Object);

	UINT64 GetObjectByIndex(UINT32 Index);

	UINT64 GetObjectById(UINT32 Id);


	string GetNameByObject(UINT64 Object);
	string GetNameByChildProperties(UINT64 Object);
	UINT32 GetIdByObject(UINT64 Object);

	UINT64 GetModouleBaseAddress(HANDLE Pid, string ModuleName);

	uint8_t DeriveKey(uint8_t g);
	void DecryptBuffer(char* buffer, int length);
	
    UINT64 GetObjectClass(UINT64 Object);

	// By doing this we can map out the full path of the object, like "Engine.World.PersistentLevel.MyActor".
	// Through this we can find the offsets related to World down to the target data members.
	UINT64 GetOuterPrivate(UINT64 Object);

	UINT64 GetSuperStruct(UINT64 Object);

	UINT64 GetChildProperties(UINT64 Object);

	UINT64 GetChildPropertiesEnumByChildProperties(UINT64 Object);
	UINT64 GetChildPropertiesCastFlagByChildProperties(UINT64 Object);
	UINT64 GetChildPropertiesEnum(UINT64 Object);
	UINT64 GetNextChildProperties(UINT64 Object);
	string GetEnumByCastFlag(UINT64 Property, UINT64 PropertiesObjectCastFlag);

	template <typename Type>
	Type ReadBySystem(HANDLE ProcessHandle, UINT64 ReadAddress)
	{
		Type Result{};
		RtlZeroMemory(&Result, sizeof(Type));

		SIZE_T NumberOfBytesRead = 0;
		ReadProcessMemory(ProcessHandle, (PVOID)ReadAddress, (PVOID)&Result, sizeof(Type), &NumberOfBytesRead);
		return Result;
	}


	template <typename T>
	std::string to_hex(T num) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::uppercase << num; // Uppercase + 0x prefix
		// For lowercase, remove std::uppercase: ss << "0x" << std::hex << num;
		return ss.str();
	}


private:
	HANDLE Pid;
	HANDLE ProcessHandle;
	UINT64 GameBaseAddress;
	FILE* File;
	string ObjectFilePath;
	string ClassFilePath;
};



