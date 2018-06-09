#version 330
#extension GL_ARB_enhanced_layouts:enable

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
in vec4 vertexPos;
in vec3 vertexNormal;
in vec2 vertexUV0;
in vec2 vertexUV1;
in vec4 vertexColor;

#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
in vec2 boneWeights; // attr 1
in vec2 boneIndices; // attr5
#endif
#if BUMP_MODE==BUMP_MODE_BASIC
in vec4 tangent;
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
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
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

out VS_Output {
    #if BUMP_MODE==BUMP_MODE_BASIC
    vec3 light_ts;
    vec3 view_ts;
    #endif
    vec2 texCoord0;
    vec2 texCoord1;
    vec2 texCoord2;
    #ifdef FOG
    float fogFragCoord;
    #endif
    vec4 Color;
} v_out;
#ifdef TRANSFORM_FEEDBACK
layout(xfb_buffer = 0) out vec4 outValue;
#endif

void calculateTextureCoordinates()
{
    // Calculate base and blend texture coordinates
#if TC_XFORM == TEX_XFORM_OFFSET
    vec2 uv0	= vertexUV0.xy + textureScroll.xy;		// @todo combine these into one constant without affecting register combiner path
    vec2 uv1	= vertexUV1.xy + textureScroll.zw;
#elif TC_XFORM == TEX_XFORM_MATRIX
    vec2 uv0 = vec4( textureMatrix0 * vertexUV0 ).xy;
    vec2 uv1 = vec4( textureMatrix1 * vertexUV1 ).xy;
#else	// TC_XFORM == NONE
    vec2 uv0	= vertexUV0.xy;
    vec2 uv1	= vertexUV1.xy;
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
    vec3 ecPosition3;    ecPosition3 = (vec3(ecPosition))/ecPosition.w;
    vec3 reflection = reflectionMap( normal, ecPosition3 );
    return vec4( reflection, 1.0);
}
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
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
#endif

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}
vec4 preprocessVertex4(vec4 v) {
    vec4 res;
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
    res.xyz = boneTransform(v,int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    res.xyz += boneTransform(v,int(boneIndices.y)) * boneWeights.y;
#else
    res = v;
#endif
    return res;
}
vec3 preprocessVertex3(vec3 n) {
    vec3 res;
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
    res = boneTransform(n,int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    res += boneTransform(n,int(boneIndices.y)) * boneWeights.y;
#else
    res = n;
#endif
    return res;
}
vec3 toTangentSpace(vec3 v,vec3 tangent_vs,vec3 binormal_vs,vec3 normal_vs)
{
    vec3 res;
    res.x = dot(tangent_vs, v);
    res.y = dot(binormal_vs, v);
    res.z = dot(normal_vs, v);
    return res;
}
void calc_tangent_space_light_and_position( vec3 normal_vs, vec3 tangent_vs, float binormal_sign, vec3 position_vs, vec3 light_vs, out vec3 o_position_ts, out vec3 o_light_ts )
{
    // Calculate light and position(view) vector in tangent space to supply fragment program

    // calc binormal from tangent/normal
    vec3 binormal_vs = cross( tangent_vs, normal_vs) * binormal_sign;

    // Form a change of basis matrix out of the vectors
    mat3 M_ts = mat3(tangent_vs * binormal_sign, binormal_vs, normal_vs); // * binormal_sign was added to make tangent_vs seam-less ????

    // Compute light vector in tangent space
    o_light_ts = M_ts * light_vs;

    // Compute position in tangent space (used by fragment shader to determine half vector)
    o_position_ts = M_ts * position_vs;
}
void main()
{
    vec4 position_vs = preprocessVertex4(vertexPos);
    position_vs.w=1.0;

    vec3 normal = preprocessVertex3(vertexNormal);
    vec3 light = light0.Position.xyz;
    vec3 view;
#if LIGHT_SPACE == MODEL
    // Compute lighting vectors in model space (attribute normal is already in model space)
    light    = normalize( light - vertexPos.xyz );
    view    = normalize( viewerPosition.xyz - vertexPos.xyz ); // view vector in model space (vector to eye)
#else // most common case is VIEW space lighting
    #ifndef SKIN
        normal = normalize(mat3(modelViewMatrix) * normal.xyz);
    #endif
    view = normalize( -position_vs );    // view vector in view space (vector to eye) is just reverse of position vector
#endif

#if (BUMP_MODE==BUMP_MODE_BASIC) && defined(UNLIT)
    bumpMappingNeedsALightToWork();
#endif
    vec3 modelview_pos;
#if BUMP_MODE==BUMP_MODE_BASIC
    vec4 tangent_vs = preprocessVertex4(tangent);
    // normalize tangent basis
    vec3 light_modelspace = light0.Position.xyz - vertexPos.xyz;
    //view vector in model space
    vec3 view_vector = normalize(viewerPosition - vertexPos.xyz);
    // Light vector in tangent space
    // Compute view vector in view space
    vec3 V_vs = -normalize(view_vector).xyz;

#if LIGHT_SPACE == VIEW
    // lighting vectors xform into view space for skinned models
    #ifndef SKINNING
        tangent_vs.xyz = normalize(mat3(modelViewMatrix) * tangent_vs.xyz);
    #endif
#endif
    vec3 o_position_ts, o_light_ts;
    calc_tangent_space_light_and_position( normal, tangent_vs.xyz, sign(tangent.w), position_vs.xyz, light, o_position_ts, o_light_ts );
    v_out.light_ts    = vec3( o_light_ts);
    v_out.view_ts     = vec3( -o_position_ts);

    // transform vertices into projection space using the pre-multiplied matrix
    modelview_pos = position_vs.xyz;
    gl_Position = projectionMatrix * position_vs;
#else
    modelview_pos = vec4(modelViewMatrix * position_vs).xyz;
    gl_Position = projectionMatrix * modelViewMatrix *position_vs;
#endif
#ifdef FOG
    v_out.fogFragCoord = length((modelview_pos).xyz);
#endif
#if LIGHT_MODE==LIGHT_MODE_NONE
    v_out.Color.rgba = globalColor.rgba*vertexColor;
#elif LIGHT_MODE==LIGHT_MODE_PRE_LIT
    v_out.Color.rgba = globalColor.rgba*vertexColor;
#else
    //Compute the normal
    normal = normalize(transpose(inverse(modelViewMatrix)) * vec4(normal,0)).xyz;
    vec4 ecPosition = modelViewMatrix * position_vs;
    vec3 lightvec = light0.Position.xyz - ecPosition.xyz;
    v_out.Color.rgb = diffuse(normal.xyz,lightvec,globalColor).rgb;
	v_out.Color.a = globalColor.a;
#endif
    if(c_TextureUnitCount>0)
        v_out.texCoord0 = vertexUV0;
    if(c_TextureUnitCount>1)
        v_out.texCoord1 = vertexUV1;
    if(c_TextureUnitCount>2)
        v_out.texCoord2 = vertexUV1; // normal map coords
#ifdef TRANSFORM_FEEDBACK
    outValue = gl_Position;
#endif
}
