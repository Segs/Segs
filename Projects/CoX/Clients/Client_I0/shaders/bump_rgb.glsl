#version 110

// per run data
uniform vec3 viewerPosition;
uniform vec4 lightPosition;
uniform vec4 lightDiffuse;
uniform vec4 lightAmbient;
uniform vec4 textureScroll0;
uniform vec4 textureScroll1;

// per-vertex data
attribute vec3 binormal;
attribute vec3 tangent;
attribute vec3 rgbs;

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}
void main()
{
    // transform vertices into projection space using the pre-multiplied matrix
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FogFragCoord = dot(gl_ModelViewProjectionMatrix[3].xyz, gl_Vertex.xyz);
    vec3 light_modelspace = lightPosition.xyz - gl_Vertex.xyz;
    light_modelspace = normalize(light_modelspace);
    gl_FrontColor.rgb = rgbs;
    // specular computations

    //view vector in model space
    vec3 view_vector = normalize(viewerPosition - gl_Vertex.xyz);
    // halfway vector
    vec3 halfway = normalize(view_vector+light_modelspace);
    // halfway vector in tangent space
    vec3 halfway_ts = vec3( dot(halfway,tangent),dot(halfway,binormal),dot(halfway,gl_Normal));
    gl_FrontSecondaryColor.rgb = (halfway_ts*0.5)+0.5;
    gl_TexCoord[0].xy = textureScroll0.xy + gl_MultiTexCoord0.xy;
    gl_TexCoord[1].xy = textureScroll1.xy + gl_MultiTexCoord1.xy;
    gl_TexCoord[2].xy = textureScroll1.xy + gl_MultiTexCoord1.xy;
}
