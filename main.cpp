#include <thread>
#include "Dump.h"

int main()
{
	Dump Dumper;

	int Result = Dumper.Init();

	if (Result)
		printf("Dumper Init Faild %d\n", Result);

	char ObjectText[0x200] = { 0 };
	UINT64 Id = 0x1520E;
	//UINT64 Id = 0x001E475;
	Dumper.GetName(Id, ObjectText, sizeof(ObjectText));
	printf("通过Id找到的对象名称：%s\n", ObjectText);

	UINT64 Object = Dumper.GetObjectById(Id);
	printf("通过Id找到的对象：%p\n", Object);

	string ObjectTypeStr = Dumper.GetNameByObject(Dumper.GetObjectClass(Object));
	printf("对象类型为：%s\n", ObjectTypeStr.c_str());

	string OuterPrivateObjectStr = Dumper.GetNameByObject(Dumper.GetOuterPrivate(Object));
	printf("通过OuterPrivate找到的对象：%s\n", OuterPrivateObjectStr.c_str());

	string SuperStructObjectStr = Dumper.GetNameByObject(Dumper.GetSuperStruct(Object));
	printf("通过SuperStruct找到的对象：%s\n", SuperStructObjectStr.c_str());	

	UINT64 ChildProperties = Dumper.GetChildProperties(Object);
	for (size_t i = 0; ChildProperties; i++)
	{
		string PropertiesObjectName = Dumper.GetNameByChildProperties(ChildProperties);
		UINT64 EnumObject = Dumper.GetChildPropertiesEnumByChildProperties(ChildProperties);
		string PropertiesObjectEnumStr = Dumper.GetNameByObject(Dumper.GetChildPropertiesEnumByChildProperties(ChildProperties));
		UINT64 PropertiesObjectCastFlag = Dumper.GetChildPropertiesCastFlagByChildProperties(ChildProperties);
		string PropertiesObjectEnumName = Dumper.GetEnumByCastFlag(ChildProperties, PropertiesObjectCastFlag);
		
		string LineStr = PropertiesObjectEnumName;

		if (LineStr.size() < 50)
		{
			UINT64 Size = 50 - LineStr.size();
			for (size_t i = 0; i < Size; i++)
			{
				LineStr += " ";
			}
		}

		LineStr += PropertiesObjectName;

		printf("%s\n", LineStr.c_str());

		ChildProperties = Dumper.GetNextChildProperties(ChildProperties);
	}



	system("pause");
	//printf("ActorTickFunction Object��%p\n", Dumper.GetObjectByName2("ActorTickFunction"));
	Dumper.BeginDump();
	system("pause");
}
