#version 110
#define COLORBLEND 1
#define ALPHA_DISCARD_MODE 0
#define ALPHA_DISCARD_VALUE 0.000000
#define LIGHT_MODE 3
#define BUMP_MODE 1
#define TEXTURES 3
#define VERTEX_PROCESSING 1
#define FRAGMENT_MODE 0
#define LOD_ALPHA 0
#define VERTEX_PROCESSING_NONE 0
#define VERTEX_PROCESSING_SKINNING 1

#define BUMP_MODE_NONE 0
#define BUMP_MODE_BASIC 1

#define LIGHT_MODE_NONE 0
#define LIGHT_MODE_PRE_LIT 1
#define LIGHT_MODE_LIT 2
#define LIGHT_MODE_BUMP_LIT 3

const int c_TextureUnitCount = TEXTURES;

// Optional flags
// SKINNING
// BUMP
// UNLIT
// FOG
// COLOR
// TEX_XFORM
#define TEX_XFORM_NONE 0
#define TEX_XFORM_OFFSET 1
#define TEX_XFORM_MATRIX 2
#define TEX_XFORM_GEN 3
// per-vertex data
attribute vec4 vertexPos;
attribute vec3 vertexNormal;
attribute vec2 vertexUV0;
attribute vec2 vertexUV1;
attribute vec4 vertexColor;

#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
attribute vec2 boneWeights; // attr 1
attribute vec2 boneIndices; // attr5
#endif
#if BUMP_MODE==BUMP_MODE_BASIC
attribute vec3 binormal;
attribute vec3 tangent;
#endif

// per run data
struct lightParameters
{
    vec4 Position;
    vec4 Diffuse;
    vec4 Ambient;
    vec4 Specular;
};
struct FogParameters
{
    vec4 color;
    vec4 v_block; //density/start/end/unused
                  // for linear fog the scale is end-start
    int mode; // 0 - no fog, 1 - linear_fog
};

uniform FogParameters fog_params; // enabled,gl_Fog.end
uniform lightParameters light0;

uniform vec4 globalColor; // replacement for gl_Color
uniform vec3 viewerPosition;
#if TEX_XFORM == TEX_XFORM_OFFSET
uniform vec4 textureScroll; // xy - scroll for Texture 0, zw scroll for Texture 1
#elif TEX_XFORM == TEX_XFORM_MATRIX
uniform mat4 textureMatrix0;
uniform mat4 textureMatrix1;
#endif
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
uniform vec4 boneMatrices[48]; // mat 3x4
#endif

#if BUMP_MODE==BUMP_MODE_BASIC
varying vec3 VertexOut_light_ts;
varying vec3 VertexOut_view_ts;
#endif
void calculateTextureCoordinates()
{
    // Calculate base and blend texture coordinates
#if TC_XFORM == TEX_XFORM_OFFSET
    vec2 uv0 = vertexUV0.xy + textureScroll.xy;		// @todo combine these into one constant without affecting register combiner path
    vec2 uv1 = vertexUV1.xy + textureScroll.zw;
#elif TC_XFORM == TEX_XFORM_MATRIX
    vec2 uv0 = vec4(textureMatrix0 * vertexUV0).xy;
    vec2 uv1 = vec4(textureMatrix1, vertexUV1).xy;
#else	// TC_XFORM == NONE
    vec2 uv0 = vertexUV0.xy;
    vec2 uv1 = vertexUV1.xy;
#endif
}
// default vertex shader
vec3 reflectionMap(in vec3 normal, in vec3 ecPosition3)
{
    float NdotU, m;
    vec3 u;
    u = normalize(ecPosition3);
    return (reflect(u, normal));
}
vec4 ftexgen(in vec3 normal, in vec4 ecPosition)
{
    vec3 ecPosition3;    ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
    vec3 reflection = reflectionMap(normal, ecPosition3);
    return vec4(reflection, 1.0);
}
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
vec3 boneTransform(vec4 pos, int boneidx)
{
    vec3 res;
    res.x = dot(pos, boneMatrices[boneidx + 0]);
    res.y = dot(pos, boneMatrices[boneidx + 1]);
    res.z = dot(pos, boneMatrices[boneidx + 2]);
    return res;
}
vec3 boneTransform(vec3 pos, int boneidx)
{
    vec3 res;
    res.x = dot(pos, boneMatrices[boneidx + 0].xyz);
    res.y = dot(pos, boneMatrices[boneidx + 1].xyz);
    res.z = dot(pos, boneMatrices[boneidx + 2].xyz);
    return res;
}
#endif

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}
vec4 preprocessVertex4(vec4 v) {
    vec4 res;
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
    res.xyz = boneTransform(vertexPos, int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    res.xyz += boneTransform(vertexPos, int(boneIndices.y)) * boneWeights.y;
    res.w = 1.0;
#else
    res = v;
#endif
    return res;
}
vec3 preprocessVertex3(vec3 n) {
    vec3 res;
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
    res = boneTransform(n, int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    res += boneTransform(n, int(boneIndices.y)) * boneWeights.y;
#else
    res = n;
#endif
    return res;
}
vec3 toTangentSpace(vec3 v, vec3 tangent_vs, vec3 binormal_vs, vec3 normal_vs)
{
    vec3 res;
    res.x = dot(tangent_vs, v);
    res.y = dot(binormal_vs, v);
    res.z = dot(normal_vs, v);
    return res;
}
void main()
{
    vec4 position_vs = preprocessVertex4(vertexPos);
    vec3 normal_vs = preprocessVertex3(vertexNormal);
#if (BUMP_MODE==BUMP_MODE_BASIC) && defined(UNLIT)
    bumpMappingNeedsALightToWork();
#endif
    vec3 modelview_pos;
#if BUMP_MODE==BUMP_MODE_BASIC
    vec3 binormal_vs = preprocessVertex3(binormal);
    vec3 tangent_vs = preprocessVertex3(tangent);
    // normalize tangent basis
    normal_vs = normalize(normal_vs);
    binormal_vs = normalize(binormal_vs);
    tangent_vs = normalize(tangent_vs);
    // light vector is in model space, calculate light to given position
    vec3 light_modelspace = normalize(light0.Position.xyz - position_vs.xyz);// -position_vs.xyz;
                                                                             //view vector in model space
    vec3 view_vector = normalize(viewerPosition - position_vs.xyz);

    // Light vector in tangent space
    //VertexOut_light_ts = toTangentSpace(light_modelspace, tangent_vs, binormal_vs, normal_vs).xyz;
    VertexOut_view_ts = toTangentSpace(-normalize(position_vs).xyz, tangent_vs, binormal_vs, normal_vs).xyz;
    VertexOut_light_ts = toTangentSpace(light0.Position.xyz, tangent_vs, binormal_vs, normal_vs).xyz;
    
    // transform vertices into projection space using the pre-multiplied matrix
    modelview_pos = position_vs.xyz;
    gl_Position = gl_ProjectionMatrix * position_vs;
#else
    modelview_pos = vec4(gl_ModelViewMatrix * position_vs).xyz;
    gl_Position = gl_ModelViewProjectionMatrix * position_vs;
#endif
#ifdef FOG
    gl_FogFragCoord = length((modelview_pos).xyz);
#endif
#if LIGHT_MODE==LIGHT_MODE_NONE
    gl_FrontColor.rgba = globalColor.rgba*vertexColor;
#elif LIGHT_MODE==LIGHT_MODE_PRE_LIT
    gl_FrontColor.rgba = globalColor.rgba*vertexColor;
#else
    //Compute the normal
    normal_vs = normalize(gl_NormalMatrix * normal_vs);
    vec4 ecPosition = gl_ModelViewMatrix * position_vs;
    vec3 lightvec = light0.Position.xyz - ecPosition.xyz;
    gl_FrontColor.rgb = diffuse(normal_vs.xyz, lightvec, globalColor);
	gl_FrontColor.a = vertexColor.a;
#endif
    if (c_TextureUnitCount>0)
        gl_TexCoord[0].xy = vertexUV0;
    if (c_TextureUnitCount>1)
        gl_TexCoord[1].xy = vertexUV1;
    if (c_TextureUnitCount>2)
        gl_TexCoord[2].xy = vertexUV1; // normal map coords
}
