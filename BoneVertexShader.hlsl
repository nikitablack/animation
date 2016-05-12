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
};

float4 main(VS_INPUT input) : SV_POSITION
{
	return mul(float4(input.pos, 1.0f), wvpMat);
}