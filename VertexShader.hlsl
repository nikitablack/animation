cbuffer ConstantBufferPerFrame : register(b0)
{
	float4x4 viewMat;
	float4x4 projMat;
	float timePassed;
};

cbuffer ConstantBufferPerObj : register(b1)
{
	float4x4 wvpMat;
};

struct VS_INPUT
{
	float3 pos: POSITION;
	float3 normal: NORMAL;
	float4 color: COLOR;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float3 normal: NORMAL;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), wvpMat);
	output.color = input.color;
	output.normal = input.normal;
	return output;
}