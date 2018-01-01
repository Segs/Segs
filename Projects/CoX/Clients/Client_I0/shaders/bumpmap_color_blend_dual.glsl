#version 110

uniform vec4 constColor1; // env 0
uniform vec4 constColor2; // env 1

uniform vec4 lightPosition;
uniform vec4 lightAmbient; // local 0
uniform vec4 lightDiffuse; // local 1
uniform vec4 glossFactor; // local 2

uniform sampler2D myTextureMap;
uniform sampler2D myTextureMap2;
uniform sampler2D myTextureMap3;

void main()
{
    //vec4 color = {0, 1, 0, 1};

    vec4 H;                // Half vector
    vec4 L;                // Light vector
    vec4 diffuse;
    vec4 specular;
    vec4 outColor;
    vec4 gloss;
    vec4 a;
    vec4 b;
    vec4 c;

    vec4 texture0 = texture2D( myTextureMap, gl_TexCoord[0].st );
    vec4 texture1 = texture2D( myTextureMap2, gl_TexCoord[1].st );
    vec4 normal = texture2D( myTextureMap3, gl_TexCoord[2].st );

    // Expand H from [0,1] to [-1,1]
    H.xyz = gl_SecondaryColor.xyz*2.0-1.0;
    H = normalize(H);

    // Expand L from [0,1] to [-1,1]
    L.xyz = gl_Color.xyz * 2.0 - 1.0;
    L = normalize(L);
    a.xyz = mix(constColor2,constColor1,texture1).xyz;
    b.xyz = mix(a.xyz,vec3(1.0,1.0,1.0),texture0.w);
    c.xyz = (texture0 * b).xyz;

    // Expand normal from [0,1] to [-1,1]
    normal.xyz = normal.xyz*2.0 - 1.0;

    // Compute lighting
    diffuse.w = clamp(dot(normal.xyz, L.xyz),0,1);
    specular.w = clamp(dot(normal.xyz, H.xyz),0,1);
    specular.w = specular.w*specular.w;
    specular.w = specular.w*specular.w;
    specular.w = specular.w*specular.w;

    //JE: I think in the original shader we were multiplying gloss by L.y . L.y * 4 does that sound reasonable?  This looks right though...
    gloss.x = clamp(normal.w * glossFactor.w,0,1);
    outColor.xyz = c.xyz * (lightDiffuse.xyz * diffuse.w + lightAmbient.xyz);
    gl_FragColor.xyz = specular.w*gloss.x+outColor.xyz;
    gl_FragColor.w = texture1.w*constColor1.w;
}
