#version 110
//bumpmap_multiply.glsl
//TODO: use simple varying variables to pass H values
// per run data
uniform vec3 viewerPosition;
uniform vec4 lightPosition;
uniform vec4 lightDiffuse;
uniform vec4 lightAmbient;
uniform vec4 constColor1;

vec4 diffuse(vec3 normal, vec3 lightv, vec4 color)
{
    return max(dot(normal, lightv), 0.0) * color;
}

uniform sampler2D myTextureMap;
uniform sampler2D myTextureMap2;
uniform sampler2D myTextureMap3;

void main()
{
    vec4 texture0 = texture2D( myTextureMap, gl_TexCoord[0].st );
    vec4 texture1 = texture2D( myTextureMap2, gl_TexCoord[1].st );
    vec4 normal   = texture2D( myTextureMap3, gl_TexCoord[2].st );
    vec4 H;
    // Expand H from [0,1] to [-1,1]
    H.xyz = gl_SecondaryColor.xyz*2.0-1.0;
    H = normalize(H);
    // Expand normal from [0,1] to [-1,1]
    normal.xyz = normal.xyz*2.0 - 1.0;
    float specular_factor = clamp(dot(normal.xyz, H.xyz),0,1);
    specular_factor = specular_factor*specular_factor;
    specular_factor = specular_factor*specular_factor;
    specular_factor = specular_factor*specular_factor;
    // Compute specular - ((N dot H) ^ 4) * normal.a
    vec3 specular_color = vec3(specular_factor,specular_factor,specular_factor) * normal.a;
    gl_FragColor.rgb = texture0.rgb * texture1.rgb * gl_Color.rgb + specular_color; //original shader was multiplying by 8 here ?
    gl_FragColor.a = constColor1.a * texture0.a * texture1.a;
    //TODO add fog
}
