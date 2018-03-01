#include "Datatables.h"

std::map<int, netField_t*>* GetDataTable()
{
	static std::map<int, netField_t*> dt;
	return &dt;
}


/*void model_change_callback(void* ent)
{
	CEntity* ent = (CEntity*)ent;
	printf("callback");
	curEnt->SetModel(curEnt->_model_s);
}*/
