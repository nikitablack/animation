Texture2D matcapTexture;
SamplerState matcapSampler;

struct PS_INPUT
{
	float4 pos: SV_POSITION;
	float3 normal: NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 uv = normalize(input.normal).xy;
	uv.y *= -1;
	uv *= 0.5;
	uv += 0.5;
	return matcapTexture.Sample(matcapSampler, uv);
}