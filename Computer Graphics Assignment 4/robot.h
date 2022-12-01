// Declare robot class
// Computer Graphics: Assignment 2
// Colby Reinhart
// 9-27-2022

#ifndef ROBOT_HPP
#define ROBOT_HPP

#include <GL/glut.h>
#include <list>

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

// Hold mouse information
struct MouseInfo
{
	GLint leftButton, rightButton;
	int x, y;
};

// Hold camera information
struct CameraInfo
{
	float theta, phi, radius;
	float x, y, z;
};

// Represents and controls a robot
class Robot
{
public:

	// Define all moveable joints
	enum class Joints
	{
		LEFT_ELBOW = 0,
		LEFT_SHOULDER = 1,
		RIGHT_ELBOW = 2,
		RIGHT_SHOULDER = 3,
		LEFT_HIP = 4,
		LEFT_KNEE = 5,
		RIGHT_HIP = 6,
		RIGHT_KNEE = 7
	};

	static const int NUM_JOINTS = 8;

	// Constructors
	Robot() {}
	Robot(const struct Vec3& pos, const struct Vec3& rot, const int joints[NUM_JOINTS]);

	// Joint rotations
	void rotateJoint(const Joints joint, const float rot);
	int getJointRot(const Joints joint) const;
	void setJoints(const int jointPos[8]);

	// General transformations
	void translate(const Vec3& pos, bool delta = false) { pos_ = (delta) ? pos_ + pos : pos; }
	void rotate(const Vec3& rot, bool delta = false) { rot_ = (delta) ? rot_ + rot : rot; }
	void preTrans(const Vec3& pos, bool delta = false) { preTrans_ = (delta) ? preTrans_ + pos : pos; }

	// Set color
	void setColor(const float red, const float green, const float blue);

	// Toggle whether to display as wireframe
	void useWireframe(const bool use);

	// Display the robot
	void display() const;

private:
	float jointPos_[NUM_JOINTS] = { 0 }; // List of all point rotations
	float color_[3] = { 0, 0, 1 }; // Robot's color
	Vec3 pos_ = { 0 };
	Vec3 rot_ = { 0 };
	Vec3 preTrans_ = { 0 }; // Translation before rotating -- useful for rotating after translation
	bool wireframe_; // Are we drawing as wireframe?
};



// Controls robot animation
class RobotAnimator
{
public:
	// Defines an animation, including the delta
	// rotation of each joint, the delta movement
	// and rotation of the robot as a whole, and
	// the time it should take for this to happen
	struct Animation
	{
		float jointDeltas[Robot::NUM_JOINTS];
		Vec3 deltaPos;
		Vec3 deltaRot;
		Vec3 deltaPrePos;
		float deltaTime; // measured in frames
	};

	// Constructors
	RobotAnimator(Robot& robot, const bool loop = true);

	// Animation builders
	RobotAnimator& addAnim(const Animation anim);

	// Animation controllers
	void animate();
	void cancelAnimation();

	// Control animation state
	void saveState() { token_ = robot_; }
	void reset();

	static const int FRAME_DELAY = 1000 / 60; // in milliseconds

private:

	// Members to be initialized by constructor
	std::list<Animation> anims_;
	Robot& robot_;
	const bool loop_;

	// Members used by animate() to hold state
	std::list<Animation>::iterator currentAnim_;
	int animTimeLeft_ = 0;
	bool animStarted_ = false;
	Robot token_; // Used to save state
};

#endif // ROBOT_HPP