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
	row_major float4x4 wvpMat;
	row_major float4x4 wMat;
};

struct VS_INPUT
{
	float3 pos: POSITION;
};

struct VS_OUTPUT
{
	float4 pos: WORLDPOS;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1.0f);// mul(float4(input.pos, 1.0f), wMat);
	return output;
}