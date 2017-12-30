#version 110
/*
    2 bone skinning + bump map
*/

// per-vertex data
//attribute vec3 position;
//attribute vec3 normal; // attr2
attribute vec2 boneWeights; // attr 1
attribute vec2 boneIndices; // attr5
attribute vec3 binormal;
attribute vec3 tangent;

uniform vec3 viewerPosition;
uniform vec4 lightPosition;
uniform vec4 lightDiffuse;
uniform vec4 lightAmbient;
uniform vec4 textureScroll0;
uniform vec4 textureScroll1;
uniform vec4 boneMatrices[48]; // mat 3x4

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}
vec3 boneTransform(vec4 pos,int boneidx)
{
    vec3 res;
    res.x = dot(pos,boneMatrices[boneidx+0]);
    res.y = dot(pos,boneMatrices[boneidx+1]);
    res.z = dot(pos,boneMatrices[boneidx+2]);
    return res;
}
vec3 boneTransform(vec3 pos,int boneidx)
{
    vec3 res;
    res.x = dot(pos,boneMatrices[boneidx+0].xyz);
    res.y = dot(pos,boneMatrices[boneidx+1].xyz);
    res.z = dot(pos,boneMatrices[boneidx+2].xyz);
    return res;
}

void main()
{
    vec4 position_vs;
    vec3 binormal_vs;
    vec3 tangent_vs;

    position_vs.xyz = boneTransform(gl_Vertex,int(boneIndices.x))* boneWeights.x;
    vec3 normal_vs = boneTransform(gl_Normal,int(boneIndices.x))* boneWeights.x;
    binormal_vs = boneTransform(binormal,int(boneIndices.x))* boneWeights.x;
    tangent_vs = boneTransform(tangent,int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    position_vs.xyz += boneTransform(gl_Vertex,int(boneIndices.y)) * boneWeights.y;
    normal_vs += boneTransform(gl_Normal,int(boneIndices.y)) * boneWeights.y;
    binormal_vs += boneTransform(binormal,int(boneIndices.y))* boneWeights.y;
    tangent_vs += boneTransform(tangent,int(boneIndices.y))* boneWeights.y;
    position_vs.w=1.0;
    // position in screen space
    position_vs = gl_ProjectionMatrix*position_vs;
    // normalize tangent basis
    normal_vs = normalize(normal_vs);
    binormal_vs = normalize(binormal_vs);
    tangent_vs = normalize(tangent_vs);

     vec3 light_modelspace = lightPosition.xyz - gl_Vertex.xyz;
    //view vector in model space
    vec3 view_vector = normalize(viewerPosition - gl_Vertex.xyz);
    // Light vector in tangent space
    vec3 L_ts;
    L_ts.x = dot(tangent_vs, lightPosition.xyz);
    L_ts.y = dot(binormal_vs, lightPosition.xyz);
    L_ts.z = dot(normal_vs, lightPosition.xyz);
    // Compute view vector in view space
    vec3 V_vs = -normalize(position_vs).xyz;

    // Compute half vector in view space
    vec3 halfway = normalize(view_vector+light_modelspace);
    // Half vector to tangent space
    vec3 HV_ts;
    HV_ts.x = dot(tangent_vs, halfway);
    HV_ts.y = dot(binormal_vs, halfway);
    HV_ts.z = dot(normal_vs, halfway);
    // fog
    gl_FogFragCoord = dot(gl_ModelViewProjectionMatrix[2], position_vs);
    //Range compress L_ts into the primary color output
    gl_FrontColor.xyz = L_ts/2.0 + 0.5;
    //Range compress H_ts into the primary color output
    gl_FrontSecondaryColor.rgb = (HV_ts*0.5)+0.5;
    gl_Position = position_vs;
    gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
    gl_TexCoord[1].xy = gl_MultiTexCoord1.xy;
    gl_TexCoord[2].xy = gl_MultiTexCoord1.xy;
}
