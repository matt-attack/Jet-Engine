// Vertex shader input structure
struct VS_INPUT
{
    float4 Position    : POSITION;
	uint4 boneWeights  : BLENDWEIGHT;
	uint4 boneIndices  : BLENDINDICES;
};

struct VS_OUTPUT
{
    float4 Position   : SV_Position;
    //float Depth       : TEXCOORD0;
};

// Global variables
cbuffer WVP
{
	float4x4 shadowvp;
}

cbuffer Skinning
{
	float4x3 SkinMats[70];
}


VS_OUTPUT vs_main( in VS_INPUT In )
{
	VS_OUTPUT Out; 

	float4x3 m = SkinMats[int(In.boneIndices.x)] * In.boneWeights.x/255;
	m += SkinMats[int(In.boneIndices.y)] * In.boneWeights.y/255;
	m += SkinMats[int(In.boneIndices.z)] * In.boneWeights.z/255;
	m += SkinMats[int(In.boneIndices.w)] * In.boneWeights.w/255;
	
	float4 mpos = float4(mul(In.Position,m), In.Position.w);
	Out.Position = mul(mpos, shadowvp);

    //Out.Position = mul(float4(In.Position.xyz,1), shadowvp);
	//Out.Depth = Out.Position.z;
	//Out.Depth.y = Out.Depth.x*Out.Depth.x;
    return Out; //return output vertex
}

float4 ps_main( in VS_OUTPUT In) : SV_Target
{
	discard;
	return float4(1,1,1,1);
	//return float4(In.Depth,0,0,1);
}




