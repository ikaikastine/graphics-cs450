//#version 330 compatibility
//varying vec2 vST;
attribute vec3 v_coord;
varying vec4 texCoords;
uniform mat4 m,v,p;

void
main( )
{
    mat4 mvp = p*v*m;
    gl_Position = mvp * vec4(v_coord, 1.0);
    texCoords = vec4(v_coord, 1.0);
    //vST = vec2(1.0, 1.0);
    //gl_FrontColor = vec4( 0., 1., 1., 0. );
    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
}
