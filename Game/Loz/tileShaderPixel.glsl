/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D al_tex;
uniform sampler2D palTex;
varying vec4 varying_color;
varying vec2 varying_texcoord;


void main()
{
    vec4 indexVec = texture2D( al_tex, varying_texcoord );
    float index = indexVec.r;
    index += (0.5 / 16);

    float palette = varying_color.r + (0.5 / 16);

    // Keep in mind that 16x16 seems to be the smallest allowed texture
    vec2 palPos = vec2( index, palette );

    // Right now, the only significant alpha is the one in the palette texture.
    // You can make this shader also consider other alpha:
    // - alpha in the source texture (al_tex)
    // - alpha from the drawing call (palette/tint color, varying_color)

    vec4 color = texture2D( palTex, palPos );

    gl_FragColor = color;
}
