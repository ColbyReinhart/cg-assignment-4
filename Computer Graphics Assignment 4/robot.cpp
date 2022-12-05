// Implement robot class
// Computer Graphics: Assignment 2
// Colby Reinhart
// 9-27-2022

#include <math.h>
#include <iostream>
#include "robot.h"
#include "main.h"

//
// VEC3 OVERLOADS
//

// Vec3 is a cheap copy, so we'll return it by value in overloads

Vec3 Vec3::operator+(const Vec3& rhs) const
{
	Vec3 result;
	result.x = x + rhs.x;
	result.y = y + rhs.y;
	result.z = z + rhs.z;
	return result;
}

Vec3 Vec3::operator-(const Vec3& rhs) const
{
	Vec3 result;
	result.x = x - rhs.x;
	result.y = y - rhs.y;
	result.z = z - rhs.z;
	return result;
}

Vec3 Vec3::operator*(const Vec3& rhs) const
{
	Vec3 result;
	result.x = x * rhs.x;
	result.y = y * rhs.y;
	result.z = z * rhs.z;
	return result;
}

Vec3 Vec3::operator/(const Vec3& rhs) const
{
	Vec3 result;
	result.x = x / rhs.x;
	result.y = y / rhs.y;
	result.z = z / rhs.z;
	return result;
}

Vec3 Vec3::operator*(const float scalar) const
{
	Vec3 result;
	result.x = x * scalar;
	result.y = y * scalar;
	result.z = z * scalar;
	return result;
}

Vec3 Vec3::operator/(const float scalar) const
{
	Vec3 result;
	result.x = x / scalar;
	result.y = y / scalar;
	result.z = z / scalar;
	return result;
}

//
// ROBOT IMPLEMENTATION
//

Robot::Robot(const struct Vec3& pos, const struct Vec3& rot, const int joints[NUM_JOINTS]) :
	pos_(pos), rot_(rot)
{
	for (int i = 0; i < NUM_JOINTS; ++i)
	{
		jointPos_[i] = joints[i];
	}
}

void Robot::rotateJoint(const Joints joint, const float rot)
{
	jointPos_[static_cast<int>(joint)] += rot;
}

int Robot::getJointRot(const Joints joint) const
{
	return jointPos_[static_cast<int>(joint)];
}

void Robot::setJoints(const int jointPos[8])
{
	for (int i = 0; i < 8; ++i)
	{
		jointPos_[i] = jointPos[i];
	}
}

void Robot::setColor(const float red, const float green, const float blue)
{
	color_[0] = red;
	color_[1] = green;
	color_[2] = blue;
}

void Robot::useWireframe(const bool use)
{
	wireframe_ = use;
}

void Robot::display() const
{
	// Init
	glPushMatrix();

	// Set up rotation and translation
	glTranslatef(0, 2.5f, 0);	// Move center to point of rotation
	glTranslatef(preTrans_.x, preTrans_.y, preTrans_.z);
	glRotatef(rot_.x, 1, 0, 0);	// Rotate
	glRotatef(rot_.y, 0, 1, 0);
	glRotatef(rot_.z, 0, 0, 1);
	glTranslatef(pos_.x, pos_.y - 2.5f, pos_.z); // Translate to actual position
	glColor3f(color_[0], color_[1], color_[2]);

	// Draw the head
	glPushMatrix();
	glTranslatef(0, 3.75, 0);
	wireframe_ ? glutWireSphere(0.5, 20, 20) : solidSphere(10);
	glPopMatrix();

	// Draw the body
	glPushMatrix();
	glTranslatef(0, 2.5, 0);
	glScalef(1, 1.5, 1);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();

	// Draw the left arm
	glPushMatrix();
	// Upper arm
	glTranslatef(0.35, 3.1, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::LEFT_SHOULDER), 1, 0, 0); // Rotate joint
	glTranslatef(0.35, -0.25, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Lower arm
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::LEFT_ELBOW), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.35, 0); // Move into position
	glPushMatrix();
	// Scale and draw
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Pop matrix
	glPopMatrix();

	// Draw the right arm
	glPushMatrix();
	// Upper arm
	glTranslatef(-0.35, 3.1, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::RIGHT_SHOULDER), 1, 0, 0); // Rotate joint
	glTranslatef(-0.35, -0.25, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Lower arm
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::RIGHT_ELBOW), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.35, 0); // Move into position
	glPushMatrix();
	// Scale and draw
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Pop matrix
	glPopMatrix();

	// Draw the left leg
	glPushMatrix();
	// Thigh
	glTranslatef(0.3, 1.75, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::LEFT_HIP), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.4, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Knee
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::LEFT_KNEE), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.35, 0); // Move into position
	glPushMatrix();
	// Scale and draw
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Pop matrix
	glPopMatrix();

	// Draw the right leg
	glPushMatrix();
	// Thigh
	glTranslatef(-0.3, 1.75, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::RIGHT_HIP), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.4, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Knee
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot(Joints::RIGHT_KNEE), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.35, 0); // Move into position
	glPushMatrix();
	// Scale and draw
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Pop matrix
	glPopMatrix();

	// Pop initial translation matrix
	glPopMatrix();
}

//
// ROBOTANIMATOR IMPLEMENTATION
//

RobotAnimator::RobotAnimator(Robot& robot, const bool loop) :
	robot_(robot), token_(robot), loop_(loop) {}

RobotAnimator& RobotAnimator::addAnim(const Animation anim)
{
	anims_.push_back(anim);
	return *this;
}

void RobotAnimator::animate()
{
	// Do some preflight stuff if we haven't animated yet
	if (!animStarted_)
	{
		// Make sure we have an animation to play
		if (anims_.empty())
		{
			return;
		}

		// Initialize iterator and animTimeLeft
		currentAnim_ = anims_.begin();
		animTimeLeft_ = currentAnim_->deltaTime;

		// Reset and start
		reset();
		animStarted_ = true;
	}

	// If there's no time left, start the next animation
	if (animTimeLeft_ <= 0)
	{
		++currentAnim_;

		if (currentAnim_ == anims_.end())
		{
			if (loop_)
			{
				reset();
			}
			else
			{
				cancelAnimation();
			}
		}

		animTimeLeft_ = currentAnim_->deltaTime;
	}

	// Do rotations
	for (int i = 0; i < Robot::NUM_JOINTS; ++i)
	{
		const float rotate = currentAnim_->jointDeltas[i] / currentAnim_->deltaTime;
		const Robot::Joints jointToRotate = static_cast<Robot::Joints>(i);
		robot_.rotateJoint(jointToRotate, rotate);
	}

	// Do robot transformations (delta)
	robot_.translate(currentAnim_->deltaPos / currentAnim_->deltaTime, true);
	robot_.rotate(currentAnim_->deltaRot / currentAnim_->deltaTime, true);
	robot_.preTrans(currentAnim_->deltaPrePos / currentAnim_->deltaTime, true);

	// Draw the robot and decrement time left
	robot_.display();
	--animTimeLeft_;
}

void RobotAnimator::cancelAnimation()
{
	animStarted_ = false;
	anims_.clear();
}

void RobotAnimator::reset()
{
	robot_ = token_;
	currentAnim_ = anims_.begin();
	animTimeLeft_ = currentAnim_->deltaTime;
}
