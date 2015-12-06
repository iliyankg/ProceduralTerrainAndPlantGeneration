#pragma once
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "Structs.h"
#include "Utils.h"

using namespace glm;
using namespace std;

const vec4 ZERO_ZERO = vec4(0.0, 0.0, 0.0, 1.0);
const mat4 IDENTITY_MAT = mat4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

class Tree
{
private:
	//////////////////------OWN MATRIX STACK------////////////////////
	std::vector <mat4> _fullMatrixStack;
	std::vector <mat4> matrixStack;

	void _pushMatrix()
	{
		if (matrixStack.empty())
			matrixStack.push_back(IDENTITY_MAT);
		else
			matrixStack.push_back(matrixStack.back());
	}
	void _popMatrix()
	{
		_fullMatrixStack.push_back(matrixStack.back());
		matrixStack.pop_back();
	}
	//////////////////////////////////////////////////////////////////
public:
	int branchLevels = 0;

	std::vector < std::vector < unsigned int > > indecies;
	std::vector <Vertex> treeVerts;
	std::vector <Vertex> leafVerts;

	Tree() {}
	Tree(int branches) : branchLevels(branches) {}
	~Tree() {}

	void makeTree(int level, int numBranchesPerLevel, double oldLen, int prevLevelStart)
	{
		if (level == branchLevels)
			return;

		if (level == 0)
		{
			_pushMatrix();
			matrixStack.back() = translate(matrixStack.back(), vec3(0.0, 0.0, 0.0));
			vec4 temp = vec4(matrixStack.back() * ZERO_ZERO);
			treeVerts.push_back({ { temp.x, temp.y, temp.z, temp.w },{},{},{ 0.55f, 0.27f, 0.075f, 1.0 } });

			_pushMatrix();
			matrixStack.back() = translate(matrixStack.back(), vec3(0.0, oldLen, 0.0));
			temp = vec4(matrixStack.back() * ZERO_ZERO);
			treeVerts.push_back({ { temp.x, temp.y, temp.z, temp.w },{},{}, { 0.55f, 0.27f, 0.075f, 1.0 } });

			_popMatrix();
			_popMatrix();

			std::vector < unsigned int > _temp;
			_temp.push_back(0);
			_temp.push_back(1);

			indecies.push_back(_temp);

			makeTree(level + 1, 2, oldLen * 0.8, 0);
		}
		else
		{

			for (int i = 0; i < numBranchesPerLevel / 2; ++i)
			{
				int fullIndex;
				if (prevLevelStart == 0)
					fullIndex = 0;
				else
					fullIndex = pow(2, (level - 1)) + i;

				_pushMatrix();
				matrixStack.back() =
					_fullMatrixStack[fullIndex] *
					rotate(matrixStack.back(), getMinusPlusRand() * glm::radians(180.0f), vec3(0.0, 1.0, 0.0)) *
					rotate(matrixStack.back(), glm::radians(getRand(45.0f)), vec3(0.0, 0.0, 1.0)) *
					translate(matrixStack.back(), vec3(0.0, -oldLen * 0.8, 0.0/*getMinusPlusRand() * 10.0f*/));

				vec4 temp = vec4(matrixStack.back() * ZERO_ZERO);
				treeVerts.push_back({ { temp.x, temp.y, temp.z, temp.w },{},{},{ 0.55f, 0.27f, 0.075f, 1.0 } });
				_popMatrix();

				std::vector < unsigned int > _temp;
				_temp.push_back(indecies[prevLevelStart + i][1]);
				_temp.push_back(treeVerts.size() - 1);
				indecies.push_back(_temp);

				_pushMatrix();
				matrixStack.back() =
					_fullMatrixStack[fullIndex] *
					rotate(matrixStack.back(), getMinusPlusRand() * glm::radians(90.0f), vec3(0.0, 1.0, 0.0)) *
					rotate(matrixStack.back(), glm::radians(-getRand(45.0f)), vec3(0.0, 0.0, 1.0)) *
					translate(matrixStack.back(), vec3(0.0, -oldLen * 0.8, 0.0/*getMinusPlusRand() * 10.0f*/));


				temp = vec4(matrixStack.back() * ZERO_ZERO);
				treeVerts.push_back({ { temp.x, temp.y, temp.z, temp.w },{},{},{ 0.55f, 0.27f, 0.075f, 1.0 } });
				_popMatrix();

				_temp.clear();
				_temp.push_back(indecies[prevLevelStart + i][1]);
				_temp.push_back(treeVerts.size() - 1);
				indecies.push_back(_temp);

			}

			if (prevLevelStart == 0)
				prevLevelStart += 1;
			else
				prevLevelStart = prevLevelStart * 2 + 1;

			makeTree(level + 1, numBranchesPerLevel * 2, -oldLen * 0.8, prevLevelStart);
		}
	}
	void makeLeaves(int numberOfLeaves)
	{
		vec3 leafColor = vec3(0.0f, 0.9f, 0.0f);

		vec4 leafLeft = vec4(-5.0f, 0.0f, 0.0f, 1.0f);
		vec4 leafRight = vec4(5.0f, 0.0f, 0.0f, 1.0f);
		vec4 leafTop = vec4(0.0f, 10.0f, 0.0f, 1.0f);

		for (int i = 0; i < numberOfLeaves; ++i)
		{
			int index = treeVerts.size() - 1 - i;

			vec4 tempLLeft = _fullMatrixStack[index] * scale(IDENTITY_MAT, vec3(0.8, 0.8, 0.8)) * leafLeft;
			vec4 tempLRight = _fullMatrixStack[index] * scale(IDENTITY_MAT, vec3(0.8, 0.8, 0.8)) * leafRight;
			vec4 tempLTop = _fullMatrixStack[index] * scale(IDENTITY_MAT, vec3(0.8, 0.8, 0.8)) * leafTop;

			leafVerts.push_back({ { tempLLeft.x, tempLLeft.y, tempLLeft.z, tempLLeft.w },{},{},{ leafColor.x, 0.4f, leafColor.z, 1.0 } });
			leafVerts.push_back({ { tempLRight.x, tempLRight.y, tempLRight.z, tempLRight.w },{},{},{ leafColor.x, 0.4f, leafColor.z, 1.0 } });
			leafVerts.push_back({ { tempLTop.x, tempLTop.y, tempLTop.z, tempLTop.w },{},{},{ leafColor.x, leafColor.y, leafColor.z, 1.0 } });
		}
	}
};