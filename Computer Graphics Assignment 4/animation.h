// Header file for handling model and animation functionality
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#ifndef ANIMATION_H
#define ANIMATION_H

#include <GL/glut.h>
#include "robot.h"

// Base class for handling static models which can be animated.
// Pos_ represents the center of the model
class StaticModel
{
public:
	virtual void draw() const;
	void setPos(Vec3 pos) { pos_ = pos; }
	void setRot(Vec3 rot) { rot_ = rot; }
	void setScale(Vec3 scale) { scale_ = scale; }

protected:
	virtual void setTransform() const final;

	Vec3 pos_;
	Vec3 rot_;
	Vec3 scale_;
};

//
// StaticModel implementations
//

class Tree : public StaticModel
{
public:
	void draw() const override
	{
		glPushMatrix();

		// Set up pos, rot, and scale
		setTransform();

		glPopMatrix();
	}
};

#endif // ANIMATION_H