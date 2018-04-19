#include "Datatables.h"

std::map<int, networked_field*>* GetDataTable()
{
	static std::map<int, networked_field*> dt;
	return &dt;
}

int GetClassID(char* str)//basically a simple hash function
{
	int id = 0;
	const char* c = str;
	for (int i = 0; i < strlen(c); i++)
	{
		id += c[i];
		id *= c[i] - 284;
	}
	return id;
}


/*void model_change_callback(void* ent)
{
	CEntity* ent = (CEntity*)ent;
	printf("callback");
	curEnt->SetModel(curEnt->_model_s);
}*/
