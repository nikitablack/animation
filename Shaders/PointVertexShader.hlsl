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

struct VS_INPUT
{
	float3 pos: POSITION;
};

struct VS_OUTPUT
{
	float3 pos: POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = input.pos;

	return output;
}