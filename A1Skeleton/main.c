/*******************************************************************
           Multi-Part Model Construction and Manipulation
********************************************************************/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"

const int meshSize = 16;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

// Lighting/shading and material properties for submarine - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat submarine_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat submarine_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat submarine_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat submarine_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground / sea floor 
static QuadMesh groundMesh;

#define PI 3.141592654

static GLfloat theta = 0.0;
static GLfloat moveX = 0.0;
static GLfloat moveY = 0.0;
static GLfloat moveZ = 0.0;
static GLfloat subAngle = 0.0;
static GLfloat speed = 0.0;

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
Vector3D ScreenToWorld(int x, int y);


int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(200, 30);
    glutCreateWindow("Assignment 1");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);

    // Start event loop, never returns
    glutMainLoop();

    return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);   // This light is currently off

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
    glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    // Set up ground/sea floor quad mesh
    Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
    Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
    Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
    groundMesh = NewQuadMesh(meshSize);
    InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

    Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
    Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
    Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
    SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

    // Set up the bounding box of the scene
    // Currently unused. You could set up bounding boxes for your objects eventually.
    //Set(&BBox.min, -8.0f, 0.0, -8.0);
    //Set(&BBox.max, 8.0f, 6.0,  8.0);
}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
    gluLookAt(0.0, 6.0, 22.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);


	// Draw ground/sea floor
	DrawMeshQM(&groundMesh, meshSize);


    // Draw Submarine

    // Set submarine material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);


    // Apply transformations to move submarine
    // ...

	glTranslatef(moveX, moveY, moveZ);


	glRotatef(subAngle, 0.0, 1.0, 0.0);

    // Apply transformations to construct submarine, modify this!
    // sub body
	glPushMatrix();
    glTranslatef(0.0, 4.0, 0.0);
    glRotatef(0.0, 0.0, 1.0, 0.0);
    glScalef(8.0, 1.0, 1.0);
    glutSolidSphere(1, 20, 20);
    glPopMatrix();

	//top
	glPushMatrix();
	glTranslatef(1.8, 5.3, 0.0);
	//glRotatef(0.0, 0.0, 1.0, 0.0);
	glScalef(2.0, 1.0, 0.6);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.8, 5.1, 0.0);
	glRotatef(90, 0.0, 0.0, 1.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glScalef(1.5, 0.5, 0.2);
	glutSolidCube(1.0);
	glPopMatrix();

	//back wings
	glPushMatrix();
	glTranslatef(-7, 4, 0.0);
	glRotatef(90, 0.0, 0.0, 1.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.0, 0.3);
	glutSolidCube(1.0);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(-7, 4, 0.0);
	//glRotatef(90, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.0, 0.3);
	glutSolidCube(1.0);
	glPopMatrix();


	//propeller 1 
	glPushMatrix();
	glTranslatef(-8.0, 3.95, 0.0);
	glRotatef(theta, 1.0, 0.0, 0.0);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 0.0, 1.0);
	glScalef(4.0, 1.0, 0.4);
	glutSolidCube(0.3);
	glPopMatrix();

	//propeller 2
	glPushMatrix();
	glTranslatef(-8.0, 3.95, 0.0);
	glRotatef(theta, 1.0, 0.0, 0.0);
	glRotatef(90.0, 0.0, 0.0, 1.0);
	glScalef(4.0, 1.0, 0.4);
	glutSolidCube(0.3);
	glPopMatrix();


    glutSwapBuffers();   // Double buffering, swap buffers
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode - 
    // display function will then set up camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int threads = 0;
void moveSub()
{
	//printf("speed is : %f\n", speed);
	//theta += speed*100 *7.0;
	if (speed >= 0.01) {
		//printf("forwards\n");
		theta += speed*30 *7.0;
		if (theta > 360.0)
			theta -= 360.0;
	}
	else if (speed <= 0.01){
		//printf("backwards\n");
		theta += speed * 30 *7.0;
		if (theta < 0)
			theta += 360.0;
	}
	

	moveX += speed*cos(PI / 180 * -subAngle);
	moveZ += speed*sin(PI / 180 * -subAngle);
	//if (threads < 1) {
	glutTimerFunc(100, moveSub, 0);
		
	//}
	
	glutPostRedisplay();
}



// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'f':
		
		speed += 0.02;
		if (speed >= 0.2) {
			speed = 0.2;
		}
		
		if (threads < 1) {
			moveSub();
			threads += 1;
		}
		//glutTimerFunc(200, moveSubF, 0);
		glutPostRedisplay();
        break;
	 case 'b':
		 
		 speed -= 0.02;
		 if (speed <= -0.2) {
			 speed = -0.2;
		 }
		
		 if (threads < 1) {
			 moveSub();
			 threads += 1;
		 }
		 //glutTimerFunc(200, moveSubB, 0);
		 glutPostRedisplay();
		 break;
	}

    glutPostRedisplay();   // Trigger a window redisplay
}


// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	switch (key) {
		// Help key
	case GLUT_KEY_F1:

		printf("Use the 'f' and 'b' keys to move the submarine forwards and backwards\n");
		printf("Use the left and right arrow keys to turn the submarine left and right\n");
		printf("Use the up and down arrow keys to move the submarine up and down\n");
		break;
	case GLUT_KEY_DOWN:
		moveY -= 0.1;
		glutPostRedisplay();
		break;

	case GLUT_KEY_UP:
		moveY += 0.1;
		glutPostRedisplay();
		break;

	case GLUT_KEY_RIGHT:
		subAngle -= 10;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		subAngle += 10;
		glutPostRedisplay();
		break;
	default:
		break;
	}

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
    currentButton = button;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
			

        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;
        }
        break;
    default:
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON)
    {
        ;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
    // you will need to finish this if you use the mouse
    return NewVector3D(0, 0, 0);
}



