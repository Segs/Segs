#version 110
uniform vec4 constColor1;
uniform vec4 constColor2;
uniform vec4 lightsOn;

uniform sampler2D myTextureMap;
uniform sampler2D myTextureMap2;
void main()
{
    vec4 texclr0 = texture2D( myTextureMap, gl_TexCoord[0].st );
    vec4 texclr1 = texture2D( myTextureMap2, gl_TexCoord[1].st );
    vec4 colorResult;
    colorResult.xyz = mix(constColor1,texclr0,texclr0.w).xyz*4.0*gl_Color.xyz;
    colorResult.w = constColor1.w;
    //todo: fog processing ?
    colorResult.xyz = colorResult.xyz + texclr1.xyz * lightsOn.x;
    gl_FragColor = colorResult;
}
