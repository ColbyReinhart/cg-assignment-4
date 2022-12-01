#include "point.h"
#include "robot.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

// GLOBAL VARIABLES ////////////////////////////////////////////////////////////

static size_t windowWidth = 640;
static size_t windowHeight = 480;
static float aspectRatio;

GLint leftMouseButton, rightMouseButton;    //status of the mouse buttons
int mouseX = 0, mouseY = 0;                 //last known X and Y of the mouse
bool sphereOn = false;                      //show the camera radius sphere


//note to students reading this code:
//  yes, I should really be more object-oriented with this code.
//  a lot of this would be simplified and better encapsulated inside
//  of a Camera class. don't let your code get this ugly!
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
    //glShadeModel(GL_FLAT);

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
    //draw a simple grid under the teapot
    glColor3f(1, 1, 1);
    for (int dir = 0; dir < 2; dir++)
    {
        for (int i = -5; i < 6; i++)
        {
            glBegin(GL_LINE_STRIP);
            for (int j = -5; j < 6; j++)
                glVertex3f(dir < 1 ? i : j, -0.73f, dir < 1 ? j : i);
            glEnd();
        }
    }

    //and then draw the teapot itself!
    glEnable(GL_LIGHTING);

    //see documentation for glutSolidTeapot; glutSolidTeapot must be called with 
    //a different winding set. there is a known 'bug' that results in the 
    //winding of the teapot to be backwards.
    glFrontFace(GL_CW);
    robot.display();
    glFrontFace(GL_CCW);

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
    glScalef(1, -2, 0.75);


    glColor3f(0, 1, 0);
    glutWireCube(1.0f);

    //draw the reels on top of the camera...
    for (int currentReel = 0; currentReel < 2; currentReel++)
    {
        float radius = 0.25f;
        int resolution = 32;
        Point reelCenter = Point(0, -0.25 + (currentReel == 0 ? 0 : 0.5), -0.5);
        glBegin(GL_LINES);
        Point s = reelCenter - Point(0, 0.25, 0);
        glVertex3f(s.x, s.y, s.z);
        for (int i = 0; i < resolution; i++)
        {
            float ex = -cosf(i / (float)resolution * M_PI);
            float why = sinf(i / (float)resolution * M_PI);
            Point p = Point(0, ex * radius, -why * radius * 3) + reelCenter;
            glVertex3f(p.x, p.y, p.z);  //end of this line...
            glVertex3f(p.x, p.y, p.z);  //and start of the next
        }
        Point f = reelCenter + Point(0, 0.25, 0);
        glVertex3f(f.x, f.y, f.z);
        glEnd();
    }

    //and just draw the lens shield manually because 
    //i don't want to think about shear matrices.
    //clockwise looking from behind the camera:
    float lensOff = 0.3f;
    float lensOut = 0.2f;
    Point v0 = Point(0.5, 0.5, -0.5);
    Point v1 = Point(-0.5, 0.5, -0.5);
    Point v2 = Point(-0.5, 0.5, 0.5);
    Point v3 = Point(0.5, 0.5, 0.5);

    Point l0 = v0 + Point(lensOut, 0, 0) + Point(0, lensOut, 0) + Point(0, 0, -lensOff);
    Point l1 = v1 + Point(-lensOut, 0, 0) + Point(0, lensOut, 0) + Point(0, 0, -lensOff);
    Point l2 = v2 + Point(-lensOut, 0, 0) + Point(0, lensOut, 0) + Point(0, 0, lensOff);
    Point l3 = v3 + Point(lensOut, 0, 0) + Point(0, lensOut, 0) + Point(0, 0, lensOff);


    glBegin(GL_LINE_STRIP);
    l0.glVertex();
    l1.glVertex();
    l2.glVertex();
    l3.glVertex();
    l0.glVertex();
    glEnd();

    //and connect the two
    glBegin(GL_LINES);
    v0.glVertex();  l0.glVertex();
    v1.glVertex();  l1.glVertex();
    v2.glVertex();  l2.glVertex();
    v3.glVertex();  l3.glVertex();
    glEnd();


    if (sphereOn)
    {
        //draw a point at the center of the camera
        glColor3f(1, 0, 0);
        glPointSize(10);
        glBegin(GL_POINTS);
        Point(0, 0, 0).glVertex();
        glEnd();
        glPopMatrix();

        //do the same set of transformations, but without the scale..
        glPushMatrix();
        glTranslatef(innerCamXYZ.x, innerCamXYZ.y, innerCamXYZ.z);
        glRotatef(-innerCamTPR.x * 180.0 / M_PI, 0, 1, 0);
        glRotatef(innerCamTPR.y * 180.0 / M_PI, 1, 0, 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glColor4f(1, 1, 1, 0.3);
        glutSolidSphere(1, 32, 32);

        glDisable(GL_DEPTH_TEST);
        glColor3f(1, 0, 0);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, -1, 0);
        glEnd();
        glColor3f(0, 0, 1);
        glBegin(GL_POINTS);
        glVertex3f(0, -1, 0);
        glEnd();
        glEnable(GL_DEPTH_TEST);

        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);


        glPopMatrix();
    }
    else {
        glPopMatrix();
    }


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

    float borderWidth = 3;
    //start with the code from the outer camera, which covers the whole screen!
    glViewport(0, 0, windowWidth, windowHeight);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);    glPushMatrix(); glLoadIdentity();   gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    if (currentCamera == CAMERA_OUTER)
        glColor3f(1, 0, 0);
    else
        glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(0, 1); glVertex2f(1, 1); glVertex2f(1, 0);
    glEnd();
    glViewport(borderWidth, borderWidth, windowWidth - borderWidth * 2, windowHeight - borderWidth * 2);
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(0, 1); glVertex2f(1, 1); glVertex2f(1, 0);
    glEnd();

    glMatrixMode(GL_PROJECTION);    glPopMatrix();
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

    ///     draw border and background for preview box in upper corner  //////////////////////

        //next, do the code for the inner camera, which only sets in the top-right
        //corner!
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

    ///     begin drawing scene in upper corner //////////////////////////////////////////////

    glViewport(2 * windowWidth / 3.0, 2 * windowHeight / 3.0,
        windowWidth / 3.0, windowHeight / 3.0);
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

// main() //////////////////////////////////////////////////////////////////////
//
//  Program entry point. Does not process command line arguments.
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    //create a double-buffered GLUT window at (50,50) with predefined windowsize
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("double cameras... woahhhhh double cameras");

    //give the camera a 'pretty' starting point!
    innerCamXYZ = Point(5, 5, 5);
    innerCamTPR = Point(-M_PI / 4.0, M_PI / 4.0, 1);
    recomputeOrientation(innerCamDir, innerCamTPR);
    innerCamDir.normalize();

    outerCamTPR = Point(1.50, 2.0, 14.0);
    outerCamXYZ = Point(0, 0, 0);
    recomputeOrientation(outerCamXYZ, outerCamTPR);

    //register callback functions...
    glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
    glutDisplayFunc(renderCallback);
    glutReshapeFunc(resizeWindow);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotion);

    //do some basic OpenGL setup
    initScene();

    //and enter the GLUT loop, never to exit.
    glutMainLoop();

    return(0);
}