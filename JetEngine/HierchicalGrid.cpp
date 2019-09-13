#include "HierchicalGrid.h"

void HierarchicalGrid::Query(int rect[4], std::vector<int>& out)
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
			rect[i] = levels_[0].dimension_ - 1;
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