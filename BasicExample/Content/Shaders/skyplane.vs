/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 eye : TEXCOORD1;
	float3 light : TEXCOORD2;
};

cbuffer SkyBuffer
{
	float translation;
	float scale;
	float brightness;
	float padding;
	float3 eyep;
	float pad;
	float3 lightd;
};

PixelInputType vs_main(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, transpose(worldMatrix));
	output.eye = normalize(output.position.xyz - eyep.xyz);
    output.position = mul(output.position, transpose(viewMatrix));
    output.position = mul(output.position, transpose(projectionMatrix));
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
	//output.light = normalize(input.position.xyz - 
    return output;
}

/////////////
// GLOBALS //
/////////////
Texture2D cloudTexture : register(t0);
Texture2D perturbTexture : register(t1);
SamplerState SampleType;



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ps_main(PixelInputType input) : SV_TARGET
{
	float4 perturbValue;
	float4 cloudColor;

	// Translate the texture coordinate sampling location by the translation value.
	input.tex.x = input.tex.x + translation;//*2;//8;

	// Sample the texture value from the perturb texture using the translated texture coordinates.
	perturbValue = perturbTexture.Sample(SampleType, input.tex.xy);

	// Multiply the perturb value by the perturb scale.
	perturbValue = perturbValue * scale;

	// Add the texture coordinates as well as the translation value to get the perturbed texture coordinate sampling location.
	perturbValue.xy = perturbValue.xy + input.tex.xy + translation*2;//8;
	
	 // We are at height bumpScale.  March forward until we hit a hair or the 
    // base surface.  Instead of dropping down discrete y-voxels we should be
    // marching in texels and dropping our y-value accordingly (TODO: fix)
    /*float height = 1.0;

    // Number of height divisions
    float numSteps = 20;

    float2 offsetCoord = perturbValue.xy;//
	//offsetCoord = input.tex.xy;
    float4 NB = cloudTexture.Sample(SampleType, offsetCoord);

    float3 _tsE = mul(float3x3(float3(-1,0,0),
			     float3(0,-1,0),
			     float3(0,0,-1)), 
		input.eye);
   
	float n1 = perturbTexture.Sample(SampleType, input.tex.xy*1667+0.1).b;
	float n2 = perturbTexture.Sample(SampleType, input.tex.xy*1683+0.6).b;
	float n3 = perturbTexture.Sample(SampleType, input.tex.xy*1641+0.4).b;

	_tsE -= float3(n1*2-1, n2*2-1, n3*2-1)*0.2;
	_tsE = normalize(_tsE);
	
    // We have to negate tsE because we're walking away from the eye.
    //vec2 delta = vec2(-_tsE.x, _tsE.y) * bumpScale / (_tsE.z * numSteps);
    float step;
    float2 delta;

	float bumpScale = 0.055;//25;//125;//25;
    // Constant in z
    step = 1.0 / numSteps;
    delta = float2(-_tsE.x, _tsE.y) * bumpScale / (_tsE.z * numSteps);

        // Can also step along constant in xy; the results are essentially
        // the same in each case.
        // delta = 1.0 / (25.6 * numSteps) * vec2(-tsE.x, tsE.y);
        // step = tsE.z * bumpScale * (25.6 * numSteps) / (length(tsE.xy) * 400);
	//float amax = NB.r;
	float accum = 0.0;
    for (int i = 0; i < numSteps; i++)//while (NB.a < height) 
	{
		if (NB.r < height)
		{
			height -= step;
			offsetCoord += delta;
			NB = cloudTexture.Sample(SampleType, offsetCoord);
		} 
		else
		{
			//float delta1 = NB.r - height;
			//float delta2 = ( NB.r + step ) - amax;//fLastSampledHeight;

			//float ratio = delta1/(delta1+delta2);
			//delta/=2;
			//step/=2;
			//height += step*ratio;
			//offsetCoord = (ratio) * (offsetCoord-delta)vLastOffset + (1.0-ratio) * offsetCoord;
			//accum = 1;
			accum += (1.0f/numSteps)*(step/(1.0f/numSteps));
		}
    }
	//if (accum > 0)
		//accum += n1;
	
	float light = 1.3f;
	//trace for color
	float numLightSteps = 25;
	step = 1.0f/numSteps;
	
	_tsE = mul(float3x3(float3(0,0,1),
			     float3(1,0,0),
			     float3(0,-1,0)), 
		lightd);
	
	delta = float2(-_tsE.x, _tsE.y) * bumpScale / (_tsE.z * numLightSteps);
	
	//start tracing back towards the light
	for (int i = 0; i < numLightSteps; i++)//while (NB.a < height) 
	{
		if (NB.r > height)
		{
			height += step;
			offsetCoord -= delta;
			NB = cloudTexture.Sample(SampleType, offsetCoord);
		} 
		else
		{
			//float delta1 = NB.r - height;
			//float delta2 = ( NB.r + step ) - amax;//fLastSampledHeight;

			//float ratio = delta1/(delta1+delta2);
			
			//height += step*ratio;
			//offsetCoord = (ratio) * (offsetCoord-delta) + (1.0-ratio) * offsetCoord;
			//light -= 1.0f/numLightSteps;
		}
    }*/
	
	// Now sample the color from the cloud texture using the perturbed sampling coordinates.
	cloudColor = cloudTexture.Sample(SampleType, perturbValue.xy);

	// Reduce the color cloud by the brightness value.
	//cloudColor = cloudColor * 0.8;//brightness;

    //return cloudColor;
	//return float4(1,delta,1);//
	//return float4(light,light,light,accum);//*NB.r;
	//return float4(1,1,1,amax);
	return float4(1,1,1,cloudColor.r);
}