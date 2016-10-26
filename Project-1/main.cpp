#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#define ESCAPE      0x1b

const int INIT_WINDOW_SIZE = { 600 };
const char *WINDOWTITLE = { "Project 1 -- Kevin Stine" };

enum Projections {
    ORTHO,
    PERSP
};

const float MINSCALE = { 0.05f };

const float BOXSIZE = { 2.f };

float Xrot, Yrot;
float Scale;

int MainWindow;
int WhichProjection;
int DepthCueOn;
int AxesOn;
int WhichColor;
GLuint BoxList;


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };

enum Colors
{
    RED,
    YELLOW,
    GREEN,
    CYAN,
    BLUE,
    MAGENTA,
    WHITE,
    BLACK
};

char * ColorNames[ ] =
{
    "Red",
    "Yellow",
    "Green",
    "Cyan",
    "Blue",
    "Magenta",
    "White",
    "Black"
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 1., 1., 1., 0. };

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] =
{
    { 1., 0., 0. },		// red
    { 1., 1., 0. },		// yellow
    { 0., 1., 0. },		// green
    { 0., 1., 1. },		// cyan
    { 0., 0., 1. },		// blue
    { 1., 0., 1. },		// magenta
    { 1., 1., 1. },		// white
    { 0., 0., 0. },		// black
};

// Function Prototypes

void InitGraphics( );
void InitLists( );
void Resize( int, int );
void Display( );
int TO_RADIAN( float );

int main(int argc, char * argv[]) {
    
    glutInit( &argc, argv );
    
    InitGraphics( );
    
    InitLists();
    
    glutSetWindow(MainWindow);
    
    glutMainLoop();
    
    return 0;
    
}

void InitGraphics() {
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);
    
    MainWindow = glutCreateWindow( WINDOWTITLE );
    glutSetWindowTitle( WINDOWTITLE );
    
    // set the framebuffer clear values:
    
    glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );
    
    // setup the callback functions:
    // DisplayFunc -- redraw the window
    // ReshapeFunc -- handle the user resizing the window
    // KeyboardFunc -- handle a keyboard input
    // MouseFunc -- handle the mouse button going down or up
    // MotionFunc -- handle the mouse moving with a button down
    // PassiveMotionFunc -- handle the mouse moving with a button up
    // VisibilityFunc -- handle a change in window visibility
    // EntryFunc	-- handle the cursor entering or leaving the window
    // SpecialFunc -- handle special keys on the keyboard
    // SpaceballMotionFunc -- handle spaceball translation
    // SpaceballRotateFunc -- handle spaceball rotation
    // SpaceballButtonFunc -- handle spaceball button hits
    // ButtonBoxFunc -- handle button box hits
    // DialsFunc -- handle dial rotations
    // TabletMotionFunc -- handle digitizing tablet motion
    // TabletButtonFunc -- handle digitizing tablet button hits
    // MenuStateFunc -- declare when a pop-up menu is in use
    // TimerFunc -- trigger something to happen a certain time from now
    // IdleFunc -- what to do when nothing else is going on
    
    glutSetWindow( MainWindow );
    glutDisplayFunc( Display );
    glutReshapeFunc( Resize );
    glutKeyboardFunc( NULL );
    glutMouseFunc( NULL );
    glutMotionFunc( NULL );
    glutPassiveMotionFunc( NULL );
    glutVisibilityFunc( NULL );
    glutEntryFunc( NULL );
    glutSpecialFunc( NULL );
    glutSpaceballMotionFunc( NULL );
    glutSpaceballRotateFunc( NULL );
    glutSpaceballButtonFunc( NULL );
    glutButtonBoxFunc( NULL );
    glutDialsFunc( NULL );
    glutTabletMotionFunc( NULL );
    glutTabletButtonFunc( NULL );
    glutMenuStateFunc( NULL );
    glutTimerFunc( -1, NULL, 0 );
    glutIdleFunc( NULL );
    
}

void Display() {
    // set which window we want to do the graphics into:
    
    glutSetWindow( MainWindow );
    
    
    // erase the background:
    
    glDrawBuffer( GL_BACK );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );
    
    
    // specify shading to be flat:
    
    glShadeModel( GL_FLAT );
    
    
    // set the viewport to a square centered in the window:
    
    GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
    GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
    GLsizei v = vx < vy ? vx : vy;			// minimum dimension
    GLint xl = ( vx - v ) / 2;
    GLint yb = ( vy - v ) / 2;
    glViewport( xl, yb,  v, v );
    
    
    // set the viewing volume:
    // remember that the Z clipping  values are actually
    // given as DISTANCES IN FRONT OF THE EYE
    // USE gluOrtho2D( ) IF YOU ARE DOING 2D !
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    if( WhichProjection == ORTHO )
        glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
    else
        gluPerspective( 90., 1.,	0.1, 1000. );
    
    
    // place the objects into the scene:
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    
    
    // set the eye position, look-at position, and up-vector:
    
    gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );
    
    
    // rotate the scene:
    
    glRotatef( (GLfloat)Yrot, 0., 1., 0. );
    glRotatef( (GLfloat)Xrot, 1., 0., 0. );
    
    
    // uniformly scale the scene:
    
    if( Scale < MINSCALE )
        Scale = MINSCALE;
    glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
    
    
    // set the fog parameters:
    
    if( DepthCueOn != 0 )
    {
        glFogi( GL_FOG_MODE, FOGMODE );
        glFogfv( GL_FOG_COLOR, FOGCOLOR );
        glFogf( GL_FOG_DENSITY, FOGDENSITY );
        glFogf( GL_FOG_START, FOGSTART );
        glFogf( GL_FOG_END, FOGEND );
        glEnable( GL_FOG );
    }
    else
    {
        glDisable( GL_FOG );
    }
    
    
    // since we are using glScalef( ), be sure normals get unitized:
    
    glEnable( GL_NORMALIZE );
    
    
    // draw the current object:
    
    glCallList( BoxList );
    
    
    // swap the double-buffered framebuffers:
    
    glutSwapBuffers( );
    
    
    // be sure the graphics buffer has been sent:
    // note: be sure to use glFlush( ) here, not glFinish( ) !
    
    glFlush( );
}

void
InitLists( )
{
   
    glMatrixMode   (GL_MODELVIEW);
    glClear        (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();
    glTranslatef    (0.0, 0.0, -15.0);
    
    glColor3f (0.8, 0.2, 0.1);              // Red ball displaced to left.
    glPushMatrix ();
    glTranslatef    (-1.5, 0.0, 0.0);
    glRotatef       (60.0, 1,0,0);
      // Red ball rotates at twice the rate of blue ball.
    glutSolidSphere (1.0, 20, 50);
    glPopMatrix ();
    
    glColor3f (0.1, 0.2, 0.8);              // Blue ball displaced to right.
    glPushMatrix ();
    glTranslatef    (1.5, 0.0, 0.0);
    glRotatef       (60.0, 1,0,0);
    glutSolidSphere (1.0, 20, 50);
    glPopMatrix ();
    
    glutSwapBuffers();

}

// called when user resizes the window:

void
Resize( int width, int height )
{
    
    // don't really need to do anything since window size is
    // checked each time in Display( ):
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}

int TO_RADIAN (float angle) {
    float radians = angle * (3.14/180);
    return radians;
}

