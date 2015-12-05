#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Tree
{
private:
	glm::vec4 _position;

public:
	std::vector< std::vector < glm::vec4 > > points;
};