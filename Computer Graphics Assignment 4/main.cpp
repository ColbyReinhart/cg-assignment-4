#include "point.h"
#include "animation.h"
#include "main.h"

#include <math.h>
#include <stdlib.h>
#include <vector>
#include <FreeImage/FreeImage.h>

using namespace std;
void loadTextures();

// GLOBAL VARIABLES ////////////////////////////////////////////////////////////

static size_t windowWidth = 640;
static size_t windowHeight = 480;
static float aspectRatio;

GLint leftMouseButton, rightMouseButton;    // Status of the mouse buttons
int mouseX = 0, mouseY = 0;                 // Last known X and Y of the mouse
bool sphereOn = false;                      // Show the camera radius sphere

bool wireframe = false;                     // Display as a wireframe?
bool showAxes = true;                       // Display axes?
bool doRobotAnim = true;                    // Animate the robot?

// Camera
enum cameraList { CAMERA_INNER = 0, CAMERA_OUTER = 1 };
enum cameraList currentCamera = CAMERA_OUTER;

#define USING_INNER (currentCamera == CAMERA_INNER)

float M_PI = 3.141592;

Point outerCamTPR;
Point outerCamXYZ;

Point innerCamXYZ;
Point innerCamTPR;
Point innerCamDir;

// Robot
Robot robot;
Animation robotWalking(robot);

const std::string leftShoulder = "left shoulder";
const std::string leftElbow = "left elbow";
const std::string rightShoulder = "right shoulder";
const std::string rightElbow = "right elbow";
const std::string leftHip = "left hip";
const std::string leftKnee = "left knee";
const std::string rightHip = "right hip";
const std::string rightKnee = "right knee";

// Trees
vector<StaticModel*> trees;

// Textures
const int numTextures = 5;
char* textureNames[numTextures] =
{
    (char*)"textures/oak-log.png",
    (char*)"textures/oak-leaves.png",
    (char*)"textures/grass.png",
    (char*)"textures/metal.jpg",
    (char*)"textures/steve.jpg"
};
GLuint textureIDs[numTextures];

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based 
//  on its position in spherical coordinates. Should be called every time 
//  either camera's spherical coordinates are updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation(Point& xyz, Point& tpr)
{
    xyz.x = tpr.z * sinf(tpr.x) * sinf(tpr.y);
    xyz.z = tpr.z * -cosf(tpr.x) * sinf(tpr.y);
    xyz.y = tpr.z * -cosf(tpr.y);
    glutPostRedisplay();
}

// resizeWindow() //////////////////////////////////////////////////////////////
//
//  GLUT callback for window resizing. Resets GL_PROJECTION matrix and viewport.
//
////////////////////////////////////////////////////////////////////////////////
void resizeWindow(int w, int h)
{
    aspectRatio = w / (float)h;

    windowWidth = w;
    windowHeight = h;

    //update the viewport to fill the window
    glViewport(0, 0, w, h);

    //update the projection matrix with the new window properties
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspectRatio, 0.1, 100000);

    glutPostRedisplay();
}



// mouseCallback() /////////////////////////////////////////////////////////////
//
//  GLUT callback for mouse clicks. We save the state of the mouse button
//      when this is called so that we can check the status of the mouse
//      buttons inside the motion callback (whether they are up or down).
//
////////////////////////////////////////////////////////////////////////////////
void mouseCallback(int button, int state, int thisX, int thisY)
{
    //update the left and right mouse button states, if applicable
    if (button == GLUT_LEFT_BUTTON)
        leftMouseButton = state;
    else if (button == GLUT_RIGHT_BUTTON)
        rightMouseButton = state;

    //and update the last seen X and Y coordinates of the mouse
    mouseX = thisX;
    mouseY = thisY;
}

// mouseMotion() ///////////////////////////////////////////////////////////////
//
//  GLUT callback for mouse movement. We update the current camera's spherical
//      coordinates based on how much the user has moved the mouse in the
//      X or Y directions (in screen space) and whether they have held down
//      the left or right mouse buttons. If the user hasn't held down any
//      buttons, the function just updates the last seen mouse X and Y coords.
//
////////////////////////////////////////////////////////////////////////////////
void mouseMotion(int x, int y)
{
    if (leftMouseButton == GLUT_DOWN)
    {
        Point* curTPR = (USING_INNER ? &innerCamTPR : &outerCamTPR);      //just for conciseness below
        curTPR->x += (x - mouseX) * 0.005;
        curTPR->y += (USING_INNER ? -1 : 1) * (y - mouseY) * 0.005;

        // make sure that phi stays within the range (0, M_PI)
        if (curTPR->y <= 0)
            curTPR->y = 0 + 0.001;
        if (curTPR->y >= M_PI)
            curTPR->y = M_PI - 0.001;

        //update camera (x,y,z) based on (radius,theta,phi)
        if (USING_INNER)
        {
            recomputeOrientation(innerCamDir, innerCamTPR);
            innerCamDir.normalize();
        }
        else {
            recomputeOrientation(outerCamXYZ, outerCamTPR);
        }
    }
    else if (rightMouseButton == GLUT_DOWN && !USING_INNER) {
        double totalChangeSq = (x - mouseX) + (y - mouseY);

        Point* curTPR = &outerCamTPR;      //just for conciseness below
        curTPR->z += totalChangeSq * 0.01;

        //limit the camera radius to some reasonable values so the user can't get lost
        if (curTPR->z < 2.0)
            curTPR->z = 2.0;
        if (curTPR->z > 10.0 * (currentCamera + 1))
            curTPR->z = 10.0 * (currentCamera + 1);

        //update camera (x,y,z) based on (radius,theta,phi)
        recomputeOrientation(outerCamXYZ, outerCamTPR);
    }

    mouseX = x;
    mouseY = y;
}



// initScene() /////////////////////////////////////////////////////////////////
//
//  A basic scene initialization function; should be called once after the
//      OpenGL context has been created. Doesn't need to be called further.
//
////////////////////////////////////////////////////////////////////////////////
void initScene()
{
    glEnable(GL_DEPTH_TEST);

    float lightCol[4] = { 1, 1, 1, 1 };
    float ambientCol[4] = { 0.3, 0.3, 0.3, 1.0 };
    float lPosition[4] = { 10, 10, 10, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientCol);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_TEXTURE); // Matric mode for manipulating the texture transform matrix
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    loadTextures();

    glutPostRedisplay();
}


// drawSceneElements() /////////////////////////////////////////////////////////
//
//  Because we'll be drawing the scene twice from different viewpoints,
//      we encapsulate the code to draw the scene here, so that we can just
//      call this function twice once the projection and modelview matrices
//      have been set appropriately.
//
////////////////////////////////////////////////////////////////////////////////
void drawSceneElements(void)
{
    glDisable(GL_LIGHTING);

    // Draw axes
    if (showAxes)
    {
        glBegin(GL_LINES);
        // x
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0); glVertex3f(3, 0, 0);
        // y
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0); glVertex3f(0, 3, 0);
        // z
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0); glVertex3f(0, 0, 3);
        glEnd();
    }

    // Wireframes should be white and don't need lighting or textures
    glColor3f(1, 1, 1);
    if (!wireframe)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }

    // Draw the ground
    const int groundSize = 10;
    const int groundHeight = -1;

    if (wireframe)
    {
        //draw a simple grid under the teapot
        glColor3f(1, 1, 1);
        for (int dir = 0; dir < 2; dir++)
        {
            for (int i = -groundSize; i <= groundSize; i++)
            {
                glBegin(GL_LINE_STRIP);
                for (int j = -groundSize; j <= groundSize; j++)
                    glVertex3f(dir < 1 ? i : j, groundHeight, dir < 1 ? j : i);
                glEnd();
            }
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, textureIDs[2]); // ground.png
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(groundSize, groundHeight, groundSize);
        glTexCoord2f(0.0, groundSize);
        glVertex3f(groundSize, groundHeight, -groundSize);
        glTexCoord2f(groundSize, groundSize);
        glVertex3f(-groundSize, groundHeight, -groundSize);
        glTexCoord2f(groundSize, 0.0);
        glVertex3f(-groundSize, groundHeight, groundSize);
        glEnd();
    }


    // Draw the robot
    glBindTexture(GL_TEXTURE_2D, textureIDs[4]); // steve.jpg
    robot.useWireframe(wireframe);
    robot.draw();

    // Draw trees
    for (StaticModel* tree : trees)
    {
        tree->useWireframe(wireframe);
        tree->draw(textureIDs);
    }

    glDisable(GL_TEXTURE_2D);
}


// drawInnerCamera() ///////////////////////////////////////////////////////////
//
//  Draws a representation of the inner camera in space. This should only be
//      called when rendering the scene from the POV of the outer camera,
//      so that we can visualize where the inner camera is positioned
//      and what it is looking at.
//
////////////////////////////////////////////////////////////////////////////////
void drawInnerCamera()
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(innerCamXYZ.x, innerCamXYZ.y, innerCamXYZ.z);
    glRotatef(-innerCamTPR.x * 180.0 / M_PI, 0, 1, 0);
    glRotatef(innerCamTPR.y * 180.0 / M_PI, 1, 0, 0);
    glColor3f(0, 1, 0);

    // Camera box
    glPushMatrix();
    glScalef(2, 0.5, 1.5);
    glutWireCube(1.0f);
    glPopMatrix();

    // Camera flash
    glPushMatrix();
    glTranslatef(-0.5f, 0, -0.85f);
    glScalef(2, 1, 1);
    glutWireCube(0.2f);
    glPopMatrix();
    
    // Camera lens
    const float lensRadius = 0.5f;
    glBegin(GL_LINE_LOOP);
    for (float i = 0; i < 2 * M_PI; i += 0.1f)
    {
        glVertex3f(lensRadius * cos(i), -0.25f, lensRadius * sin(i));
    }
    glEnd();

    glPopMatrix();
    glPopAttrib();
}

// renderCallback() ////////////////////////////////////////////////////////////
//
//  GLUT callback for scene rendering. Sets up the modelview matrix, renders
//      a teapot to the back buffer, and switches the back buffer with the
//      front buffer (what the user sees).
//
////////////////////////////////////////////////////////////////////////////////
void renderCallback(void)
{
    //clear the render buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    float borderWidth = 3;
    //start with the code from the outer camera, which covers the whole screen!

    // Viewport
    glViewport(0, 0, windowWidth, windowHeight);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    // Model view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // The outer camera has a red border while the inner camera has a white border
    if (currentCamera == CAMERA_OUTER)
        glColor3f(1, 0, 0);
    else
        glColor3f(1, 1, 1);

    // Draw the border as a square covering the screen. We'll fill the screen later
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(0, 1); glVertex2f(1, 1); glVertex2f(1, 0);
    glEnd();

    // Set the viewport dimensions
    glViewport(borderWidth, borderWidth, windowWidth - borderWidth * 2, windowHeight - borderWidth * 2);

    // Draw a black screen over the border square
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(0, 1); glVertex2f(1, 1); glVertex2f(1, 0);
    glEnd();

    // Set up lighting and depth
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    //update the modelview matrix based on the camera's position
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(outerCamXYZ.x, outerCamXYZ.y, outerCamXYZ.z,
        0, 0, 0,
        0, 1, 0);

    drawSceneElements();
    drawInnerCamera();

    // Set up the inner camera
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    //step 1: set the projection matrix using gluOrtho2D -- but save it first!
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    //step 2: clear the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //step 3: set the viewport matrix a little larger than needed...
    glViewport(2 * windowWidth / 3.0 - borderWidth, 2 * windowHeight / 3.0 - borderWidth,
        windowWidth / 3.0 + borderWidth, windowHeight / 3.0 + borderWidth);

    //step 3a: and fill it with a white rectangle!
    if (currentCamera == CAMERA_OUTER)
        glColor3f(1, 1, 1);
    else
        glColor3f(1, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(0, 1); glVertex2f(1, 1); glVertex2f(1, 0);
    glEnd();

    //step 4: trim the viewport window to the size we want it...
    glViewport(2 * windowWidth / 3.0, 2 * windowHeight / 3.0,
        windowWidth / 3.0, windowHeight / 3.0);

    //step 4a: and color it black! the padding we gave it before is now a border.
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(0, 1); glVertex2f(1, 1); glVertex2f(1, 0);
    glEnd();

    //before rendering the scene in the corner, pop the old projection matrix back
    //and re-enable lighting!
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // Begin drawing scene in upper corner

    // Set the viewport matrix
    glViewport(2 * windowWidth / 3.0, 2 * windowHeight / 3.0,
        windowWidth / 3.0, windowHeight / 3.0);

    // Set up the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(innerCamXYZ.x, innerCamXYZ.y, innerCamXYZ.z,      //camera is located at (x,y,z)
        innerCamXYZ.x + innerCamDir.x,                  //looking at a point that is
        innerCamXYZ.y + innerCamDir.y,                  //one unit along its direction
        innerCamXYZ.z + innerCamDir.z,
        0.0f, 1.0f, 0.0f);                                //up vector is (0,1,0) (positive Y)

    glClear(GL_DEPTH_BUFFER_BIT);                   //ensure that the overlay is always on top!


    drawSceneElements();

    //push the back buffer to the screen
    glutSwapBuffers();
}

void doAnimation(int v)
{
    if (doRobotAnim)
    {
        robotWalking.animate();
    }
    glutPostRedisplay();
    glutTimerFunc(Animation::FRAME_DELAY, doAnimation, v);
}

void loadTextures()
{
    glGenTextures(numTextures, textureIDs); // Get the texture object IDs

    // Load all textures
    for (int i = 0; i < numTextures; ++i)
    {
        // Check image format
        FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(textureNames[i]);
        if (format == FIF_UNKNOWN)
        {
            std::cerr << "Unknown file format for " << textureNames[i] << std::endl;
            exit(1);
        }

        // Get image bitmap
        FIBITMAP* bitmap = FreeImage_Load(format, textureNames[i], 0);
        if (bitmap == nullptr)
        {
            std::cerr << "Failed to load image " << textureNames[i] << std::endl;
            exit(2);
        }

        // Convert to BGR format
        {
            FIBITMAP* temp = FreeImage_ConvertTo24Bits(bitmap);
            FreeImage_Unload(bitmap);
            bitmap = temp;
        }

        // Grab data from bitmap
        void* imgData = FreeImage_GetBits(bitmap);
        int imgWidth = FreeImage_GetWidth(bitmap);
        int imgHeight = FreeImage_GetHeight(bitmap);

        // Bind the texture
        if (imgData != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                imgWidth,
                imgHeight,
                0,
                GL_BGR_EXT,
                GL_UNSIGNED_BYTE,
                imgData
            );
            // Required since there are no mipmaps.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        }
        else
        {
            std::cerr << "Failed to get texture data from " << textureNames[i] << std::endl;
            exit(3);
        }
    }
}

void processKeyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // Escape key
        exit(0);
    case '1': // Toggle wireframe
        wireframe = !wireframe;
        break;
    case '2': // Toggle axes
        showAxes = !showAxes;
        break;
    case 'a': // Toggle robot animation
        doRobotAnim = !doRobotAnim;
        break;
    case 'i': // Switch to inner camera
        currentCamera = CAMERA_INNER;
        break;
    case 'o': // Switch to outer camera
        currentCamera = CAMERA_OUTER;
        break;
    }

    glutPostRedisplay();
}

void processSpecialKeys(int key, int x, int y)
{
    const float cameraSpeedDivisor = 5;

    switch (key)
    {
    case GLUT_KEY_UP: // Move inner camera forward
        innerCamXYZ += (innerCamDir / cameraSpeedDivisor);
        break;
    case GLUT_KEY_DOWN: // Move inner camera backward
        innerCamXYZ -= (innerCamDir / cameraSpeedDivisor);
        break;
    case GLUT_KEY_LEFT: // Strafe inner camera left
        innerCamXYZ -= (cross(innerCamDir, Point(0, 1, 0)) / cameraSpeedDivisor);
        break;
    case GLUT_KEY_RIGHT:
        innerCamXYZ += (cross(innerCamDir, Point(0, 1, 0)) / cameraSpeedDivisor);
        break;
    }

    glutPostRedisplay();
}

// main() //////////////////////////////////////////////////////////////////////
//
//  Program entry point. Does not process command line arguments.
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // Print controls
    std::cout << "CONTROLS:" << std::endl
        << "Hold left click and drag to move the outer camera" << std::endl
        << "ESC:\t\tExit the program" << std::endl
        << "1:\t\tToggle wireframes" << std::endl
        << "2:\t\tToggle the axes in the center of the screen" << std::endl
        << "a:\t\tToggle the robot's animation on and off" << std::endl
        << "i:\t\tSwitch control to the inner camera" << std::endl
        << "o:\t\tSwitch control to the outer camera" << std::endl
        << "Arrow Keys:\tMove the inner camera" << std::endl;

    //create a double-buffered GLUT window at (50,50) with predefined windowsize
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Colby Reinhart 811053904");

    //give the camera a 'pretty' starting point!
    innerCamXYZ = Point(5, 5, 5);
    innerCamTPR = Point(-M_PI / 4.0, M_PI / 4.0, 1);
    recomputeOrientation(innerCamDir, innerCamTPR);
    innerCamDir.normalize();

    outerCamTPR = Point(1.50, 2.0, 14.0);
    outerCamXYZ = Point(0, 0, 0);
    recomputeOrientation(outerCamXYZ, outerCamTPR);

    // Set up robot's starting position
    robot.translate({ 0, -1, -6 }, false);
    robot.rotateJoint(leftElbow, -20.0f);
    robot.rotateJoint(leftShoulder, 30.0f);
    robot.rotateJoint(rightElbow, -20.0f);
    robot.rotateJoint(rightShoulder, -30.0f);
    robot.rotateJoint(leftHip, -30.0f);
    robot.rotateJoint(leftKnee, 5.0f);
    robot.rotateJoint(rightHip, 20.0f);

    //
    // Create straight line walking animation
    //

    KeyFrame walk1(30);
    walk1.addComponent(new JointRotation(leftShoulder, -30.0f));
    walk1.addComponent(new JointRotation(rightShoulder, 30.0f));
    walk1.addComponent(new JointRotation(leftHip, 25.0f));
    walk1.addComponent(new JointRotation(leftKnee, -5.0f));
    walk1.addComponent(new JointRotation(rightHip, -25.0f));
    walk1.addComponent(new JointRotation(rightKnee, 40.0f));
    walk1.addComponent(new Translation({ 0, 0, 0.75f }));
    robotWalking.addKeyframe(&walk1);

    KeyFrame walk2(30);
    walk2.addComponent(new JointRotation(leftShoulder, -30.0f));
    walk2.addComponent(new JointRotation(rightShoulder, 30.0f));
    walk2.addComponent(new JointRotation(leftHip, 25.0f));
    walk2.addComponent(new JointRotation(rightHip, -25.0f));
    walk2.addComponent(new JointRotation(rightKnee, -35.0f));
    walk2.addComponent(new Translation({ 0, 0, 0.75f }));
    robotWalking.addKeyframe(&walk2);

    KeyFrame walk3(30);
    walk3.addComponent(new JointRotation(leftShoulder, 30.0f));
    walk3.addComponent(new JointRotation(rightShoulder, -30.0f));
    walk3.addComponent(new JointRotation(leftHip, -25.0f));
    walk3.addComponent(new JointRotation(leftKnee, 40.0f));
    walk3.addComponent(new JointRotation(rightHip, 25.0f));
    walk3.addComponent(new JointRotation(rightKnee, -5.0f));
    walk3.addComponent(new Translation({ 0, 0, 0.75f }));
    robotWalking.addKeyframe(&walk3);

    KeyFrame walk4(30);
    walk4.addComponent(new JointRotation(leftShoulder, 30.0f));
    walk4.addComponent(new JointRotation(rightShoulder, -30.0f));
    walk4.addComponent(new JointRotation(leftHip, -25.0f));
    walk4.addComponent(new JointRotation(leftKnee, -35.0f));
    walk4.addComponent(new JointRotation(rightHip, 25.0f));
    walk4.addComponent(new Translation({ 0, 0, 0.75f }));
    robotWalking.addKeyframe(&walk4);

    // repeat the walking cycle three times
    robotWalking.addKeyframe(&walk1);
    robotWalking.addKeyframe(&walk2);
    robotWalking.addKeyframe(&walk3);
    robotWalking.addKeyframe(&walk4);
    robotWalking.addKeyframe(&walk1);
    robotWalking.addKeyframe(&walk2);
    robotWalking.addKeyframe(&walk3);
    robotWalking.addKeyframe(&walk4);
    robotWalking.addKeyframe(&walk1);
    robotWalking.addKeyframe(&walk2);
    robotWalking.addKeyframe(&walk3);
    robotWalking.addKeyframe(&walk4);

    robotWalking.initialize();

    // Create some trees
    trees.emplace_back(new Tree({ -8, 0, -8 }));
    trees.emplace_back(new Tree({ -8, 0, 8 }));
    trees.emplace_back(new Tree({ 8, 0, -8 }));

    //register callback functions
    glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
    glutDisplayFunc(renderCallback);
    glutReshapeFunc(resizeWindow);
    glutMouseFunc(mouseCallback);
    glutKeyboardFunc(processKeyInput);
    glutSpecialFunc(processSpecialKeys);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(0, doAnimation, 0);

    //do some basic OpenGL setup
    initScene();

    //and enter the GLUT loop, never to exit.
    glutMainLoop();

    return(0);
}