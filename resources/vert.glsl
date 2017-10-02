#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat3 T;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space
attribute vec2 aTex;

varying vec3 vertPos; // Pass to fragment shader
varying vec3 normal;  // Pass to fragment shader
varying vec2 vTex;

void main()
{
	gl_Position = P * MV * aPos;
    vTex = (T * vec3(aTex, 1.0)).xy;
    normal = (MV * vec4(aNor, 0.0)).xyz;
    vertPos = vec3(MV * aPos);
}
