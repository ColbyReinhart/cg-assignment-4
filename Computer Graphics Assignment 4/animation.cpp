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
	glRotatef(rot_.x, 1, 0, 0);
	glRotatef(rot_.y, 0, 1, 0);
	glRotatef(rot_.z, 0, 0, 1);
	glScalef(scale_.x, scale_.y, scale_.z);
}

//
// Tree : StaticModel
//

void Tree::draw() const
{
	// Start a new matrix
	glPushMatrix();
	glColor3f(1, 1, 1);

	// Set transform
	setTransform();

	// Draw the trunk
	glPushMatrix();
	glTranslatef(0, 1.2f, 0);
	glScalef(2, 4.3f, 2);
	glutSolidCube(1);
	glPopMatrix();

	// Pop the used matrix
	glPopMatrix();
}