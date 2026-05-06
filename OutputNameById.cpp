#include <thread>
#include "Dump.h"

int main()
{
	Dump Dumper;

	int Result = Dumper.Init();

	if (Result)
		printf("Dumper Init Faild %d\n", Result);

	
	
	while (true)
	{
		char ObjectText[0x200] = { 0 };
		UINT64 Id;
		printf("请输入要查询的Id：");
		scanf_s("%p", &Id);
		printf("输入的Id：%p\n", Id);

		Dumper.GetName(Id, ObjectText, sizeof(ObjectText));

		printf("对象名：%s\n", ObjectText);

		UINT64 Object = Dumper.GetObjectById(Id);
		printf("对象地址：%p\n", Object);

	}


	
	system("pause");
}