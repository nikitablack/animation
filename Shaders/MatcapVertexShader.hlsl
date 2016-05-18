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
	float3 normal: NORMAL;
	int3 bones: BONE;
	float3 bonesWeights: BONE_WEIGHT;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float3 normal: NORMAL;
	float2 uv: UV;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), wvpMat);

	float4x4 wvMat = mul(wMat, viewMat);
	float3 e = -normalize(mul(float4(input.pos, 1.0f), wvMat)).xyz;
	float3 n = normalize(mul(float4(input.normal, 0.0f), wvMat)).xyz;

	float3 r = reflect(e, n);
	float m = 2.0f * sqrt(pow(r.x, 2.0f) + pow(r.y, 2.0f) + pow(r.z + 1.0f, 2.0f));
	output.normal = mul(float4(input.normal, 0.0f), wvMat).xyz;
	output.uv = r.xy / m + 0.5f;

	/*float4 nrm = mul(float4(input.normal, 0.0f), wMat);
	output.normal = mul(nrm, viewMat).xyz;*/
	return output;
}