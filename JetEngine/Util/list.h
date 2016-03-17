#ifndef LIST_H_
#define LIST_H_

#ifdef _WIN32
#include <windows.h>
#endif

template <class T>
class Node
{
public:
	// bunch of types
	typedef T value_type;
	typedef T& reference_type;
	typedef T const& const_reference_type;
	typedef T* pointer_type;
	typedef T const* const_pointer_type;

	// From now on, T should never appear
	//private:
	value_type m_value;
	Node* m_next;
};

template <class T>
class Iterator
{
public:
	typedef Node<T> node_type;

	node_type *ptr;

	Iterator()
	{
		ptr = 0;
	};

	Iterator(Node<T> *pos)
	{
		ptr = pos;
	};

	void prot_inc()
	{
		ptr = ptr->m_next;
	};

	void prot_dec()
	{

	};

	Iterator operator++(int)
	{
		prot_inc();
		return *this;
	};

	bool operator==(const Iterator& r)
	{
		return this->ptr == r.ptr;
	}

	bool operator!=(const Iterator& r)
	{
		return this->ptr != r.ptr;
	}

	T operator*()
	{
		if (this->ptr)
		{
			return this->ptr->m_value;
		}
	}
};


template <class T>
class List
{
	// private, no need to expose implementation
	typedef Node<T> node_type;

	// From now on, T should never appear
	typedef node_type* node_pointer;

	unsigned int _size;
public:
	typedef typename node_type::value_type value_type;
	typedef typename node_type::reference_type reference_type;
	typedef typename node_type::const_reference_type const_reference_type;


	typedef Iterator<T> iterator;
#ifdef _WIN32
	HANDLE mutex;
#endif
	//typedef typename
	// ...

	iterator begin()
	{
		return iterator(this->m_head);
	}

	iterator end()
	{
		return iterator(0);
	}

	List()
	{
#ifdef _WIN32
		mutex = CreateMutex( 
			NULL,              // default security attributes
			FALSE,             // initially not owned
			NULL);             // unnamed mutex
#endif

		_size = 0;
		m_head = m_tail = 0;
	}

	~List()
	{
		if (m_head != 0)
		{
			//delete all
			while (m_head != 0)
			{
				pop();
			}
		}
#ifdef _WIN32
		CloseHandle(mutex);
#endif
	}

	void clear()
	{
		while (m_head != 0)
		{
			pop();
		}
		_size = 0;
	}

	void lock()
	{
#ifdef _WIN32
		DWORD dwWaitResult = WaitForSingleObject( mutex,    // handle to mutex
			INFINITE);
#endif
	}

	void unlock()
	{
#ifdef _WIN32
		ReleaseMutex(mutex);
#endif
	}

	void pop()
	{
#ifdef _WIN32
		DWORD dwWaitResult = WaitForSingleObject( mutex,    // handle to mutex
			INFINITE);
#endif
		//pop off front
		if (this->m_head)
		{
			node_pointer temp = this->m_head;
			if (temp->m_next)
			{
				this->m_head = temp->m_next;
			}
			else
			{
				this->m_head = 0;
			}

			if (this->m_tail == temp)
			{
				this->m_tail = 0;
			}
			_size -= 1;
			delete temp;
		}
#ifdef _WIN32
		ReleaseMutex(mutex);
#endif
	}

	void pop_front()
	{
		pop();
	}

	void pop_back()
	{
#ifdef _WIN32
		DWORD dwWaitResult = WaitForSingleObject( mutex,    // handle to mutex
			INFINITE);
#endif
		if (this->m_tail)
		{
			if (this->m_head == this->m_tail)
			{
				this->m_head = 0;
			}
			delete this->m_tail;
			this->m_tail = 0;
			_size -= 1;
		}
#ifdef _WIN32
		ReleaseMutex(mutex);
#endif
	}

	void remove(value_type val)
	{
		if (this->m_head != 0)
		{
			node_pointer pos = this->m_head;
			node_pointer last = 0;
			while (pos)
			{
				if (pos->m_value == val)
				{
					if (last)
						last->m_next = pos->m_next;
					else
						this->m_head = pos->m_next;

					delete[] pos;
					break;
				}

				last = pos;
				pos = pos->m_next;
			}
		}
	}

	void push_front(value_type val)
	{
#ifdef _WIN32
		DWORD dwWaitResult = WaitForSingleObject( mutex,    // handle to mutex
			INFINITE);
#endif
		if (m_head == 0)
		{
			m_head = new node_type;
			m_head->m_value = val;//m_head->setData(info);
			m_head->m_next = 0;

			m_tail = m_head;
		}
		else
		{
			node_pointer temp = new node_type;
			temp->m_value = val;
			temp->m_next = m_head;

			m_head = temp;
		}
		_size += 1;
#ifdef _WIN32
		ReleaseMutex(mutex);
#endif
	}

	void push_back(value_type val)
	{
#ifdef _WIN32
		DWORD dwWaitResult = WaitForSingleObject( mutex,    // handle to mutex
			INFINITE);
#endif
		if(m_head == 0) //if our list is currently empty
		{
			m_head = new node_type;
			m_head->m_value = val;//m_head->setData(info);
			m_head->m_next = 0;

			m_tail = m_head;
		}
		else //if not empty add to the end and move the tail
		{
			node_pointer temp = new node_type;
			temp->m_value = val;//temp->setData(info);
			temp->m_next = 0;//temp->setNextNull();
			m_tail->m_next = temp;//m_tail->setNext(temp);
			m_tail = m_tail->m_next;//m_tail->getNext();
		}
		_size += 1;
#ifdef _WIN32
		ReleaseMutex(mutex);
#endif
	}

	void add(value_type info)
	{

	}

	value_type front()
	{
		return this->m_head->m_value;
	}

	value_type back()
	{
		return this->m_tail->m_value;
	}

	unsigned int size()
	{
		return this->_size;
	}

private:
	node_pointer m_head, m_tail;
};


#endif /* LIST_H_ */
