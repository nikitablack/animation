cbuffer ConstantBufferImmutable : register(b0)
{
	float4 checkboardColors[2];
};

cbuffer ConstantBufferProjectionMatrix : register(b1)
{
	float4x4 projMat;
};

cbuffer ConstantBufferPerFrame : register(b2)
{
	float4x4 viewMat;
	float timePassed;
};

cbuffer ConstantBufferPerObj : register(b3)
{
	float4x4 wvpMat;
	float4x4 wMat;
};

struct HS_CONTROL_POINT_INPUT
{
	float3 vPosition : WORLDPOS;
};

struct HS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS; 
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4] : SV_TessFactor;
	float InsideTessFactor[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 16

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<HS_CONTROL_POINT_INPUT, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	float factor = 8.0f;

	Output.EdgeTessFactor[0] = factor;
	Output.EdgeTessFactor[1] = factor;
	Output.EdgeTessFactor[2] = factor;
	Output.EdgeTessFactor[3] = factor;
	Output.InsideTessFactor[0] = factor;
	Output.InsideTessFactor[1] = factor;

	return Output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(InputPatch<HS_CONTROL_POINT_INPUT, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.vPosition = ip[i].vPosition;

	return Output;
}
