// Vertex shader input structure
struct VS_INPUT
{
    float4 Position   : POSITION;//all we need to making shadows
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

VS_OUTPUT vs_main( in VS_INPUT In )
{
	VS_OUTPUT Out; 
	
    Out.Position = mul(float4(In.Position.xyz,1), shadowvp);
	//Out.Depth = Out.Position.z;
	//Out.Depth.y = Out.Depth.x*Out.Depth.x;//for variance
    return Out; //return output vertex
}

float4 ps_main( in VS_OUTPUT In) : SV_Target
{
	discard;
	return float4(1,1,1,1);
	//return float4(In.Depth,0,0,1);
}




