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

		size_t Len  = Dumper.GetName(Id, ObjectText, sizeof(ObjectText));

		printf("Object Name: %s\n", ObjectText);
		
		Dumper.DecryptBuffer(ObjectText, Len);
		printf("Decrypted object name: %s\n", ObjectText);
	}


	
	system("pause");
}