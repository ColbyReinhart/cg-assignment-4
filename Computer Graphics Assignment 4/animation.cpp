// Implementations for handling model and animation functionality
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#include "animation.h"
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

void Tree::draw(GLuint* textureIDs) const
{
	// Start a new matrix
	glPushMatrix();

	// Set transform
	setTransform();

	// Draw the trunk
	if (textureIDs != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	}
	glPushMatrix();
	glTranslatef(0, 1.2f, 0);
	glScalef(2, 4.5f, 2);
	solidCube(1);
	glPopMatrix();

	// Draw the leaves
	if (textureIDs != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
	}
	glPushMatrix();
	glTranslatef(0, 4.5f, 0);
	glScalef(10.0f, 2.0f, 10.0f);
	solidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 6.5f, 0);
	glScalef(6.0f, 2.0f, 6.0f);
	solidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 8.5f, 0);
	glScalef(2.0f, 2.0f, 6.0f);
	solidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2.0f, 8.5f, 0);
	glScalef(2.0f, 2.0f, 2.0f);
	solidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2.0f, 8.5f, 0);
	glScalef(2.0f, 2.0f, 2.0f);
	solidCube(1);
	glPopMatrix();

	// Pop the used matrix
	glPopMatrix();
}

//
// DynamicModel
//

DynamicModel::DynamicModel(const struct Vec3& pos, const struct Vec3& rot, const struct Vec3& scale)
{
	pos_ = pos;
	rot_ = rot;
	scale_ = scale;
}

void DynamicModel::translate(const Vec3& pos, const bool delta)
{
	if (delta)
	{
		pos_ = pos_ + pos;
	}
	else
	{
		pos_ = pos;
	}
}

void DynamicModel::rotate(const Vec3& rot, const bool delta)
{
	if (delta)
	{
		rot_ = rot_ + rot;
	}
	else
	{
		rot_ = rot;
	}
}

void DynamicModel::scale(const Vec3& scale, const bool delta)
{
	if (delta)
	{
		scale_ = scale_ * scale;
	}
	else
	{
		scale_ = scale;
	}
}

//
// Robot : DynamicModel
//

Robot::Robot()
{
	// Fill the joints map
	joints_["left shoulder"] = 0;
	joints_["left elbow"] = 0;
	joints_["right shoulder"] = 0;
	joints_["right elbow"] = 0;
	joints_["left hip"] = 0;
	joints_["left knee"] = 0;
	joints_["right hip"] = 0;
	joints_["right knee"] = 0;
}

void Robot::draw() const
{
	// Init
	glPushMatrix();

	// Set up rotation and translation
	glTranslatef(0, 2.5f, 0);	// Move center to point of rotation
	glRotatef(rot_.x, 1, 0, 0);	// Rotate
	glRotatef(rot_.y, 0, 1, 0);
	glRotatef(rot_.z, 0, 0, 1);
	glTranslatef(pos_.x, pos_.y - 2.5f, pos_.z); // Translate to actual position
	glColor3f(1, 1, 1); // Color suitable for texturing

	// Draw the head
	glPushMatrix();
	glTranslatef(0, 3.65, 0);
	wireframe_ ? glutWireCube(0.8f) : solidCube(0.8f);
	glPopMatrix();

	// Draw the body
	glPushMatrix();
	glTranslatef(0, 2.5, 0);
	glScalef(1, 1.5, 0.6f);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();

	// Draw the left arm
	glPushMatrix();
	// Upper arm
	glTranslatef(0.35, 3.1, 0); // Move center to point of rotation
	glRotatef(getJointRot("left shoulder"), 1, 0, 0); // Rotate joint
	glTranslatef(0.35, -0.25, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Lower arm
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot("left elbow"), 1, 0, 0); // Rotate joint
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
	glRotatef(getJointRot("right shoulder"), 1, 0, 0); // Rotate joint
	glTranslatef(-0.35, -0.25, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Lower arm
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot("right elbow"), 1, 0, 0); // Rotate joint
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
	glRotatef(getJointRot("left hip"), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.4, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Knee
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot("left knee"), 1, 0, 0); // Rotate joint
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
	glRotatef(getJointRot("right hip"), 1, 0, 0); // Rotate joint
	glTranslatef(0, -0.4, 0); // Move into position
	// Scale and draw
	glPushMatrix();
	glScalef(0.4, 0.85, 0.4);
	wireframe_ ? glutWireCube(1) : solidCube(1);
	glPopMatrix();
	// Knee
	glTranslatef(0, -0.5, 0); // Move center to point of rotation
	glRotatef(getJointRot("right knee"), 1, 0, 0); // Rotate joint
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
// KeyFrameComponent Implementations
//

// Translation
Translation::Translation(const Vec3& translation, const bool delta)
	: KeyFrameComponent(delta), translation_(translation) {}

void Translation::apply(DynamicModel& model, const float timeDelta) const
{
	model.translate(translation_ / timeDelta, delta_);
}

// Rotation
Rotation::Rotation(const Vec3& rotation, const bool delta)
	: KeyFrameComponent(delta), rotation_(rotation) {}

void Rotation::apply(DynamicModel& model, const float timeDelta) const
{
	model.rotate(rotation_ / timeDelta, delta_);
}

// Scale
Scale::Scale(const Vec3& scale, const bool delta)
	: KeyFrameComponent(delta), scale_(scale) {}

void Scale::apply(DynamicModel& model, const float timeDelta) const
{
	model.scale(scale_ / timeDelta, delta_);
}

// JointRotation
JointRotation::JointRotation
(
	const std::string& name,
	const float value,
	const bool delta
) : KeyFrameComponent(delta), name_(name), value_(value) {}

void JointRotation::apply(DynamicModel& model, const float timeDelta) const
{
	model.rotateJoint(name_, value_ / timeDelta);
}

//
// KeyFrame
//

KeyFrame& KeyFrame::addComponent(KeyFrameComponent* component)
{
	components_.push_back(component);
	return *this;
}

void KeyFrame::initialize()
{
	timeLeft_ = timeDelta_;
}

void KeyFrame::apply(DynamicModel& model)
{
	// For each keyframe component, apply the movement
	// divided by the time delta.
	for (KeyFrameComponent* component : components_)
	{
		component->apply(model, timeDelta_);
	}

	// Decrement counter
	--timeLeft_;
}

//
// Animation
//

Animation& Animation::addKeyframe(KeyFrame* keyframe)
{
	keyframes_.push_back(keyframe);
	initialized_ = false;
	return *this;
}

void Animation::initialize()
{
	currentKeyframe_ = keyframes_.begin();
	initialized_ = true;
}

void Animation::animate()
{
	// Crash if not initialized
	if (!initialized_)
	{
		std::cerr << "ERROR: attempt to call Animation::animate() without"
			<< " initializing first!" << std::endl;
		exit(10);
	}

	if ((*currentKeyframe_)->finished())
	{
		++currentKeyframe_;

		if (currentKeyframe_ == keyframes_.end())
		{
			currentKeyframe_ = keyframes_.begin();
		}

		(*currentKeyframe_)->initialize();
	}

	(*currentKeyframe_)->apply(model_);
}

void Animation::reset()
{
	(*currentKeyframe_)->reset();
	currentKeyframe_ = keyframes_.begin();
}