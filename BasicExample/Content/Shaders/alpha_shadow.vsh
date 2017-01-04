// Vertex shader input structure
struct VS_INPUT
{
    float4 Position   : POSITION;//all we need to make shadows
	float2 Texture   : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position   : SV_Position;
	float2 Texture : TEXCOORD0;
    //float Depth       : TEXCOORD0;
};

// Global variables
cbuffer WVP
{
	float4x4 shadowvp;
}

VS_OUTPUT vs_main( in VS_INPUT In )
{
	VS_OUTPUT Out; 
	
    Out.Position = mul(float4(In.Position.xyz,1), shadowvp);
	Out.Texture = In.Texture;
	//Out.Depth = Out.Position.z;
	//Out.Depth.y = Out.Depth.x*Out.Depth.x;//for variance
    return Out; //return output vertex
}

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0) = sampler_state
{
    Texture = (Texture);
};


float4 ps_main( in VS_OUTPUT In) : SV_Target
{
	//discard;
	//if (cos(In.Texture.y) > 0.5)//In.TexCoord.y < 0.5)//
	//	discard;
	if (Texture.Sample(TextureSampler, In.Texture).a < 0.1)
		discard;
	return float4(1,1,1,1);
	//return float4(In.Depth,0,0,1);
}




