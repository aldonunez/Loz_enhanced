/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

texture al_tex;
texture palTex;

sampler2D al_texSampler = sampler_state
{
    Texture = <al_tex>;
};

sampler2D palTexSampler = sampler_state
{
    Texture = <palTex>;
};

float4 ps_main( VS_OUTPUT input ) : COLOR0
{
    float4 indexVec = tex2D( al_texSampler, input.Texcoord );
    float  index = indexVec.r + (0.5 / 16);

    float  palette = input.Color.r + (0.5 / 16);
    float2 palCoord = float2( index, palette );
    float4 color = tex2D( palTexSampler, palCoord );

    return color;
}
