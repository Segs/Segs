#version 110

// per run data
uniform vec4 lightPosition;
uniform vec4 lightDiffuse;
uniform vec4 lightAmbient;
uniform vec4 textureScroll0;
uniform vec4 textureScroll1;

// per-vertex data
attribute vec3 binormal;
attribute vec3 tangent;

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}
vec3 mulNoTranslate(mat4 transform,vec3 val)
{
    return vec3(
                dot(transform[0].xyz,val),
                dot(transform[1].xyz,val),
                dot(transform[2].xyz,val)
                );
}
void main()
{
    gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
    gl_TexCoord[1].xy = gl_MultiTexCoord1.xy;
    gl_TexCoord[2].xy = gl_MultiTexCoord0.xy;

    // transform vertices into projection space using the pre-multiplied matrix
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FogFragCoord = dot(gl_ModelViewProjectionMatrix[3].xyz, gl_Vertex.xyz);
    vec3 position_vs = (gl_ModelViewMatrix * gl_Vertex).xyz;
    vec3 normal_vs = normalize(mulNoTranslate(gl_ModelViewMatrix , gl_Normal)).xyz;
    vec3 binormal_vs = normalize(mulNoTranslate(gl_ModelViewMatrix , binormal));
    vec3 tangent_vs = normalize(mulNoTranslate(gl_ModelViewMatrix , tangent));

    vec3 light_tangentspace;
    light_tangentspace.x = dot(tangent_vs,lightPosition.xyz);
    light_tangentspace.y = dot(binormal_vs,lightPosition.xyz);
    light_tangentspace.z = dot(normal_vs,lightPosition.xyz);

    vec3 view_vs = -normalize(position_vs);
    //half vector in view space
    vec3 h_vs = normalize(view_vs+lightPosition.xyz);
    //half vector to tangent space
    vec3 halfway_ts=vec3(dot(tangent_vs,h_vs),dot(binormal_vs,h_vs),dot(normal_vs,h_vs));

    gl_FrontColor.rgb = light_tangentspace*0.5 + 0.5;
    gl_FrontSecondaryColor.rgb = (halfway_ts*0.5)+0.5;
}
