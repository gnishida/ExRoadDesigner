/*********************************************************************
This file is part of QtUrban.

    QtUrban is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    QtUrban is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtUrban.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#pragma once

#include <vector>
#include "UrbanGeometry.h"
#include "MainWindow.h"

class Block;
class RoadGraph;

class RoadGraph;

class BlockGenerator {
protected:
	MainWindow* mainWin;

public:
	BlockGenerator(MainWindow* mainWin);
	~BlockGenerator();

	//bool generateBlocks(ucore::RoadGraph* roadGraph, std::vector<ucore::Block*>& blocks);

	void run();
	
private:
	bool removeIntersectingEdges(RoadGraph* roads);
	void buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding);
};
		
