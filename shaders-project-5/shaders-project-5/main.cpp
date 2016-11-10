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

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "glut.h"

#include "glslprogram.h"
//#include "glslprogram.cpp"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.


// title of these windows:

const char *WINDOWTITLE = { "Project 2 -- Kevin Stine" };
const char *GLUITITLE   = { "User Interface Window" };

// blade parameters:

#define BLADE_RADIUS    1.0
#define BLADE_WIDTH     0.4




// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
    PERSP,
    ORTHO
};

enum Views
{
    OUTSIDE,
    INSIDE
};

enum Textures
{
    REGULAR,
    DISTORTED,
    NONE
};


// which button:

enum ButtonVals
{
    RESET,
    QUIT
};

float   White[] = {1., 1., 1., 1.};
float   Red[] = {1., 0., 0., 1.};
float   Green[] = {0., 1., 0., 1.};
float   Blue[] = {0., 0., 1., 1.};
float   Cyan[] = {0., 1., 1., 1.};

// utility to create an array from 3 seperate values:

float *
Array3( float a, float b, float c)
{
    static float array[4];
    
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = 1;
    return array;
}

float *
MulArray3( float factor, float array0[3])
{
    static float array[4];
    
    array[0] = factor * array0[0];
    array[1] = factor * array0[1];
    array[2] = factor * array0[2];
    array[3] = 1;
    return array;
}


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };
//const GLfloat BACKCOLOR[ ] = { 1., 1., 1., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

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

// Struct for setting up a sphere

bool	Distort;		// global -- true means to distort the texture

struct point {
    float x, y, z;		// coordinates
    float nx, ny, nz;	// surface normal
    float s, t;		// texture coords
};

int		NumLngs, NumLats;
struct point *	Pts;

struct point *
PtsPointer( int lat, int lng )
{
    if( lat < 0 )	lat += (NumLats-1);
    if( lng < 0 )	lng += (NumLngs-1);
    if( lat > NumLats-1 )	lat -= (NumLats-1);
    if( lng > NumLngs-1 )	lng -= (NumLngs-1);
    return &Pts[ NumLngs*lat + lng ];
}

// bmp to texture


int	ReadInt( FILE * );
short	ReadShort( FILE * );


struct bmfh
{
    short bfType;
    int bfSize;
    short bfReserved1;
    short bfReserved2;
    int bfOffBits;
} FileHeader;

struct bmih
{
    int biSize;
    int biWidth;
    int biHeight;
    short biPlanes;
    short biBitCount;
    int biCompression;
    int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    int biClrUsed;
    int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
int     BLADE_ANGLE;
float   Time;
int     MS_IN_THE_ANIMATION_CYCLE;
int     MS_PER_CYCLE = 15000;
GLuint  TopBlades;
GLuint  RearBlades;
GLuint  RandomDraw;
int     WhichView;
int     WhichTex;
GLuint  tex0, tex1, textureBind;
int     textureView;
GLuint  DistortList;
GLuint  TorusList;
GLuint  CubeList;
GLuint  TeaList;

bool    Light0On;
bool    Light1On;
bool    Light2On;
GLuint  LightZero;
GLuint  LightOne;
GLuint  LightTwo;

GLSLProgram	*Pattern;


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void    DoViewMenu( int );
void    DoTexMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );
void    Cross( float[3], float[3], float[3] );
float   Unit( float[3], float[3] );

void    SetMaterial(float, float, float, float);

// main program:

int
main( int argc, char *argv[ ] )
{
    // turn on the glut package:
    // (do this before checking argc and argv since it might
    // pull some command line arguments out)
    
    glutInit( &argc, argv );
    
    
    // setup all the graphics stuff:
    
    InitGraphics( );
    
    
    // create the display structures that will not change:
    
    InitLists( );
    
    // init all the global variables used by Display( ):
    // this will also post a redisplay
    
    Reset( );
    
    
    // setup all the user interface stuff:
    
    InitMenus( );
    
    
    // draw the scene once and wait for some interaction:
    // (this will never return)
    
    glutSetWindow( MainWindow );
    
    Animate();
    
    glutMainLoop( );
    
    
    // this is here to make the compiler happy:
    
    return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
    // put animation stuff in here -- change some global variables
    // for Display( ) to find:
    
    // force a call to Display( ) next time it is convenient:
    
    glutSetWindow( MainWindow );
    
    int ms = glutGet( GLUT_ELAPSED_TIME );
    ms %= MS_PER_CYCLE;
    Time = (float)ms / (float)MS_PER_CYCLE;		// [0.,1.)
    glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
    if( DebugOn != 0 )
    {
        fprintf( stderr, "Display\n" );
    }
    
    
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
    if( WhichProjection == PERSP )
        gluPerspective( 90., 1.,	0.1, 1000. );
    else
        //glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
        glOrtho(-20., 20., -20., 20., 0.1, 1000.);
    
    
    
    // place the objects into the scene:
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    if (WhichView == OUTSIDE)
        gluLookAt( 2., 2., 2.,     0., 0., 0.,     0., 1., 0. );
    else
        gluLookAt( -0.4, 1.8, -4.9,     0., 0., 0.,     0., 1., 0. );
    
    
    
    
    
    
    // set the eye position, look-at position, and up-vector:
    
    
    
    
    // rotate the scene:
    
    glRotatef( (GLfloat)Yrot, 0., 1., 0. );
    glRotatef( (GLfloat)Xrot, 1., 0., 0. );
    
    //glLoadIdentity();
    
    
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
    
    
    // possibly draw the axes:
    
    if( AxesOn != 0 )
    {
        glColor3fv( &Colors[WhichColor][0] );
        glCallList( AxesList );
    }
    
    
    // since we are using glScalef( ), be sure normals get unitized:
    
    glEnable( GL_NORMALIZE );
    
 
    
    // draw the shapes and colors

    
    if (Light0On)
        glEnable(GL_LIGHT0);
    else
        glDisable(GL_LIGHT0);
    if (Light1On)
        glEnable(GL_LIGHT1);
    else
        glDisable(GL_LIGHT1);
    if (Light2On)
        glEnable(GL_LIGHT2);
    else
        glDisable(GL_LIGHT2);
    
    glPushMatrix();
    float x = 2.;
    glTranslatef(x, 0., 0.);
    glRotatef(360*Time, 0., .2, 0.);
    
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, MulArray3(.5f, White));
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightfv(GL_LIGHT0, GL_POSITION, Array3(x, 0, 0.));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Green);
    glLightfv(GL_LIGHT0, GL_SPECULAR, Green);
    glTranslatef(-2., 0., 0.);
    glCallList( LightZero );
    glPopMatrix();
    
    glPushMatrix();
    glCallList( TorusList );
    glCallList( CubeList );
    glPopMatrix();
    
    glPushMatrix();
    glRotatef(360*Time, 0., 0., 1.);
    glTranslatef(0., -1., 1.);
    
    glCallList( TeaList );
    glPopMatrix();
    
    glPushMatrix();
    glCallList( LightOne );
    glCallList( LightTwo );
    glPopMatrix();
    
    
    // swap the double-buffered framebuffers:
    
    glutSwapBuffers( );
    
    
    // be sure the graphics buffer has been sent:
    // note: be sure to use glFlush( ) here, not glFinish( ) !
    
    glFlush( );
}


void
DoAxesMenu( int id )
{
    AxesOn = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
    WhichColor = id - RED;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
    DebugOn = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
    DepthCueOn = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
    switch( id )
    {
        case RESET:
            Reset( );
            break;
            
        case QUIT:
            // gracefully close out the graphics:
            // gracefully close the graphics window:
            // gracefully exit the program:
            glutSetWindow( MainWindow );
            glFinish( );
            glutDestroyWindow( MainWindow );
            exit( 0 );
            break;
            
        default:
            fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
    }
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
    WhichProjection = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}

void
DoViewMenu( int id )
{
    WhichView = id;
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}

void
DoTexMenu (int id )
{
    WhichTex = id;
    glutSetWindow( MainWindow );
    glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
    glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    
    char c;			// one character to print
    for( ; ( c = *s ) != '\0'; s++ )
    {
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
    }
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
    glPushMatrix( );
    glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    float sf = ht / ( 119.05f + 33.33f );
    glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
    char c;			// one character to print
    for( ; ( c = *s ) != '\0'; s++ )
    {
        glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
    }
    glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
    // get # of milliseconds since the start of the program:
    
    int ms = glutGet( GLUT_ELAPSED_TIME );
    
    // convert it to seconds:
    
    return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
    glutSetWindow( MainWindow );
    
    int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
    int colormenu = glutCreateMenu( DoColorMenu );
    for( int i = 0; i < numColors; i++ )
    {
        glutAddMenuEntry( ColorNames[i], i );
    }
    
    int texturemenu = glutCreateMenu( DoTexMenu );
    glutAddMenuEntry( "Regular Texture", 0 );
    glutAddMenuEntry( "Distorted Texture", 1 );
    glutAddMenuEntry( "No Texture", 2 );
    
    int axesmenu = glutCreateMenu( DoAxesMenu );
    glutAddMenuEntry( "Off",  0 );
    glutAddMenuEntry( "On",   1 );
    
    int depthcuemenu = glutCreateMenu( DoDepthMenu );
    glutAddMenuEntry( "Off",  0 );
    glutAddMenuEntry( "On",   1 );
    
    int debugmenu = glutCreateMenu( DoDebugMenu );
    glutAddMenuEntry( "Off",  0 );
    glutAddMenuEntry( "On",   1 );
    
    int projmenu = glutCreateMenu( DoProjectMenu );
    glutAddMenuEntry( "Orthographic",  ORTHO );
    glutAddMenuEntry( "Perspective",   PERSP );
    
    int viewmenu = glutCreateMenu( DoViewMenu );
    glutAddMenuEntry( "Inside", INSIDE );
    glutAddMenuEntry( "Outside", OUTSIDE );
    
    int mainmenu = glutCreateMenu( DoMainMenu );
    glutAddSubMenu(   "Texture",       texturemenu );
    glutAddSubMenu(   "View",          viewmenu );
    glutAddSubMenu(   "Axes",          axesmenu);
    glutAddSubMenu(   "Colors",        colormenu);
    glutAddSubMenu(   "Depth Cue",     depthcuemenu);
    glutAddSubMenu(   "Projection",    projmenu );
    glutAddMenuEntry( "Reset",         RESET );
    glutAddSubMenu(   "Debug",         debugmenu);
    glutAddMenuEntry( "Quit",          QUIT );
    
    // attach the pop-up menu to the right mouse button:
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
    // request the display modes:
    // ask for red-green-blue-alpha color, double-buffering, and z-buffering:
    
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    
    // set the initial window configuration:
    
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );
    
    // open the window and set its title:
    
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
    glutKeyboardFunc( Keyboard );
    glutMouseFunc( MouseButton );
    glutMotionFunc( MouseMotion );
    glutPassiveMotionFunc( NULL );
    glutVisibilityFunc( Visibility );
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
    glutIdleFunc( Animate );
    
    // init glew (a window must be open to do this):
    
#ifdef WIN32
    GLenum err = glewInit( );
    if( err != GLEW_OK )
    {
        fprintf( stderr, "glewInit Error\n" );
    }
    else
        fprintf( stderr, "GLEW initialized OK\n" );
    fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
    
    // do this *after* opening the window and init'ing glew:
    
    char    *patternVert = (char *) "pattern.vert";
    char    *patternFrag = (char *) "pattern.frag";


    
    Pattern = new GLSLProgram( );
    //bool valid = Pattern->Create( patternVert,  patternFrag );
    bool valid = Pattern->Create( "pattern.vert",  "pattern.frag" );

    if( ! valid )
    {
        fprintf( stderr, "Shader cannot be created!\n" );
        DoMainMenu( QUIT );
    }
    else
    {
        fprintf( stderr, "Shader created.\n" );
    }
    Pattern->SetVerbose( false );
    
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
    
    glutSetWindow( MainWindow );
    
    // Setup Torus List
    TorusList = glGenLists( 1 );
    
    glNewList(TorusList, GL_COMPILE);
    glPushMatrix();
    
    glTranslatef(1., 1., -2.);
    
    glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4, White));
    glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
    glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
    glMaterialf(GL_BACK, GL_SHININESS, 5.);
    glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, MulArray3(1., Red));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MulArray3(1., Red));
    glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.7, White));
    glMaterialf(GL_FRONT, GL_SHININESS, 20.);
    glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
    
    glShadeModel(GL_FLAT);
    
    glEnable( GL_LIGHTING );
    
    
    //glColor3f(1, 0, 0);
    
    glutSolidTorus(.5, 1, 100, 100);
    
    glDisable(GL_LIGHT0);
    glPopMatrix();
    glEndList();
    
    
    // Setup Cube List
    
    CubeList = glGenLists( 1 );
    
    glNewList(CubeList, GL_COMPILE);
    glPushMatrix();
    glTranslatef(-1., -1., 0);
    
    glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4, White));
    glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
    glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
    glMaterialf(GL_BACK, GL_SHININESS, 5.);
    glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, MulArray3(1., Blue));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MulArray3(1., Blue));
    glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.7, Blue));
    glMaterialf(GL_FRONT, GL_SHININESS, 0.);
    glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
    
    glShadeModel(GL_SMOOTH);
    glEnable( GL_LIGHTING );
    
    glutSolidCube( 1 );
    
    glDisable(GL_LIGHT1);
    glPopMatrix();
    glEndList();
    
    TeaList = glGenLists(1);
    glNewList(TeaList, GL_COMPILE);
    glPushMatrix();
    
    glTranslatef(1., 0., 0.);
    
    glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4, Cyan));
    glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., Cyan));
    glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
    glMaterialf(GL_BACK, GL_SHININESS, 5.);
    glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, MulArray3(1., Cyan));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MulArray3(1., Cyan));
    glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.7, Cyan));
    glMaterialf(GL_FRONT, GL_SHININESS, 8.);
    glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
    
    glShadeModel(GL_SMOOTH);
    
    glEnable( GL_LIGHTING );
    
    glutSolidTeapot( .5 );
    glDisable(GL_LIGHT0);
    glPopMatrix();
    glEndList();
    
    LightZero = glGenLists( 1 );
    glNewList(LightZero, GL_COMPILE);
    
    
    
    glPushMatrix();
    glTranslatef(-3., -2., 0.);
    glColor3f(1., 1., 1.);
    glutSolidSphere(.25, 10, 10);
    glPopMatrix();
    glEndList();
    
    LightOne = glGenLists( 1 );
    glNewList(LightOne, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0., 0., 2.);
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, MulArray3(.3f, White));
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    glLightfv(GL_LIGHT1, GL_POSITION, Array3(0., 0., 2.));
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, Array3(0., 0., 0.));
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 4.);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.);
    glLightfv(GL_LIGHT1, GL_AMBIENT, Array3(0., 0., 0.));
    
    glLightfv(GL_LIGHT1, GL_DIFFUSE, MulArray3(1., White));
    glLightfv(GL_LIGHT1, GL_SPECULAR, MulArray3(.7, White));
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.);
    
    
    glColor3f(1., 0., 0.);
    
    glutSolidSphere(.25, 10, 10);
    glPopMatrix();
    glEndList();
    
    LightTwo = glGenLists( 1 );
    glNewList(LightTwo, GL_COMPILE);
    glPushMatrix();
    glTranslatef(1., 1., 0.);
    
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, MulArray3(.3f, Blue));
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    glLightfv(GL_LIGHT2, GL_POSITION, Array3(1., 1., 0.));
    glLightfv(GL_LIGHT2, GL_AMBIENT, Array3(0., 0., 0.));
    glLightfv(GL_LIGHT2, GL_DIFFUSE, MulArray3(1., Blue));
    glLightfv(GL_LIGHT2, GL_SPECULAR, MulArray3(.7, Blue));
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.);
    
    glutSolidSphere(.25, 10, 10);
    glPopMatrix();
    glEndList();
    
    // create the axes:
    /*
    AxesList = glGenLists( 1 );
    glNewList( AxesList, GL_COMPILE );
    glColor3f(1., 1., 1.);
    glLineWidth( AXES_WIDTH );
    Axes( 1.5 );
    glLineWidth( 1. );
    glEndList( );
    */
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
    if( DebugOn != 0 )
        fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );
    
    switch( c )
    {
        case 'p':
        case 'P':
            WhichProjection = PERSP;
            break;
            
        case 'o':
        case 'O':
            WhichProjection = ORTHO;
            break;
        
        case '0':
            Light0On = ! Light0On;
            break;
            
        case '1':
            Light1On = ! Light1On;
            break;
            
        case '2':
            Light2On = ! Light2On;
            break;
            
        case 'q':
        case 'Q':
        case ESCAPE:
            DoMainMenu( QUIT );	// will not return here
            break;				// happy compiler
            
            
        default:
            fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
    }
    
    // force a call to Display( ):
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
    int b = 0;			// LEFT, MIDDLE, or RIGHT
    
    if( DebugOn != 0 )
        fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );
    
    
    // get the proper button bit mask:
    
    switch( button )
    {
        case GLUT_LEFT_BUTTON:
            b = LEFT;		break;
            
        case GLUT_MIDDLE_BUTTON:
            b = MIDDLE;		break;
            
        case GLUT_RIGHT_BUTTON:
            b = RIGHT;		break;
            
        default:
            b = 0;
            fprintf( stderr, "Unknown mouse button: %d\n", button );
    }
    
    
    // button down sets the bit, up clears the bit:
    
    if( state == GLUT_DOWN )
    {
        Xmouse = x;
        Ymouse = y;
        ActiveButton |= b;		// set the proper bit
    }
    else
    {
        ActiveButton &= ~b;		// clear the proper bit
    }
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
    if( DebugOn != 0 )
        fprintf( stderr, "MouseMotion: %d, %d\n", x, y );
    
    
    int dx = x - Xmouse;		// change in mouse coords
    int dy = y - Ymouse;
    
    if( ( ActiveButton & LEFT ) != 0 )
    {
        Xrot += ( ANGFACT*dy );
        Yrot += ( ANGFACT*dx );
    }
    
    
    if( ( ActiveButton & MIDDLE ) != 0 )
    {
        Scale += SCLFACT * (float) ( dx - dy );
        
        // keep object from turning inside-out or disappearing:
        
        if( Scale < MINSCALE )
            Scale = MINSCALE;
    }
    
    Xmouse = x;			// new current position
    Ymouse = y;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
    ActiveButton = 0;
    AxesOn = 1;
    DebugOn = 0;
    DepthCueOn = 0;
    Scale  = 1.0;
    WhichColor = WHITE;
    WhichProjection = PERSP;
    Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
    if( DebugOn != 0 )
        fprintf( stderr, "ReSize: %d, %d\n", width, height );
    
    // don't really need to do anything since window size is
    // checked each time in Display( ):
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
    if( DebugOn != 0 )
        fprintf( stderr, "Visibility: %d\n", state );
    
    if( state == GLUT_VISIBLE )
    {
        glutSetWindow( MainWindow );
        glutPostRedisplay( );
    }
    else
    {
        // could optimize by keeping track of the fact
        // that the window is not visible and avoid
        // animating or redrawing it ...
    }
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
    0.f, 1.f, 0.f, 1.f
};

static float xy[ ] = {
    -.5f, .5f, .5f, -.5f
};

static int xorder[ ] = {
    1, 2, -3, 4
};

static float yx[ ] = {
    0.f, 0.f, -.5f, .5f
};

static float yy[ ] = {
    0.f, .6f, 1.f, 1.f
};

static int yorder[ ] = {
    1, 2, 3, -2, 4
};

static float zx[ ] = {
    1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[ ] = {
    .5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[ ] = {
    1, 2, 3, 4, -5, 6
};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
    glBegin( GL_LINE_STRIP );
    glVertex3f( length, 0., 0. );
    glVertex3f( 0., 0., 0. );
    glVertex3f( 0., length, 0. );
    glEnd( );
    glBegin( GL_LINE_STRIP );
    glVertex3f( 0., 0., 0. );
    glVertex3f( 0., 0., length );
    glEnd( );
    
    float fact = LENFRAC * length;
    float base = BASEFRAC * length;
    
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < 4; i++ )
    {
        int j = xorder[i];
        if( j < 0 )
        {
            
            glEnd( );
            glBegin( GL_LINE_STRIP );
            j = -j;
        }
        j--;
        glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
    }
    glEnd( );
    
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < 5; i++ )
    {
        int j = yorder[i];
        if( j < 0 )
        {
            
            glEnd( );
            glBegin( GL_LINE_STRIP );
            j = -j;
        }
        j--;
        glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
    }
    glEnd( );
    
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < 6; i++ )
    {
        int j = zorder[i];
        if( j < 0 )
        {
            
            glEnd( );
            glBegin( GL_LINE_STRIP );
            j = -j;
        }
        j--;
        glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
    }
    glEnd( );
    
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
    // guarantee valid input:
    
    float h = hsv[0] / 60.f;
    while( h >= 6. )	h -= 6.;
    while( h <  0. ) 	h += 6.;
    
    float s = hsv[1];
    if( s < 0. )
        s = 0.;
    if( s > 1. )
        s = 1.;
    
    float v = hsv[2];
    if( v < 0. )
        v = 0.;
    if( v > 1. )
        v = 1.;
    
    // if sat==0, then is a gray:
    
    if( s == 0.0 )
    {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }
    
    // get an rgb from the hue itself:
    
    float i = floor( h );
    float f = h - i;
    float p = v * ( 1.f - s );
    float q = v * ( 1.f - s*f );
    float t = v * ( 1.f - ( s * (1.f-f) ) );
    
    float r, g, b;			// red, green, blue
    switch( (int) i )
    {
        case 0:
            r = v;	g = t;	b = p;
            break;
            
        case 1:
            r = q;	g = v;	b = p;
            break;
            
        case 2:
            r = p;	g = v;	b = t;
            break;
            
        case 3:
            r = p;	g = q;	b = v;
            break;
            
        case 4:
            r = t;	g = p;	b = v;
            break;
            
        case 5:
            r = v;	g = p;	b = q;
            break;
    }
    
    
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}


float
Dot( float v1[3], float v2[3] )
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}
void
Cross( float v1[3], float v2[3], float vout[3] )
{
    float tmp[3];
    tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
    tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
    tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
    vout[0] = tmp[0];
    vout[1] = tmp[1];
    vout[2] = tmp[2];
}
float
Unit( float vin[3], float vout[3] )
{
    float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
    if( dist > 0.0 )
    {
        dist = sqrt( dist );
        vout[0] = vin[0] / dist;
        vout[1] = vin[1] / dist;
        vout[2] = vin[2] / dist;
    }
    else
    {
        vout[0] = vin[0];
        vout[1] = vin[1];
        vout[2] = vin[2];
    }
    return dist;
}

void
DrawPoint( struct point *p )
{
    glNormal3f( p->nx, p->ny, p->nz );
    glTexCoord2f( p->s, p->t );
    glVertex3f( p->x, p->y, p->z );
}

void
SetMaterial( float r, float g, float b,  float shininess )
{
    glMaterialfv( GL_BACK, GL_EMISSION, Array3( 0., 0., 0. ) );
    glMaterialfv( GL_BACK, GL_AMBIENT, MulArray3( .4f, White ) );
    glMaterialfv( GL_BACK, GL_DIFFUSE, MulArray3( 1., White ) );
    glMaterialfv( GL_BACK, GL_SPECULAR, Array3( 0., 0., 0. ) );
    glMaterialf (  GL_BACK, GL_SHININESS, 2.f );
    glMaterialfv( GL_FRONT, GL_EMISSION, Array3( 0., 0., 0. ) );
    glMaterialfv( GL_FRONT, GL_AMBIENT, Array3( r, g, b ) );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, Array3( r, g, b ) );
    glMaterialfv( GL_FRONT, GL_SPECULAR, MulArray3( .8f, White ) );
    glMaterialf ( GL_FRONT, GL_SHININESS, shininess );
}
