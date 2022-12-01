// Implementations for handling model and animation functionality
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#include "animation.h"

//
// StaticModel
//

void StaticModel::setTransform() const
{
	glTranslatef(pos_.x, pos_.y, pos_.z);
	glRotatef(rot_.x, rot_.y, rot_.z);
	glScalef(scale_.x, scale_.y, scale_.z);
}