#pragma once
#include <vector>

using namespace std;


namespace ds
{
	////////////////////////////----UTILITY METHODS----/////////////////////////////////
	inline float getRand()
	{
		//return rand() % 3 - 1;
		return (((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f;

	}
	float average(float one, float two, float three, float four)
	{
		return (one + two + three + four) / 4;
	}
	float getGridValue(int x, int y, vector< vector < float > > &terrain)
	{
		//Catching out of bounds tests and wrapping them around to the other side of the map. 
		if (x < 0)
			x = terrain.size() - 1;
		if (x > terrain.size() - 1)
			x = 0;
		if (y < 0)
			y = terrain.size() - 1;
		if (y > terrain.size() - 1)
			y = 0;

		return terrain[x][y];
	}
	///////////////////////////////////////////////////////////////////////////////////
	
	/**Randomises the initial values of the corner vertices
	* \param terrain vector< vector < float > > A reference to the terrain container.
	*/
	void diamondSquareSetup(vector< vector < float > > &terrain)
	{
		int size = terrain.size() - 1;

		terrain[0][0] = getRand() * 100;
		terrain[0][size] = getRand() * 100;
		terrain[size][0] = getRand() * 100;
		terrain[size][size] = getRand() * 100;
	}
	
	/**Performes the diamond step.
	* \param terrain vector< vector < float > > A reference to the terrain container.
	* \param topLeftx int Top left vertex X coord of the quad which will be diamondised.
	* \param topLefty int Top left vertex Y coord of the quad which will be diamondised.
	* \param fullSize int The full size of the quads. 
	*/
	void diamond(vector< vector < float > > &terrain, int topLeftx, int topLefty, int fullSize, float range)
	{
		//Middle vertex
		terrain[topLeftx + fullSize / 2][topLefty + fullSize / 2] = average(
			terrain[topLeftx][topLefty],						//Top left vertex
			terrain[topLeftx + fullSize][topLefty + fullSize],	//Bottom right vertex
			terrain[topLeftx + fullSize][topLefty],				//Top right vertex
			terrain[topLeftx][topLefty + fullSize]				//Bottom left vertex
			) + getRand() * range;
	}

	void square(vector< vector < float > > &terrain, int middlex, int middley, int fullSize, float range)
	{
		//Middle vertex
		terrain[middlex][middley] = average(
			getGridValue(middlex + fullSize / 2, middley, terrain), //Left vertex
			getGridValue(middlex, middley + fullSize / 2, terrain), //Top vertex
			getGridValue(middlex - fullSize / 2, middley, terrain), //Right vertex
			getGridValue(middlex, middley - fullSize / 2, terrain)	//Bottom vertex
			) + getRand() * range;
	}

	void diamondSquare(vector< vector < float > > &terrain, int size, float range)
	{
		if (size == 1)
			return;

		//Diamond
		for (int i  = 0; i < (terrain.size() - 1); i += size)
		{
			for (int j = 0; j < (terrain.size() - 1); j += size)
			{
				diamond(terrain, i, j, size, range);
			}
		}

		//Square
		for (int i = 0; i < terrain.size() - 1; i += size)
		{
			for (int j = 0; j < terrain.size() - 1; j += size)
			{				
				ds::square(terrain, i + size / 2, j, size, range);
				ds::square(terrain, i, j + size / 2, size, range);
				ds::square(terrain, i + size / 2, j + size, size, range);
				ds::square(terrain, i + size, j + size / 2, size, range);
			}
		}
		diamondSquare(terrain, size / 2, range / 2.0f);
	}
}