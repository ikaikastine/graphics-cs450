
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "glut.h"
#include "glslprogram.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

GLSLProgram	*Pattern;
float		 Time;

void	DoMainMenu( int );
void	Reset( );


// which button:

enum ButtonVals
{
    RESET,
    QUIT
};

int		MainWindow;				// window id for main graphics window


void
Display( )
{
	

	Pattern->Use( );
	Pattern->SetUniformVariable( "uTime",  Time );
	//glCallList( SphereList );
	Pattern->Use( 0 );

	
}


void
InitGraphics( )
{


	// do this *after* opening the window and init'ing glew:

	Pattern = new GLSLProgram( );
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

void
DoMainMenu( int id )
{
    switch( id )
    {
        case RESET:
            //Reset( );
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
    
    //InitLists( );
    
    // init all the global variables used by Display( ):
    // this will also post a redisplay
    
    //Reset( );
    
    
    // setup all the user interface stuff:
    
    //InitMenus( );
    
    
    // draw the scene once and wait for some interaction:
    // (this will never return)
    
    glutSetWindow( MainWindow );
    
    //Animate();
    
    glutMainLoop( );
    
    
    // this is here to make the compiler happy:
    
    return 0;
}


// not great programming style, but if you are uncomfortable with compiling
// multiple files in visual studio, this works too...

//#include "glslprogram.cpp"
