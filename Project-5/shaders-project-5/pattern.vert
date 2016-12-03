//#version 330 compatibility
//varying vec2 vST;
attribute vec3 v_coord;
varying vec4 texCoords;
uniform mat4 m,v,p;
varying float Time;
varying vec3 vColor;
varying float uDs;
attribute float Animate;

void
main( )
{
    Time = Time;
    //vec4 pos = gl_Vertex * Time;
    vec4 pos = gl_Vertex;
    vColor = pos.xyz;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
