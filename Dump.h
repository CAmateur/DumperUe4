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
	VOID GetName(UINT32 Id, char* Buffer, size_t Size);

	VOID BeginDump();

	string DumpObject(UINT64 Object);

	VOID DumpClass(UINT64 Object);

	UINT64 GetObjectByIndex(UINT32 Index);

	UINT64 GetObjectById(UINT32 Id);

	UINT64 GetObjectByName(string Name);

	UINT64 GetObjectByName2(string Name);


	string GetNameByObject(UINT64 Object);
	string GetNameByChildProperties(UINT64 Object);
	UINT32 GetIdByObject(UINT64 Object);

	UINT64 GetModouleBaseAddress(HANDLE Pid, string ModuleName);


	//parmeter: Object
	
    UINT64 GetObjectClass(UINT64 Object);

	//�������������Ĵ�·���Ķ��������� "Engine.World.PersistentLevel.MyActor"��
	//ͨ������������ǿ���֪��world˳��ƫ���ҵ�������Ҫ�����ݳ�Ա
	UINT64 GetOuterPrivate(UINT64 Object);

	UINT64 GetSuperStruct(UINT64 Object);

	UINT64 GetChildProperties(UINT64 Object);

	UINT64 GetChildPropertiesEnumByChildProperties(UINT64 Object);
	UINT64 GetChildPropertiesCastFlagByChildProperties(UINT64 Object);
	UINT64 GetChildPropertiesEnum(UINT64 Object);
	BOOLEAN FileterObjects(UINT64 Object);
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

	// ����תʮ�������ַ�����ͨ�ð棩
	template <typename T>
	std::string to_hex(T num) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::uppercase << num; // ��д+0xǰ׺
		// Сд��ȥ�� std::uppercase �� ss << "0x" << std::hex << num;
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



