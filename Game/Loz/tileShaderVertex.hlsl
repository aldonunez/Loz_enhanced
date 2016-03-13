/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

struct VS_INPUT
{
    float4 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

float4x4 al_projview_matrix;

VS_OUTPUT vs_main( VS_INPUT input ) 
{
    VS_OUTPUT output;
    output.Position = mul( input.Position, al_projview_matrix );
    output.Color = input.Color;
    output.Texcoord = input.Texcoord;
    return output;
}
