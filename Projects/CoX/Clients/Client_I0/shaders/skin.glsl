#version 110
/*
    2 bone skinning
*/

// per-vertex data
//attribute vec3 position;
//attribute vec3 normal; // attr2
attribute vec2 boneWeights; // attr 1
attribute vec2 boneIndices; // attr5
attribute vec3 binormal;
attribute vec3 tangent;

/*
PARAM   projection[4]    = { state.matrix.projection };       # Projection matrix
PARAM   ambient          = program.local[4];                  # ambient color
PARAM   diffuse          = program.local[5];                  # diffuse Color * Kd
PARAM   L_vs             = program.local[6];                  # Light vector in view space
PARAM   boneMatrices[48] = { program.local[16..63] };         # Matrices for each bone; each matrix has 3 rows
ATTRIB  boneWeights      = vertex.attrib[1];
ATTRIB  boneIndices      = vertex.attrib[5];
*/
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightPosition;
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
    position_vs.xyz = boneTransform(gl_Vertex,int(boneIndices.x))* boneWeights.x;
    vec3 normal_vs = boneTransform(gl_Normal,int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    position_vs.xyz += boneTransform(gl_Vertex,int(boneIndices.y)) * boneWeights.y;
    normal_vs += boneTransform(gl_Normal,int(boneIndices.y)) * boneWeights.y;
    position_vs.w=1.0;
    // position in screen space
    position_vs = gl_ModelViewProjectionMatrix*position_vs;
    // fog
    gl_FogFragCoord = dot(gl_ModelViewProjectionMatrix[2], position_vs);
    // color
    gl_FrontColor.rgba = diffuse(normal_vs,lightPosition.xyz,lightDiffuse)+lightAmbient;
    gl_BackColor = vec4(0,0,0,0);
    gl_Position = position_vs;
    gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
    gl_TexCoord[1].xy = gl_MultiTexCoord1.xy;
}
