
#ifndef RESOURCEMGR_HEADER
#define RESOURCEMGR_HEADER

#include "Defines.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <vector>
#include <map>
#include <string>
#include <mutex>

class CShader;
class ResourceManager;

//change this to use reference counting

class Resource {
	int count = 0;
public:
	Resource() { m_resmgr = 0; }
	virtual ~Resource() {}
	bool managed() const { return m_resmgr != 0; }
	ResourceManager *resmgr() { return m_resmgr; }
	const ResourceManager *resmgr() const { return m_resmgr; }

	void AddRef()
	{
		count++;
	}
	void Release()
	{
		count--;
	}
	virtual void Reload(ResourceManager* mgr, const std::string& filename) = 0;

private:
	friend class ResourceManager;
	ResourceManager *m_resmgr;
};

template <class T>
class ResHandle
{
public:
	T* ptr;

	ResHandle()
	{
		ptr = 0;
	}
	ResHandle(Resource* res)
	{
		ptr = res;
		res->AddRef();
	}

	ResHandle(ResHandle&& other)
	{
		//copy constructor just copy the handle over
		this->ptr = other.ptr;
		other.ptr = 0;
	}

	~ResHandle()
	{
		if (ptr)
			ptr->Release();
	}
};

class ResourceManager
{
#ifdef _WIN32
	HANDLE change_notifier;
	OVERLAPPED change_overlapped;
	char file_change_buffer[5000];
#endif
public:
	ResourceManager(void);
	~ResourceManager(void);

	void init();

	void release_unused();

	size_t stack_size();

	template <class T> ResHandle<T> get(const std::string &filename)//todo, optimize me by hashing then searching
	{
		resource_lock.lock();
		std::map<std::string, Resource *>::iterator iter;
		for (int i = m_stack.size() - 1; i >= 0; i--) {
			iter = m_stack[i].find(filename);
			if (iter != m_stack[i].end()) {
				T *ptr = dynamic_cast<T *>(iter->second);
				//assert(ptr);
				resource_lock.unlock();

				return ResHandle<T>(ptr);
			}
		}

		resource_lock.unlock();

		//allocate here
		//append the path to the content folder
		std::string path = "Content/" + filename;
		T* t = new T;
		T *rv = T::load_as_resource(path,t);
		rv->m_resmgr = this;

		resource_lock.lock();

		m_stack[m_stack.size() - 1][filename] = rv;

		resource_lock.unlock();

		logf("[ResourceManager] Loaded %s\n", filename.c_str());

		return ResHandle<T>(rv);
	}

	template <class T> T* get_unsafe(const std::string &filename)//todo, optimize me by hashing then searching
	{
		resource_lock.lock();
		std::map<std::string, Resource *>::iterator iter;
		//for (int i = m_stack.size() - 1; i >= 0; i--) {
			//change to not be stack based
			iter = m_stack.find(filename);
			if (iter != m_stack.end()) {
				T *ptr = dynamic_cast<T *>(iter->second);
				//assert(ptr);
				resource_lock.unlock();

				ptr->AddRef();
				return ptr;
			}
		//}

		resource_lock.unlock();

		//allocate here
		//append the path to the content folder
		std::string path = "Content/" + filename;
		T* t = new T;
		T *rv = T::load_as_resource(path, t);
		rv->m_resmgr = this;

		resource_lock.lock();

		m_stack[filename] = rv;

		resource_lock.unlock();

		logf("[ResourceManager] Loaded %s\n", filename.c_str());

		rv->AddRef();
		return rv;
	}

	CShader * get_shader(const std::string &filename);

	void update();

	std::mutex reload_lock;
private:
	std::mutex resource_lock;
	std::map<std::string, Resource *> m_stack;
};

extern ResourceManager resources;

#endif