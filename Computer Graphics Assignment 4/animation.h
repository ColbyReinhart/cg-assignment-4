// Header file for handling model and animation functionality
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#ifndef ANIMATION_H
#define ANIMATION_H

#include <unordered_map>
#include <string>
#include "main.h"

// A vector with an x, y, and z component,
// with some simple arithmetic overloads
struct Vec3
{
	float x, y, z;

	// Vector arithmetic overloads
	Vec3 operator+(const Vec3& rhs) const;
	Vec3 operator-(const Vec3& rhs) const;
	Vec3 operator*(const Vec3& rhs) const;
	Vec3 operator/(const Vec3& rhs) const;

	// Scaler multiplication overloads
	Vec3 operator*(const float scalar) const;
	Vec3 operator/(const float scalar) const;
};

// Base class for handling static models which can be animated.
// Pos_ represents the center of the model
class StaticModel
{
public:
	StaticModel(){}
	StaticModel(const Vec3& pos) { pos_ = pos; }
	virtual void draw(GLuint* textureIDs = nullptr) const = 0;
	void setPos(Vec3 pos) { pos_ = pos; }
	void setRot(Vec3 rot) { rot_ = rot; }
	void setScale(Vec3 scale) { scale_ = scale; }

protected:
	virtual void setTransform() const final;

	Vec3 pos_ = { 0 };
	Vec3 rot_ = { 0 };
	Vec3 scale_ = { 1, 1, 1 };
};

//
// StaticModel implementations
//

class Tree : public StaticModel
{
public:
	Tree(){}
	Tree(const Vec3& pos): StaticModel(pos) {}
	void draw(GLuint* textureIDs = nullptr) const override;
};

// Base class for handling dynamic animated models which have moving joints
// Pos_ represents the center of the model
class DynamicModel
{
public:
	// Constructors
	DynamicModel() {}
	DynamicModel(const struct Vec3& pos, const struct Vec3& rot = { 0 }, const struct Vec3& scale = { 1, 1, 1 });
	
	// Joint rotations
	void rotateJoint(std::string joint, const int rot) { joints_[joint] = rot; }
	int getJointRot(std::string joint) const { return joints_.at(joint); }

	// Display
	virtual void draw() const = 0;
	void useWireframe(const bool use = true) { wireframe_ = use; }

protected:
	std::unordered_map<std::string, int> joints_;
	Vec3 pos_ = { 0 };
	Vec3 rot_ = { 0 };
	Vec3 scale_ = { 1, 1, 1 };
	bool wireframe_ = false;
};

//
// DynamicModel implementations
//

class Robot : public DynamicModel
{
	Robot();
	Robot
	(
		const struct Vec3& pos,
		const struct Vec3& rot = { 0 },
		const struct Vec3& scale = { 1, 1, 1 }
	) : Robot() { pos_ = pos; rot_ = rot; scale_ = scale; }

	virtual void draw() const override;
};

#endif // ANIMATION_H