#ifndef VECTOR_HEADER
#define VECTOR_HEADER

template<class T>
class Vector
{

public:
	Vector()
	{
		data = 0;
		used = 0;
		reallocate(10);
	}

	~Vector()
	{
		if (data)
			delete[] data;
	}

	void clear()
	{
		if (data)
			delete[] data;
		data = 0;
		used = 0;

		reallocate(10);
	}

	void erase(unsigned int position)
	{
		//todo
	}

	unsigned int begin()
	{
		return 0;
	}

	bool empty()
	{
		return used == 0;
	}

	unsigned int size()
	{
		return used;
	}

	void resize(unsigned int size, T value)
	{
		//todo
	}

	void reallocate(unsigned int new_size)
	{
		T* olddata = data;

		data = new T[new_size];
		allocated = new_size;

		//copy
		unsigned int end = used < new_size ? used : new_size;
		//construct new data
		for (unsigned int i = 0; i < end; i++)
		{
			data[i] = olddata[i];//new ((void*)data[i]) T(olddata[i]); //oh well, doesnt work
		}

		//destruct old data
		for (unsigned int j = 0; j < used; j++)
		{
			T* p = &data[j];
			p->~T();
		}

		if (allocated < used)
			used = allocated;

		//delete old data
		if (olddata)
			delete[] olddata;
	}

	T back()
	{
		return data[used-1];
	}

	T front()
	{
		return data[0];
	}

	void pop_back()
	{
		this->data[this->used-1] = 0;
		this->used--;
	}

	void push_back(const T& item)
	{
		if (used + 1 > allocated)//need to resize
		{
			//reallocate to fit more data
			unsigned int newalloc = allocated + 10;
			this->reallocate(newalloc);
		}
		this->data[this->used] = item;
		this->used++;
	}

	void insert(const T& item, unsigned int index)
	{
		if (used + 1 > allocated)//need to resize
		{
			//reallocate to fit more data
			unsigned int newalloc = allocated + 10;
			this->reallocate(newalloc);
		}
		else//no resize needed
		{
			if (used > index)//item not inserted at end
			{

			}
			else//element inserted at end
			{
				this->data[this->used] = item;
			}
		}
		used++;
	}

	T& operator [](unsigned int index)
	{
		return data[index];
	}

private:
	T* data;
	unsigned int allocated;
	unsigned int used;
};

#endif