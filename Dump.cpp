#include "Dump.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <unordered_set>
Dump::Dump()
{
	Pid = 0;
	ProcessHandle = 0;
	ObjectFilePath = "DumpObjects.cpp";
	ClassFilePath = "Class.cpp";
	GameBaseAddress = 0;
	fopen_s(&File, ClassFilePath.c_str(), "w+");
}

Dump::~Dump()
{
	fclose(File);
}


int Dump::Init()
{
	//HMODULE  CeAssistantV2 = LoadLibraryA("CeAssistantV2.dll");

	//if (!CeAssistantV2)
	//	return 1;

	//Sleep(5000);

	HWND GamehWnd = FindWindowA("UnrealWindow", nullptr);
	//HWND GamehWnd = FindWindowA("Qt5QWindowIcon", nullptr);
	if (!GamehWnd)
		return 2;

	GetWindowThreadProcessId(GamehWnd, (DWORD*)(&Pid));

	if (!Pid)
		return 3;

	ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)Pid);

	if (!ProcessHandle)
		return 4;

	GameBaseAddress = GetModouleBaseAddress(Pid, "OAR-Win64-Shipping.exe");

	if (!GameBaseAddress)
		return 5;

	return 0;
}
VOID Dump::GetName(UINT32 Id, char* Buffer, size_t Size)
{
	UINT64 GName = GameBaseAddress + Offsets::GNames;

	UINT64 N = GName + 0x10 + ((Id & 0x1FFFFFFF) >> 0x10) * 8;

	UINT64 Offsets = (Id & 0xFFFF) * 2;

	UINT64 TempStringAddress = ReadBySystem<UINT64>(ProcessHandle, N);

	if(TempStringAddress == 0)
		return;

	UWORD TempLength = ReadBySystem<UWORD>(ProcessHandle, TempStringAddress + Offsets) >> 6;

	if(TempLength == 0)
		return;

	SIZE_T NumberOfRead = 0;

	ReadProcessMemory(ProcessHandle, (PVOID)(TempStringAddress + Offsets + 2), (PVOID)Buffer, TempLength, &NumberOfRead);

}
//VOID Dump::GetName(uint32 Id, char* Buffer, size_t Size)
//{
//	UINT64 GName = ReadBySystem<UINT64>(ProcessHandle, HpjyhdOffsets::GNames);
//
//	if (Id >= 0 && Id < 2000000)
//	{
//		UINT64 ChunkIndex = Id / 16384;
//		UINT64 WithinChunkIndex = Id % 16384;
//		UINT64 SegmentAddress = ReadBySystem<UINT64>(ProcessHandle, GName + ChunkIndex * 8);
//
//		if (SegmentAddress)
//		{
//			UINT64 SegmentNameAddress = ReadBySystem<UINT64>(ProcessHandle, SegmentAddress + WithinChunkIndex * 8);
//			if (SegmentNameAddress && Buffer)
//			{
//				SIZE_T NumberOfRead = 0;
//				ReadProcessMemory(ProcessHandle, (PVOID)(SegmentNameAddress + 12), (PVOID)Buffer, Size, &NumberOfRead);
//
//			}
//		}
//	}
//
//}


string Dump::DumpObject(UINT64 Object)
{
	if (Object == 0)
		return string();

	string Name = GetNameByObject(Object);
	if (Name == "" || Name == "None")
		return string();

	for (UINT64 OuterPrivate = GetOuterPrivate(Object); OuterPrivate; OuterPrivate = GetOuterPrivate(OuterPrivate))
	{
		Name = GetNameByObject(OuterPrivate) + "." + Name;
	}
	UINT64 ObjectType = GetObjectClass(Object);
	string ClassName = GetNameByObject(ObjectType);

	return ClassName + "\t " + Name;
}



UINT64 Dump::GetObjectByIndex(UINT32 Index)
{
	UINT64 PChunks = ReadBySystem<UINT64>(ProcessHandle, GameBaseAddress + Offsets::GObjects);
	UINT64 Chunks = ReadBySystem<UINT64>(ProcessHandle, PChunks);
	UINT64 Object = ReadBySystem<UINT64>(ProcessHandle, Chunks + Index * Offsets::UObject::Size);
	return Object;
}

UINT64 Dump::GetObjectById(UINT32 Id)
{
	UINT64 PGObject = ReadBySystem<UINT64>(ProcessHandle, GameBaseAddress + Offsets::GObjects);
	UINT64 Chunks = ReadBySystem<UINT64>(ProcessHandle, PGObject);
	UINT64 Index = 0;
	UINT64 Object = 0;
	BOOLEAN IsCycle = TRUE;
	UINT64 ObjectIsNullCount = 0;
	do
	{
		Object = ReadBySystem<UINT64>(ProcessHandle, Chunks + Index * Offsets::UObject::Size);
		UINT32 CurrentId = GetIdByObject(Object);
		if (CurrentId == Id)
		{
			return Object;
		}
		Index++;

		if (!Object)
			ObjectIsNullCount++;

		if (ObjectIsNullCount > 5)
			IsCycle = FALSE;

	} while (IsCycle);

	return 0;
}

string Dump::GetNameByObject(UINT64 Object)
{
	if (Object == 0)
		return string();

	UINT32 Id = ReadBySystem<UINT32>(ProcessHandle, Object + Offsets::UObject::Name);
	char ObjectText[0x500] = { 0 };
	GetName(Id, ObjectText, sizeof(ObjectText));
	string ObjectName(ObjectText);

	size_t pos = ObjectName.find_last_of('/');
	if (pos != string::npos)
	{
		ObjectName = ObjectName.substr(pos + 1); // Get the substring after the last '/'
	}

	if (ObjectName == "None")
	{
		ObjectName = "";
	}
	return ObjectName;
}

string Dump::GetNameByChildProperties(UINT64 Object)
{
	if (Object == 0)
		return string();

	UINT32 Id = ReadBySystem<UINT32>(ProcessHandle, Object + Offsets::UObject::FField::Name);
	char ObjectText[0x500] = { 0 };
	GetName(Id, ObjectText, sizeof(ObjectText));
	string ObjectName(ObjectText);

	size_t pos = ObjectName.find_last_of('/');
	if (pos != string::npos)
	{
		ObjectName = ObjectName.substr(pos + 1); // Get the substring after the last '/'
	}

	if (ObjectName == "None")
	{
		ObjectName = "";
	}
	return ObjectName;
}


UINT64 Dump::GetOuterPrivate(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT64 OuterPrivate = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::Outer);
	return OuterPrivate;
}

UINT32 Dump::GetIdByObject(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT32 Id = ReadBySystem<UINT32>(ProcessHandle, Object + Offsets::UObject::Name);
	return Id;
}

UINT64 Dump::GetModouleBaseAddress(HANDLE Pid, string ModuleName)
{
	UINT64 BaseAddr = 0;
	
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, (DWORD)Pid);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		printf("Error code: %d", GetLastError());
		return 0;
	}

	MODULEENTRY32 moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32); // Initialize structure size


	// Get the first module
	if (Module32First(hSnapshot, &moduleEntry))
	{
		do
		{
			// Case-insensitive string match of module name
			if (_stricmp(moduleEntry.szModule, ModuleName.c_str()) == 0)
			{
				// Found target module, get base address
				BaseAddr = (uintptr_t)moduleEntry.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnapshot, &moduleEntry)); // Continue traversing modules
	}

	CloseHandle(hSnapshot);
	return BaseAddr;
}

UINT64 Dump::GetObjectClass(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT64 ObjectClass = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::Class);
	return ObjectClass;
}

UINT64 Dump::GetObjectByName2(string Name)
{
	for (size_t i = 0; i < 2000000; i++)
	{
		UINT64 Object = GetObjectByIndex(i);
		if (Object == 0)
			continue;

		if (GetNameByObject(Object) == Name)
		{
			return Object;
		}

	}
	return 0;
}

UINT64 Dump::GetObjectByName(string Name)
{
	for (size_t i = 0; i < 2000000; i++)
	{
		UINT64 Object = GetObjectByIndex(i);
		if (Object == 0)
			continue;

		if (DumpObject(Object) == Name)
		{
			return Object;
		}

	}
	return 0;
}



BOOLEAN Dump::FileterObjects(UINT64 Object)
{
	if (Object == 0)
		return FALSE;

	// Found Class\'s base string, all Class objects inherit from CoreUObject.Class
	static UINT64 ObjectClassStr = GetObjectByName("Class	 CoreUObject.Class");

	for (UINT64 ObjectClass = GetObjectClass(Object);
		ObjectClass;
		ObjectClass = GetSuperStruct(ObjectClass))
	{
		if (ObjectClass == ObjectClassStr)
		{
			// Inherits from Class, indicating this is our target object
					return TRUE;
		}

	}

	return FALSE;
}

UINT64 Dump::GetSuperStruct(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT64 ParentClass = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::SuperStruct);
	return ParentClass;
}

UINT64 Dump::GetChildProperties(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT64 Properties = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::ChildProperties);
	return Properties;
}




UINT64 Dump::GetChildPropertiesEnum(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT64 PChildProperties = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::ChildProperties);
	UINT64 ChildPropertiesType = ReadBySystem<UINT64>(ProcessHandle, PChildProperties + Offsets::UObject::FField::EnumProperty);
	return ChildPropertiesType;
}
UINT64 Dump::GetChildPropertiesEnumByChildProperties(UINT64 Object)
{
	if (Object == 0)
		return 0;

	UINT64 ChildPropertiesType = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::FField::EnumProperty);
	return ChildPropertiesType;
}

UINT64 Dump::GetChildPropertiesCastFlagByChildProperties(UINT64 Object)
{
	if (Object == 0)
		return 0;

	UINT64 PPropertiesFlag = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::FField::Class);
	UINT64 PropertiesFlag = ReadBySystem<UINT64>(ProcessHandle, PPropertiesFlag + Offsets::UObject::FField::FFieldClass::CastFlag);

	return PropertiesFlag;
}

UINT64 Dump::GetNextChildProperties(UINT64 Object)
{
	if (Object == 0)
		return 0;
	UINT64 Next = ReadBySystem<UINT64>(ProcessHandle, Object + Offsets::UObject::FField::Next);
	return Next;
}

std::string GetValidClassName(const std::string& ClassName) {
	if (ClassName.empty() || ClassName == "None") return ClassName;
	if (ClassName.find("Actor") != std::string::npos ||
		ClassName.find("Pawn") != std::string::npos ||
		ClassName.find("Character") != std::string::npos ||
		ClassName.find("GameMode") != std::string::npos ||
		ClassName.find("GameState") != std::string::npos ||
		ClassName.find("PlayerController") != std::string::npos ||
		ClassName.find("PlayerState") != std::string::npos ||
		ClassName.find("Controller") != std::string::npos ||
		ClassName.find("HUD") != std::string::npos ||
		ClassName.find("Session") != std::string::npos ||
		ClassName.find("Weapon") != std::string::npos ||
		ClassName.find("Projectile") != std::string::npos) {

		if (ClassName.find("Component") == std::string::npos &&
			ClassName.find("Widget") == std::string::npos && 
			ClassName.find("Settings") == std::string::npos) {
			return "A" + ClassName;
		}
	}
	return "U" + ClassName;
}

// Core Upgrade 1: By passing the Property pointer (memory address of property object), we can read deep inner information.
// For example, the real type stored inside an Array, the bit-field mask of a Bool, etc., thereby parsing the complete generic nested type.
string Dump::GetEnumByCastFlag(UINT64 Property, UINT64 PropertiesObjectCastFlag)
{
	// Core Upgrade 2: Precisely map EClassCastFlags inside Unreal Engine to basic C++ data types.
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::Int8Property) return "int8";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::ByteProperty) return "uint8";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::Int16Property) return "int16";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::UInt16Property) return "uint16";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::IntProperty) return "int32";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::UInt32Property) return "uint32";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::Int64Property) return "int64";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::UInt64Property) return "uint64";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::FloatProperty) return "float";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::DoubleProperty) return "double";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::BoolProperty)
	{
		// Core Upgrade 6: Advanced Boolean Parsing. Read the FieldMask at offset 0x7B to check if it is a bit-field.
		// If FieldMask is not all FF or 0, it means it is a bitmask like uint8 bFlag : 1;, output uint8, otherwise output bool.
		uint8 FieldMask = ReadBySystem<uint8>(ProcessHandle, Property + Offsets::UObject::FField::BoolOrUint8Mask);
		if (FieldMask != 0xFF && FieldMask != 0)
			return "uint8";
		return "bool";
	}
	
	// Core Upgrade 3: Handle UE string types, automatically inject "class " prefix to keep output format consistent with Dumper-7.
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::NameProperty) return "class FName";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::StrProperty) return "class FString";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::TextProperty) return "class FText";

	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::EnumProperty) {
		UINT64 EnumObj = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty + 8);
		if (!EnumObj) EnumObj = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string EnumName = GetNameByObject(EnumObj);
		if (!EnumName.empty() && EnumName != "None") return "E" + EnumName;
		return "UEnum";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::StructProperty) {
		UINT64 InnerStruct = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string StructName = GetNameByObject(InnerStruct);
		if (!StructName.empty() && StructName != "None") return "struct F" + StructName;
		return "struct FStruct";
	}

	// Core Upgrade 5: Extract pointer types for objects and classes utilizing GetValidClassName.
	// Automatically add prefix \'A\' for Actors/Pawns/GameModes, and \'U\' for the rest.
	// Additionally, it can use TSubclassOf for ClassProperty generic wrapping, perfectly restoring the original class archetype.
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::ClassProperty) {
		UINT64 MetaClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty + 8);
		string ClassName = GetNameByObject(MetaClass);
		if (!ClassName.empty() && ClassName != "None") return "TSubclassOf<class " + GetValidClassName(ClassName) + ">";
		return "class UClass*";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::SoftClassProperty) {
		UINT64 MetaClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty + 8);
		string ClassName = GetNameByObject(MetaClass);
		if (!ClassName.empty() && ClassName != "None") return "TSoftClassPtr<class " + GetValidClassName(ClassName) + ">";
		return "TSoftClassPtr";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::ObjectProperty) {
		UINT64 InnerClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string ClassName = GetNameByObject(InnerClass);
		if (!ClassName.empty() && ClassName != "None") return "class " + GetValidClassName(ClassName) + "*";
		return "class UObject*";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::SoftObjectProperty) {
		UINT64 InnerClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string ClassName = GetNameByObject(InnerClass);
		if (!ClassName.empty() && ClassName != "None") return "TSoftObjectPtr<class " + GetValidClassName(ClassName) + ">";
		return "TSoftObjectPtr";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::WeakObjectProperty) {
		UINT64 InnerClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string ClassName = GetNameByObject(InnerClass);
		if (!ClassName.empty() && ClassName != "None") return "TWeakObjectPtr<class " + GetValidClassName(ClassName) + ">";
		return "TWeakObjectPtr";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::LazyObjectProperty) {
		UINT64 InnerClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string ClassName = GetNameByObject(InnerClass);
		if (!ClassName.empty() && ClassName != "None") return "TLazyObjectPtr<class " + GetValidClassName(ClassName) + ">";
		return "TLazyObjectPtr";
	}

	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::InterfaceProperty) {
		UINT64 InnerClass = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		string ClassName = GetNameByObject(InnerClass);
		if (!ClassName.empty() && ClassName != "None") return "TScriptInterface<I" + ClassName + ">";
		return "TScriptInterface";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::FieldPathProperty) return "TFieldPath";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::OptionalProperty) return "TOptional";

	// Core Upgrade 4: Generic container parsing block (TArray/TMap/TSet).
	// By reading Property+0x78 (EnumProperty offset), we can get the pointer to the inner property of the container.
	// Then recursively execute GetEnumByCastFlag to get the inner object type (InnerType) and assemble it into formats like TArray<T>.
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::ArrayProperty) {
		UINT64 InnerType = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		if (InnerType) {
			UINT64 InnerCastFlag = GetChildPropertiesCastFlagByChildProperties(InnerType);
			string InnerName = GetEnumByCastFlag(InnerType, InnerCastFlag);
			if (!InnerName.empty() && InnerName != "Unknown") return "TArray<" + InnerName + ">";
		}
		return "TArray";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::MapProperty) {
		UINT64 KeyProp = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		UINT64 ValueProp = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty + 8);
		if (KeyProp && ValueProp) {
			UINT64 KeyFlag = GetChildPropertiesCastFlagByChildProperties(KeyProp);
			UINT64 ValueFlag = GetChildPropertiesCastFlagByChildProperties(ValueProp);
			return "TMap<" + GetEnumByCastFlag(KeyProp, KeyFlag) + ", " + GetEnumByCastFlag(ValueProp, ValueFlag) + ">";
		}
		return "TMap";
	}
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::SetProperty) {
		UINT64 ElementProp = ReadBySystem<UINT64>(ProcessHandle, Property + Offsets::UObject::FField::EnumProperty);
		if (ElementProp) {
			UINT64 ElementFlag = GetChildPropertiesCastFlagByChildProperties(ElementProp);
			return "TSet<" + GetEnumByCastFlag(ElementProp, ElementFlag) + ">";
		}
		return "TSet";
	}

	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::DelegateProperty) return "FDelegate";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::MulticastDelegateProperty) return "FMulticastDelegate";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::MulticastInlineDelegateProperty) return "FMulticastInlineDelegate";
	if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::MulticastSparseDelegateProperty) return "FMulticastSparseDelegate";

	return "Unknown";
}



VOID Dump::DumpClass(UINT64 Object)
{

	if (Object == 0)
		return;
	unordered_set<UINT64> Visited;
	UINT32 ID = GetIdByObject(Object);

	if (!(ID & 0x1FFFFFFF))
	{
		return;
	}

	// Serialization structure output for DumpClass. It iterates through the ChildProperties linked list to get all members,
	// and calls GetEnumByCastFlag (the advanced deep probing logic above) to lay the foundation for a new generation of structured Dump.
	string OutputStr = "Object:[" + to_hex(Object) + "] Id:[" + to_hex(ID) + "]\n" + GetNameByObject(GetObjectClass(Object)) + " " + GetNameByObject(Object);
	UINT64 SuperStructObject = GetSuperStruct(Object);
	if (SuperStructObject)
	{
		OutputStr += " : public " + GetNameByObject(SuperStructObject);
	}
	OutputStr += "\n{\n";

	for (UINT64 ChildProperties = GetChildProperties(Object);
		ChildProperties != 0;
		ChildProperties = GetNextChildProperties(ChildProperties))
	{

		if (Visited.count(ChildProperties))
			break;

		Visited.insert(ChildProperties);

		if (!ReadBySystem<UINT64>(ProcessHandle, ChildProperties))
		{
			continue;
		}
		UINT64 TypeObject = GetChildPropertiesEnumByChildProperties(ChildProperties);
		UINT64 PropertiesObjectCastFlag = GetChildPropertiesCastFlagByChildProperties(ChildProperties);
		string PropertiesObjectEnumName = GetEnumByCastFlag(ChildProperties, PropertiesObjectCastFlag);

		string LineStr = "    " + PropertiesObjectEnumName;

		if (LineStr.size() < 70)
		{
			UINT64 Size = 70 - LineStr.size();
			for (size_t i = 0; i < Size; i++)
			{
				LineStr += " ";
			}
		}

		string PropName = GetNameByChildProperties(ChildProperties);
		if (PropertiesObjectCastFlag & (UINT64)EClassCastFlags::BoolProperty)
		{
			uint8 FieldMask = ReadBySystem<uint8>(ProcessHandle, ChildProperties + 0x7B);
			if (FieldMask != 0xFF && FieldMask != 0)
			{
				LineStr += PropName + " : 1;";
			}
			else
			{
				LineStr += PropName + ";";
			}
		}
		else
		{
			LineStr += PropName + ";";
		}

		if (LineStr.size() < 120)
		{
			UINT64 Size = 120 - LineStr.size();
			for (size_t i = 0; i < Size; i++)
			{
				LineStr += " ";
			}
		}

		UINT32 Size = ReadBySystem<UINT32>(ProcessHandle, ChildProperties + Offsets::UObject::FField::ElementSize);
		UINT32 Offset = ReadBySystem<UINT32>(ProcessHandle, ChildProperties + Offsets::UObject::FField::Offset_Internal);
		CHAR Buffer[0x500] = { 0 };
		//printf("%s\n", TypeName.c_str());
		sprintf_s(Buffer, "// 0x%.4X(0x%.4X)\n", Offset, Size);
		OutputStr += LineStr + Buffer;
	}
	OutputStr += "};\n";
	fprintf_s(File, "%s", OutputStr.c_str());

}



VOID Dump::BeginDump()
{
	size_t DumpCount = 0;
	for (size_t i = 0; i < 1000000; i++)
	{

		UINT64 Object = GetObjectByIndex(i);
		if (Object == 0)
			continue;

		printf("DumpCount:%d Index:%d\n", DumpCount, i);

		//		if (FileterObjects(Object))
		{
			DumpClass(Object);
			DumpCount++;

		}

	}
	fclose(File);
}
