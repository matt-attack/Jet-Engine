#pragma once

#include <vector>

class HierarchicalGrid
{
	struct GridCell
	{
		std::vector<int> contents_;
	};
	struct GridLevel
	{
		std::vector<int> data_;// -1 = empty, otherwise the id of the internal data
		int size_;
		int resolution_;
		int dimension_;
		float inv_resolution_;
		int cell_size_;
	};
	std::vector<GridCell> cells_;
	std::vector<GridLevel> levels_;

	int o_x_, o_y_;
public:
	HierarchicalGrid(int size, int x_origin, int y_origin, int levels = 1, int grid_dimension = 128) :
		o_x_(x_origin),
		o_y_(y_origin)
	{
		int min = 1000 < grid_dimension*grid_dimension ? 1000 : grid_dimension * grid_dimension;
		cells_.reserve(min);
		levels_.resize(levels);
		for (int i = 0; i < levels; i++)
		{ 
			GridLevel& l = levels_[i];
			l.data_.resize(grid_dimension * grid_dimension, -1);
			l.cell_size_ = size;
			l.size_ = grid_dimension;
			l.dimension_ = size;
			l.resolution_ = size / grid_dimension;
		}
	}

	// rect
	// 0 minx 1 miny 2 maxx 3 maxy
	void Insert(int id, int rect[4])
	{
		rect[0] -= o_x_;
		rect[1] -= o_y_;
		rect[2] -= o_x_;
		rect[3] -= o_y_;

		// clamp it yo
		for (int i = 0; i < 4; i++)
		{
			if (rect[i] < 0)
				rect[i] = 0;
			else if (rect[i] >= levels_[0].dimension_)
				rect[i] = levels_[0].dimension_;
		}

		for (int x = rect[0]; x < rect[2]; x += levels_[0].resolution_)
		{
			for (int y = rect[1]; y < rect[3]; y += levels_[0].resolution_)
			{
				// add it to the cell
				CellInsert(CellIndex(x, y, levels_[0].size_, levels_[0].resolution_), id);
			}
		}
	}

	void Remove(int id, int rect[4])
	{
		rect[0] -= o_x_;
		rect[1] -= o_y_;
		rect[2] -= o_x_;
		rect[3] -= o_y_;

		// clamp it yo
		for (int i = 0; i < 4; i++)
		{
			if (rect[i] < 0)
				rect[i] = 0;
			else if (rect[i] >= levels_[0].dimension_)
				rect[i] = levels_[0].dimension_;
		}

		for (int x = rect[0]; x < rect[2]; x += levels_[0].resolution_)
		{
			for (int y = rect[1]; y < rect[3]; y += levels_[0].resolution_)
			{
				// add it to the cell
				CellRemove(CellIndex(x, y, levels_[0].size_, levels_[0].resolution_), id);
			}
		}
	}

	void Query(int rect[4], std::vector<int>& out)
	{
		rect[0] -= o_x_;
		rect[1] -= o_y_;
		rect[2] -= o_x_;
		rect[3] -= o_y_;

		// clamp it yo
		for (int i = 0; i < 4; i++)
		{
			if (rect[i] < 0)
				rect[i] = 0;
			else if (rect[i] >= levels_[0].dimension_)
				rect[i] = levels_[0].dimension_;
		}

		for (int x = rect[0]; x < rect[2]; x += levels_[0].resolution_)
		{
			for (int y = rect[1]; y < rect[3]; y += levels_[0].resolution_)
			{
				// add it to the cell
				int id = levels_[0].data_[CellIndex(x, y, levels_[0].size_, levels_[0].resolution_)];
				if (id >= 0)
				{
					for (size_t i = 0; i < cells_[id].contents_.size(); i++)
					{
						out.push_back(cells_[id].contents_[i]);
					}
				}
			}
		}
	}

private:
	
	inline
	int CellIndex(int x, int y, int dim, int res)
	{
		return x/res * dim + y/res;
	}

	inline
	void CellInsert(int cell, int id)
	{
		int index = levels_[0].data_[cell];
		if (levels_[0].data_[cell] >= 0)
		{
			//add it
			cells_[index].contents_.push_back(id);
		}
		else
		{
			// allocate
			cells_.resize(cells_.size() + 1);
			levels_[0].data_[cell] = cells_.size() - 1;
			cells_.back().contents_.push_back(id);
		}
	}

	inline
	void CellRemove(int cell, int id)
	{
		int index = levels_[0].data_[cell];
		int found = -1;
		for (size_t i = 0; i < cells_[index].contents_.size(); i++)
		{
			int val = cells_[index].contents_[i];
			if (val == id)
			{
				found = i;
				break;
			}
		}
		_ASSERT(found >= 0);
		cells_[index].contents_.erase(cells_[index].contents_.begin() + found);
	}
};