#include "ResourceManager.h"
#include "Defines.h"
#include "Graphics\CRenderer.h"

#include <string>
#include <new>

ResourceManager resources;

ResourceManager::ResourceManager(void)
{
	push();
}

ResourceManager::~ResourceManager(void)
{
	pop();
#ifdef _WIN32
	FindCloseChangeNotification(this->change_notifier);
#endif
}

void ResourceManager::init()
{
#ifdef _WIN32
	char dir[500];
	DWORD bytesreturned = 0;
	GetCurrentDirectoryA(sizeof(dir),dir);
	HANDLE directory = CreateFileA(dir, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_LIST_DIRECTORY | FILE_FLAG_OVERLAPPED, 0);
	if (directory == INVALID_HANDLE_VALUE)
	{
		//uh oh
	}
	memset(&change_overlapped, 0, sizeof(OVERLAPPED));
	ReadDirectoryChangesW(directory, file_change_buffer, sizeof(file_change_buffer), true, FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesreturned, &change_overlapped, 0);
#endif
}

#include "IMaterial.h"
void ResourceManager::update()
{
#ifdef _WIN32
	//poll filesystem for changes
	if (HasOverlappedIoCompleted(&change_overlapped))
	{
		FILE_NOTIFY_INFORMATION* data = (FILE_NOTIFY_INFORMATION*)file_change_buffer;

		char filename[500];
		char filename2[500];
		//well, we need normal char
		std::wcstombs(filename, data->FileName, 499);
		/*for (int i = 0; i < strlen(filename)+1; i++)
		{
			if (filename[i] == '\\')
				filename2[i] = '/';
			else
				filename2[i] = filename[i];
		}*/
		logf("[ResourceManager] File %s changed.\n", filename);

		std::string resname = filename;
		if (strncmp(filename, "Content\\", 8) == 0)
		{
			resname = &filename[8];
		}
		for (int i = 0; i < resname.length(); i++)
		{
			if (resname[i] == '\\')
				resname[i] = '/';
		}
		//ok, we need to know type from the filename
		std::map<std::string, Resource *>::iterator iter;
		for (int i = m_stack.size() - 1; i >= 0; i--) {
			iter = m_stack[i].find(resname);
			if (iter != m_stack[i].end()) {
				//T *ptr = dynamic_cast<T *>(iter->second);
				//assert(ptr);
				FILE* f = fopen(filename, "rb");
				if (f == 0)
					return;
				fclose(f);

				logf("[ResourceManager] Reloading file %s!\n", filename);

				log("it was a loaded resource\n");
				this->reload_lock.lock();
				iter->second->Reload(this, filename);
				iter->second->m_resmgr = this;
				this->reload_lock.unlock();
				break;
			}
			/*iter = m_stack[i].find(filename2);
			if (iter != m_stack[i].end()) {
				//T *ptr = dynamic_cast<T *>(iter->second);
				//assert(ptr);
				logf("[ResourceManager] Reloading file %s!\n", filename);

				log("it was a loaded resource\n");
				this->reload_lock.lock();
				iter->second->Reload(this, filename);
				iter->second->m_resmgr = this;
				this->reload_lock.unlock();
				break;
			}*/
		}

#ifndef MATT_SERVER
		//this shouldnt be here, change CTexture to be inherited from resource
		if (strcmp(filename, "texture.png") == 0)
		{
			//renderer->terrain_texture->Release();

			//lets reload the image
			/*auto h = D3DXCreateTextureFromFileExA(renderer->d3ddev,
			"texture.png",
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			4,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DX_FILTER_LINEAR,
			D3DX_FILTER_POINT,//point works better with alpha, but looks wierd
			D3DCOLOR_XRGB(255, 255, 255),//makes white clear
			NULL,
			NULL,
			&renderer->terrain_texture);*/
		}

		//load material textures here I guess
		for (auto ii: IMaterial::GetList())
		{
			ii.second->Update(renderer);
		}
#endif

		//I guess just clear overlapped buffer
		memset(&change_overlapped, 0, sizeof(OVERLAPPED));
		memset(&file_change_buffer, 0, sizeof(file_change_buffer));

		//lets go again
		char dir[500];
		DWORD bytesreturned = 0;
		GetCurrentDirectoryA(500,dir);
		HANDLE directory = CreateFileA(dir, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_LIST_DIRECTORY | FILE_FLAG_OVERLAPPED, 0);
		if (directory == INVALID_HANDLE_VALUE)
		{
			//uh oh
		}
		ReadDirectoryChangesW(directory, file_change_buffer, sizeof(file_change_buffer), true, FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesreturned, &change_overlapped, 0);
	}
#endif
}

void ResourceManager::push()
{
	log("[ResourceManager] Pushing!\n");
	m_stack.push_back(std::map<std::string, Resource *>());
}

void ResourceManager::pop()
{
	log("[ResourceManager] Popping!\n");
	std::map<std::string, Resource *> &v = m_stack[m_stack.size() - 1];
	std::map<std::string, Resource *>::reverse_iterator iter;
	for (iter = v.rbegin(); iter != v.rend(); iter++)//need to iterate through backwards
	{
		logf("[ResourceManager]   Unloaded %s\n", iter->first.c_str());

		delete iter->second;//fails on obj model
	}
	m_stack.pop_back();
}

size_t ResourceManager::stack_size() { return m_stack.size(); }
