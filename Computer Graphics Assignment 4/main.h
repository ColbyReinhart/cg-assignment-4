// Main header file
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#ifndef MAIN_H
#define MAIN_H

#include <GL/glut.h>
#include <iostream>

struct TexRect;
struct CubeTexCoords;

void solidCube(const GLfloat size, const CubeTexCoords* coords = nullptr);
void solidSphere(const GLfloat size);

#endif // MAIN_H
