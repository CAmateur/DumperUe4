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
		printf("Please enter the Id to query: ");
		scanf_s("%p", &Id);
		printf("Entered Id: %p\n", Id);

		Dumper.GetName(Id, ObjectText, sizeof(ObjectText));

		printf("Object Name: %s\n", ObjectText);

		UINT64 Object = Dumper.GetObjectById(Id);
		printf("Object Address: %p\n", Object);

	}


	
	system("pause");
}