#version 110

uniform vec4 constColor1; // env 0
uniform vec4 constColor2; // env 1
uniform sampler2D myTextureMap;
uniform sampler2D myTextureMap2;

void main() {
// Variable declarations
vec4 characterColor;
vec4 colorA;
vec4 colorB;

// Do texture fetches
vec4 texture0 = texture2D( myTextureMap, gl_TexCoord[0].st );
vec4 texture1 = texture2D( myTextureMap2, gl_TexCoord[1].st );
// Compute rgb
characterColor = mix(constColor1, constColor2,texture1.x); // Blend between the two character colors using texture1
colorA = 4.0* gl_Color * texture0;
colorB.xyz = 4.0 *  gl_Color.xyz * colorA.xyz * characterColor.xyz;
gl_FragColor.xyz = mix(colorA.xyz,colorB.xyz, texture0.w).xyz; // Blend between colors using texture0's alpha


// Compute alpha
gl_FragColor.w = constColor1.w * texture1.w;

// TODO add fog

}
