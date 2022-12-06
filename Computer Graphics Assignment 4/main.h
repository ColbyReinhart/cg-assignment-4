// Main header file
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#ifndef MAIN_H
#define MAIN_H

#include <GL/glut.h>
#include <iostream>

struct TexRect
{
	float bottomLeftX = 0.0f;
	float bottomLeftY = 0.0f;
	float topRightX = 1.0f;
	float topRightY = 1.0f;
};

struct CubeTexCoords
{
	TexRect faces[6];
};

void solidCube(const GLfloat size, const CubeTexCoords* coords = nullptr);
void solidSphere(const GLfloat size);

#endif // MAIN_H
