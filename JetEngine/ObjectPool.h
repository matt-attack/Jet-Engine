#pragma once

#include <vector>

template <class T>
class ObjectPool
{
	int max_size_;
	int current_max_index_;

	T* pool_;
	std::vector<T*> free_positions_;
	//int* free_positions_;//todo implement with one of these
public:

	ObjectPool(unsigned int size) : max_size_(size), current_max_index_(0)
	{
		pool_ = new T[max_size_];
		free_positions_.reserve(std::max<int>(size/10, 100));// just in case
	}

	T* allocate_with_index(int& index)
	{
		if (free_positions_.size())
		{
			T* ptr = free_positions_.back();
			free_positions_.pop_back();
			index = ptr - pool_;
			return ptr;
		}

		// crash if we allocate too much
		_ASSERT(current_max_index_ + 1 < max_size_);

		index = current_max_index_;
		return &pool_[current_max_index_++];
	}

	T* allocate()
	{
		if (free_positions_.size())
		{
			T* ptr = free_positions_.back();
			free_positions_.pop_back();
			return ptr;
		}

		// crash if we allocate too much
		_ASSERT(current_max_index_ + 1 < max_size_);

		return &pool_[current_max_index_++];
	}

	int allocate_index()
	{
		if (free_positions_.size())
		{
			T* ptr = free_positions_.back();
			free_positions_.pop_back();
			return ptr - pool_;
		}

		// crash if we allocate too much
		_ASSERT(current_max_index_ + 1 < max_size_);

		return current_max_index_++;
	}



	void free(T* ptr)
	{
		int index = ptr - pool;
		if (index == current_max_index_)
		{
			current_max_index_--;
			return;
		}

		free_positions_.push_back(ptr);
	}

	inline
	T& operator [](int index)
	{
		return pool_[index];
	}
};