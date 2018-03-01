#include "IModel.h"
#ifndef MATT_SERVER
#include "CIndexBuffer.h"
#include "CRenderer.h"
#include "Shader.h"
#endif

int GetModelID(const char* str)//basically a simple hash function
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

std::map<int, std::string>* GetModelList()
{
	static std::map<int, std::string> list;
	return &list;
}