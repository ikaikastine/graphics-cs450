//#version 330

uniform float uKa, uKd, uKs;
uniform vec3 uColor;
uniform vec3 uSpecularColor;
uniform float uShininess;
varying vec2 vST;
varying vec3 vN;
varying vec3 vL;
varying vec3 vE;
varying vec3 uS0;
varying vec3 uT0;
varying vec3 uSize;


varying vec4 texCoords;
uniform sampler2D mytexture;
const float PI = 	3.14159265;
const float AMP = 	0.2;		// amplitude
const float W = 	2.;		// frequency
varying vec3 uDs;
uniform float uTime;

void
main( )
{
    vec3 myColor = uColor;
    
    if( uS0-uSize/2. < vST.s  &&  vST.s < uS0+uSize/2.  && uT0-uSize/2. < vST.t   &&   vST.t < uT0+uSize/2. ) {
        myColor = vec3( 1., 0., 0. );
    }
    
    vec3 ambient = uKa * myColor;
    
    gl_FragColor = vec4( myColor,  1. );
}
