//#version 330

varying vec3 vColor;
varying float Time;

void
main( )
{
    //gl_FragColor = vec4( vColor, 1. ) * Time;
    gl_FragColor = vec4( vColor,  1. );
}
