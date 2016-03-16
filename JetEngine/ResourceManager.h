
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

class ResourceManager;

class Resource {
public:
	Resource() { m_resmgr = 0; }
	virtual ~Resource() {}
	bool managed() const { return m_resmgr != 0; }
	ResourceManager *resmgr() { return m_resmgr; }
	const ResourceManager *resmgr() const { return m_resmgr; }

	virtual void Reload(ResourceManager* mgr, const std::string& filename) = 0;

private:
	friend class ResourceManager;
	ResourceManager *m_resmgr;
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

	void push();
	void pop();

	size_t stack_size();

	template <class T> T *get(const std::string &filename)//todo, optimize me by hashing then searching
	{
		resource_lock.lock();
		std::map<std::string, Resource *>::iterator iter;
		for (int i = m_stack.size() - 1; i >= 0; i--) {
			iter = m_stack[i].find(filename);
			if (iter != m_stack[i].end()) {
				T *ptr = dynamic_cast<T *>(iter->second);
				//assert(ptr);
				resource_lock.unlock();
				return ptr;
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

		return rv;
	}

	void update();

	std::mutex reload_lock;
private:
	std::mutex resource_lock;
	std::vector<std::map<std::string, Resource *> > m_stack;
};

extern ResourceManager resources;

#endif