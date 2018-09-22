#version 330
#extension GL_ARB_arrays_of_arrays : require

#define BUMP_MODE_NONE 0
#define BUMP_MODE_BASIC 1

#define LIGHT_MODE_NONE 0
#define LIGHT_MODE_PRE_LIT 1
#define LIGHT_MODE_LIT 2
#define LIGHT_MODE_BUMP_LIT 3

#define FRAGMENT_MODE_MULTIPLY 0
#define FRAGMENT_MODE_MULTIPLY_REG 1
#define FRAGMENT_MODE_COLORBLEND_DUAL 2
#define FRAGMENT_MODE_ADDGLOW 3
#define FRAGMENT_MODE_ALPHADETAIL 4
#if FLAT_SHADED==1
#define SHADE_MODEL flat
#else
#define SHADE_MODEL smooth
#endif
const uint stippler[16][4] = uint[16][4](
uint[4](0x55555555u,0x00000000u,0x11111111u,0x00000000u), // 0 -15
uint[4](0x55555555u,0x00000000u,0x11111111u,0x00000000u), // 16-31
uint[4](0x55555555u,0x00000000u,0x11111111u,0x00000000u), // 31-47
uint[4](0x55555555u,0x00000000u,0x55555555u,0x00000000u),
uint[4](0x55555555u,0x22222222u,0x55555555u,0x00000000u),
uint[4](0x55555555u,0x22222222u,0x55555555u,0x88888888u),
uint[4](0x55555555u,0xAAAAAAAAu,0x55555555u,0x88888888u), // 7
uint[4](0x55555555u,0xAAAAAAAAu,0x55555555u,0xAAAAAAAAu),
uint[4](0x55555555u,0xEEEEEEEEu,0x55555555u,0xAAAAAAAAu),
uint[4](0x55555555u,0xEEEEEEEEu,0x55555555u,0xBBBBBBBBu),
uint[4](0x55555555u,0xFFFFFFFFu,0x55555555u,0xBBBBBBBBu),
uint[4](0x55555555u,0xFFFFFFFFu,0x55555555u,0xFFFFFFFFu),
uint[4](0xEEEEEEEEu,0xFFFFFFFFu,0x55555555u,0xFFFFFFFFu),
uint[4](0xEEEEEEEEu,0xFFFFFFFFu,0xBBBBBBBBu,0xFFFFFFFFu),
uint[4](0xFFFFFFFFu,0xFFFFFFFFu,0xBBBBBBBBu,0xFFFFFFFFu),
uint[4](0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu)
);

const int c_TextureUnitCount = TEXTURES;
const int c_UseMaterialBlending = COLORBLEND;
const int c_BumpMode = BUMP_MODE;
const int c_FragmentMode = FRAGMENT_MODE;
const int c_LodAlpha = LOD_ALPHA; // if 1 => constColor1.w contains lod alpha to apply to final color
const int c_AlphaDiscard = ALPHA_DISCARD_MODE;
// input from vertex shader
//
in VS_Output {
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
} v_in;
// output from vertex shader
out vec4 out_Color;
// input from application

uniform vec4 constColor1;
uniform vec4 constColor2;

uniform vec4 glossFactor;
#define STIPLE_ALPHA_VALUE glossFactor.y
#define ALPHA_DISCARD_VALUE glossFactor.z
struct FogParameters
{
    vec4 color;
    vec4 v_block; //density/start/end/unused
    // for linear fog the scale is end-start
    int mode; // 0 - no fog, 1 - linear_fog
};
uniform FogParameters fog_params; // enabled,gl_Fog.end

struct lightParameters
{
    vec4 Position;
    vec4 Diffuse;
    vec4 Ambient;
    vec4 Specular;
};
uniform lightParameters light0;
uniform vec4 lightsOn; // provided only when in AddGlow mode
uniform vec3 viewerPosition;

uniform sampler2D myTextureMap;
uniform sampler2D myTextureMap2;
uniform sampler2D myTextureMap3;

// helper funcs
vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}

vec3 calcFog(vec3 in_color)
{
#ifdef FOG
    if(fog_params.mode!=0)
    {
        float fog;
        fog = (fog_params.v_block[2] - v_in.fogFragCoord) * (fog_params.v_block[2]-fog_params.v_block[1]);
        fog = clamp(fog, 0.0, 1.0);
        return mix( fog_params.color.xyz, in_color.xyz, fog);
    }
#endif
    return in_color;
}
vec4 calc_dual_tint(vec4 const0, vec4 const1, vec4 tex0, vec4 tex1)
{
    vec4 dual_color;
    dual_color.rgb = mix(const1.rgb,const0.rgb,tex1.rgb);
    dual_color.rgb = mix(dual_color.rgb, vec3(1.0, 1.0, 1.0),tex0.w);
    dual_color.rgb *= tex0.rgb;
    dual_color.a   = tex1.a * const0.a;
    return dual_color;

}
void discardStippledFragment()
{
#if USE_STIPPLING==1
   uint[4] pattern = stippler[uint(STIPLE_ALPHA_VALUE)>>4];
   uint selected_pattern = pattern[int(gl_FragCoord.y) %4];
   int selectedbit = int(gl_FragCoord.x) & 0x1F;
   bool last_bit = (selected_pattern & uint(1<<selectedbit))==uint(0);
   if(last_bit)
      discard;
#endif
}
vec4 calc_tint(in vec4 color, in vec4 tex)
{
    vec3 tinted = mix(color.rgb, tex.rgb, tex.a);
    return vec4( tinted, color.a );
}
void main()
{
    //original shaders were saling some color operations by 8?
    const float scale_factor = 1.0; //8.0
    const float scale_factor_reg = 1.0; //8.0
    discardStippledFragment();
    vec4 outColor; // color accumulator variable
    vec4 tex_base;
    vec4 tex_blend;
    vec4 tex_normal;
    // Fetch textures from enabled units
    if(c_TextureUnitCount>0)
        tex_base = texture2D( myTextureMap, v_in.texCoord0 );
    else
        tex_base = vec4(1,1,1,1);
    if(c_TextureUnitCount>1)
        tex_blend = texture2D( myTextureMap2, v_in.texCoord1 );
    else
        tex_blend = vec4(1,1,1,1);
    if(c_TextureUnitCount>2)
    {
        tex_normal = texture2D( myTextureMap3, v_in.texCoord2 );
        // Expand normal from [0,1] to [-1,1]
        tex_normal.xyz = tex_normal.xyz*2.0 - 1.0;
    }
    else
        tex_normal = vec4(0,0,0,0);
    switch(c_FragmentMode)
    {
    case FRAGMENT_MODE_MULTIPLY:
        outColor = tex_blend*tex_base;
        outColor *= scale_factor*v_in.Color; // scale factor is actually only used in MULTIPLY_REG fs
        break;
    case FRAGMENT_MODE_MULTIPLY_REG:
        outColor = tex_base * tex_blend;
        outColor.rgb	*= scale_factor_reg * v_in.Color.rgb;
        if(c_LodAlpha==1)
            outColor.a		*= constColor1.a;		// modulate by lod alpha
        break;

    case FRAGMENT_MODE_COLORBLEND_DUAL:
        outColor = calc_dual_tint(constColor1,constColor2,tex_base,tex_blend);
        outColor.rgb *= scale_factor; // *v_in.Color.rgb;
        break;
    case FRAGMENT_MODE_ADDGLOW:
        outColor = calc_tint(constColor1, tex_base );
        outColor.rgb	*= scale_factor * v_in.Color.rgb;
        //add glow for places with lightsOn.x set
        outColor.xyz += tex_blend.xyz * lightsOn.x; // TODO: consider adding glow after fog processing ?
        break;
    case FRAGMENT_MODE_ALPHADETAIL:
        // Uses texture 0 (base) alpha as a blend mask between texture 0 and 1.
        // Uses texture 1 (blend) alpha to modulate current constant alpha
        outColor.rgb	= mix( tex_blend.rgb, tex_base.rgb, tex_base.a );
        outColor.rgb	*= scale_factor * v_in.Color.rgb;
        outColor.a		= constColor1.a * tex_blend.a;
        break;
    }
    #
    if(c_BumpMode==1)
    {
#if BUMP_MODE==BUMP_MODE_BASIC
        vec3 view_ts = normalize(v_in.view_ts);
        vec3 light_ts = normalize(v_in.light_ts);
        vec3 halfway = normalize(view_ts+light_ts);
        float specular_factor = clamp(dot(tex_normal.xyz, halfway), 0.0, 1.0);
        float diffuse_factor = clamp(dot(tex_normal.xyz, light_ts), 0.0, 1.0);
        vec4 diffuse = diffuse_factor*light0.Diffuse;
        specular_factor = pow(specular_factor, 128);
        vec3 specular_val = vec3(specular_factor,specular_factor,specular_factor);
        //outColor.rgb = outColor.rgb*diffuse.rgb + specular_val.rgb + light0.Ambient.rgb;
        outColor.rgb = outColor.rgb*(diffuse.rgb  + light0.Ambient.rgb) + specular_val.rgb; //*light0.Specular.rgb;
#endif
    }
    if(c_AlphaDiscard==1)
    {
        if(outColor.a<=ALPHA_DISCARD_VALUE)
            discard;
    }
    else if(c_AlphaDiscard==2)
    {
        if(outColor.a>ALPHA_DISCARD_VALUE)
            discard;
    }
    out_Color.rgb = calcFog(outColor.rgb);
    out_Color.a = outColor.a;
}
