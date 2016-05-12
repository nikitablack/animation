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
};

struct VS_INPUT
{
	float3 pos: POSITION;
	float3 normal: NORMAL;
	uint colorId: COLOR_ID;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float3 normal: NORMAL;
};

static float4 colors[2] = {
	float4(0.8f, 0.8f, 0.8f, 1.0f),
	float4(0.5f, 0.5f, 0.5f, 1.0f)
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), wvpMat);
	output.color = checkboardColors[input.colorId];
	output.normal = input.normal;
	return output;
}