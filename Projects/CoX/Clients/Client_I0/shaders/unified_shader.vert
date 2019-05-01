#version 330
#extension GL_ARB_enhanced_layouts:enable

#define VERTEX_PROCESSING_NONE 0
#define VERTEX_PROCESSING_SKINNING 1

#define BUMP_MODE_NONE 0
#define BUMP_MODE_BASIC 1
#define BUMP_MODE_ALL 2

#define LIGHT_MODE_NONE 0
#define LIGHT_MODE_PRE_LIT 1
#define LIGHT_MODE_DIFFUSE 2
#define LIGHT_MODE_LIT 3
#define LIGHT_MODE_BUMP_LIT 4
const int c_TextureUnitCount = TEXTURES;
const int c_autogen_uv0 = REFLECTION_MODE_T0;
const int c_autogen_uv1 = REFLECTION_MODE_T1;
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

#if FLAT_SHADED==1
#define SHADE_MODEL flat
#else
#define SHADE_MODEL smooth
#endif

///////////////////////////////////////////////////////////////////////////////
// vertex shader uniforms
///////////////////////////////////////////////////////////////////////////////
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
#if TC_XFORM == TEX_XFORM_OFFSET
uniform vec4 textureScroll; // xy - scroll for Texture 0, zw scroll for Texture 1
#elif TC_XFORM == TEX_XFORM_MATRIX
uniform mat4 textureMatrix0;
uniform mat4 textureMatrix1;
#endif
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
uniform vec4 boneMatrices[48]; // mat 3x4
#endif

///////////////////////////////////////////////////////////////////////////////
// per-vertex attributes
///////////////////////////////////////////////////////////////////////////////
in vec4 vertexPos;
in vec3 vertexNormal;
in vec2 vertexUV0;
in vec2 vertexUV1;
in vec4 vertexColor;

#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
in vec2 boneWeights; // attr 1
in vec2 boneIndices; // attr 5
#endif
#if BUMP_MODE==BUMP_MODE_BASIC
in vec4 tangent;
#endif

///////////////////////////////////////////////////////////////////////////////
// Vertex shader outputs
///////////////////////////////////////////////////////////////////////////////
out VS_Output {
    SHADE_MODEL vec4 Color;
    vec2 texCoord0;
    vec2 texCoord1;
    vec2 texCoord2;
#if BUMP_MODE>=BUMP_MODE_BASIC
    SHADE_MODEL vec3 light_ts;
    SHADE_MODEL vec3 view_ts;
#endif
#ifdef FOG
    SHADE_MODEL float fogFragCoord;
#endif
} v_out;
#ifdef TRANSFORM_FEEDBACK
layout(xfb_buffer = 0) out vec4 outValue;
#endif

// default vertex shader
vec3 reflectionMap(in vec3 normal, in vec3 ecPosition3)
{
   vec3 u = normalize(ecPosition3);
   return (reflect(u, normal));
}
vec4 ftexgen(in vec3 normal, in vec4 ecPosition)
{
    vec3 ecPosition3 = (vec3(ecPosition))/ecPosition.w;
    vec3 reflection = reflectionMap( normal, ecPosition3 );
    return vec4( reflection, 1.0);
}

void calculateTextureCoordinates(in vec3 normal, in vec4 ecPosition)
{
    // Calculate base and blend texture coordinates
#if TC_XFORM == TEX_XFORM_OFFSET
    vec2 uv0	= vertexUV0.xy + textureScroll.xy;		// @todo combine these into one constant without affecting register combiner path
    vec2 uv1	= vertexUV1.xy + textureScroll.zw;
#elif TC_XFORM == TEX_XFORM_MATRIX
    vec2 uv0 = vec4( textureMatrix0 * vec4(vertexUV0,0,1) ).xy;
    vec2 uv1 = vec4( textureMatrix1 * vec4(vertexUV1,0,1) ).xy;
#else	// TC_XFORM == NONE
    vec2 uv0	= vertexUV0.xy;
    vec2 uv1	= vertexUV1.xy;
#endif
    uv0 = c_autogen_uv0==1 ? ftexgen(normal,ecPosition).xy : uv0;
    uv1 = c_autogen_uv1==1 ? ftexgen(normal,ecPosition).xy : uv1;
    if(c_TextureUnitCount>0)
        v_out.texCoord0 = uv0;
    if(c_TextureUnitCount>1)
        v_out.texCoord1 = uv1;
    if(c_TextureUnitCount>2)
        v_out.texCoord2 = uv1; // normal map coords
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
    // @note bone matrices transform directly to view space
#if VERTEX_PROCESSING==VERTEX_PROCESSING_SKINNING
    res.xyz = boneTransform(v,int(boneIndices.x))* boneWeights.x;
    // second bone contribution
    res.xyz += boneTransform(v,int(boneIndices.y)) * boneWeights.y;
#else
    res = modelViewMatrix * v;
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
vec4 lit(float NdotL, float NdotH, float m)
{
  float specular = (NdotL > 0) ? pow(max(0.0, NdotH), m) : 0;
  return vec4(1.0, max(0.0, NdotL), specular, 1.0);
}
void calculateColorBasedOnLightMode(const int light_mode, vec4 position_vs, vec3 light, vec3 normal,vec3 view)
{
    switch (light_mode)
    {
        case LIGHT_MODE_NONE:
            v_out.Color = vec4(1,1,1,1); //globalColor.rgba * vertexColor;
            break;
        case LIGHT_MODE_PRE_LIT:
            v_out.Color = vertexColor;
            break;
        case LIGHT_MODE_DIFFUSE:
            v_out.Color = clamp( dot( normal, light),0.0,1.0) * globalColor + light0.Ambient;
            break;
        case LIGHT_MODE_LIT:
        {
            vec3 H_vs = normalize( light + view );
            // Compute diffuse and specular dot products and use LIT to compute
            // lighting coefficients.
            float NdotL = dot(normal, light);
            float NdotH = dot(normal, H_vs);
            vec4 dots = lit( NdotL, NdotH, 128.0 );

            // Accumulate color contributions.
            vec4 lightAccum = (( dots.y * light0.Diffuse ) + light0.Ambient );

            v_out.Color.xyz = (( dots.z * light0.Specular ) + lightAccum ).xyz;
            v_out.Color.w = vertexColor.w;
        }
    }
}
void calculateVertexFog(vec3 vertex_pos)
{
#ifdef FOG
    v_out.fogFragCoord = length(vertex_pos);
#endif
}
void main()
{
    vec4 position_vs = preprocessVertex4(vertexPos);
    position_vs.w=1.0;
    gl_Position = projectionMatrix * position_vs;
    calculateVertexFog(position_vs.xyz);

    vec3 normal = preprocessVertex3(vertexNormal);
    vec3 light = light0.Position.xyz;
    vec3 view;
#if LIGHT_SPACE == MODEL
    // Compute lighting vectors in model space (attribute normal is already in model space)
    light    = normalize( light - vertexPos.xyz );
    view    = normalize( viewerPosition.xyz - vertexPos.xyz ); // view vector in model space (vector to eye)
#else
    // VIEW space lighting
    #ifndef SKIN
        normal = normalize(mat3(modelViewMatrix) * vertexNormal.xyz);
    #endif
    view = normalize( -position_vs ).xyz;    // view vector in view space (vector to eye) is just reverse of position vector
#endif
    // per vertex light calculations
    calculateColorBasedOnLightMode(LIGHT_MODE,position_vs,light,normal,view);

#if (BUMP_MODE==BUMP_MODE_BASIC) && defined(UNLIT)
    bumpMappingNeedsALightToWork();
#endif
#if BUMP_MODE==BUMP_MODE_BASIC
    vec3 tangent_vs = preprocessVertex3(tangent.xyz);
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
        tangent_vs = normalize(mat3(modelViewMatrix) * tangent_vs);
    #endif
#endif
    vec3 o_position_ts, o_light_ts;
    calc_tangent_space_light_and_position( normal, tangent_vs.xyz, sign(tangent.w), position_vs.xyz, light, o_position_ts, o_light_ts );
    v_out.light_ts    = vec3( o_light_ts);
    v_out.view_ts     = vec3( -o_position_ts);
#endif

    calculateTextureCoordinates(normal,gl_Position);
#ifdef TRANSFORM_FEEDBACK
    outValue = gl_Position;
#endif
}
