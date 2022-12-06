// Header file for handling model and animation functionality
// Computer Graphics Assignment 4
// By Colby Reinhart
// 12-1-2022

#ifndef ANIMATION_H
#define ANIMATION_H

#include <unordered_map>
#include <string>
#include <list>
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
	virtual DynamicModel* clone() = 0;

	// Transformation functions
	void translate(const Vec3& pos, const bool delta = true);
	void rotate(const Vec3& rot,const bool delta = true);
	void scale(const Vec3& scale, const bool delta = true);

	// Joint rotations
	void rotateJoint(std::string joint, const int rot) { joints_[joint] += rot; }
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
public:
	Robot();
	Robot
	(
		const struct Vec3& pos,
		const struct Vec3& rot = { 0 },
		const struct Vec3& scale = { 1, 1, 1 }
	) : Robot() { pos_ = pos; rot_ = rot; scale_ = scale; }

	virtual Robot* clone() override { return new Robot(*this); }

	virtual void draw() const override;
};

// A representation of a keyframe component. A list of 1 or more
// keyframe components constitute a keyframe, which defines one "step"
// of an animation.
class KeyFrameComponent
{
public:
	KeyFrameComponent(const bool delta = true) : delta_(delta) {}
	virtual void apply(DynamicModel& model, const float timeDelta) const = 0;

protected:
	bool delta_; // Should the component add to the transform or set it?
};

class Translation : public KeyFrameComponent
{
public:
	Translation(const Vec3& translation, const bool delta = true);
	virtual void apply(DynamicModel& model, const float timeDelta) const override;

protected:
	const Vec3 translation_;
};

class Rotation : public KeyFrameComponent
{
public:
	Rotation(const Vec3& rotation, const bool delta = true);
	virtual void apply(DynamicModel& model, const float timeDelta) const override;

protected:
	const Vec3 rotation_;
};

class Scale : public KeyFrameComponent
{
public:
	Scale(const Vec3& scale, const bool delta = true);
	virtual void apply(DynamicModel& model, const float timeDelta) const override;

protected:
	const Vec3 scale_;
};

class JointRotation : public KeyFrameComponent
{
public:
	JointRotation(const std::string& name, const float value, const bool delta = true);
	virtual void apply(DynamicModel& model, const float timeDelta) const override;

protected:
	const std::string& name_;
	const float value_;
};

// A representation of a keyframe, which is constituted of a list of keyframe
// components.
class KeyFrame
{
public:
	KeyFrame(const float timeDelta): timeDelta_(timeDelta) {}

	float getTimeDelta() const { return timeDelta_; }
	KeyFrame& addComponent(KeyFrameComponent* component);

	void initialize(); // Finished will return false once this is called
	void apply(DynamicModel& model);
	bool finished() { return timeLeft_ <= 0.0f; }
	void reset() { timeLeft_ = 0.0f; }

private:
	std::list<KeyFrameComponent*> components_;
	const float timeDelta_; // in frames
	float timeLeft_ = 0.0f;
};

// Class for handling animations on dynamic models. An animator
// takes a list of keyframes, which in turn takes a list of keyframe
// components.
class Animation
{
public:
	Animation(DynamicModel& model) : model_(model) {}

	Animation& addKeyframe(KeyFrame* keyframe);

	void initialize();
	void animate();
	void reset();

	static const int FRAME_DELAY = 1000 / 60; // in milliseconds

private:
	std::list<KeyFrame*> keyframes_;
	std::list<KeyFrame*>::iterator currentKeyframe_;
	DynamicModel& model_;
	bool initialized_ = false;
	DynamicModel* saveState_ = nullptr;
};

#endif // ANIMATION_H