#version 110
// default vertex shader
// per run data
uniform vec3 viewerPosition;
uniform vec4 lightPosition;
uniform vec4 lightDiffuse;
uniform vec4 lightAmbient;
uniform vec4 textureScroll0;
uniform vec4 textureScroll1;

// per-vertex data

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}
void main()
{
    // transform vertices into projection space using the pre-multiplied matrix
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    //gl_FogFragCoord = dot(gl_ModelViewProjectionMatrix[3].xyz, position);
    gl_FrontColor.rgba = gl_Color.rgba;
    gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
    gl_TexCoord[1].xy = gl_MultiTexCoord0.xy;
}
