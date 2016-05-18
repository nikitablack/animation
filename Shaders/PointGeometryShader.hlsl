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

struct GSInput
{
	float3 pos: POSITION;
};

struct GSOutput
{
	float4 pos: SV_POSITION;
};

static const float4 g_positions[4] =
{
	float4(-1.0f, -1.0f, 0, 0),
	float4(1.0f, -1.0f, 0, 0),
	float4(-1.0f, 1.0f, 0, 0),
	float4(1.0f, 1.0f, 0, 0),
};

[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> outputStream)
{
	//float4x4 wvMat = mul(wMat, viewMat);
	float4 viewPos = mul(float4(input[0].pos, 1.0f), wvpMat);
	viewPos /= viewPos.w;

	float ratio = 1280.0f / 1024.0f;
	GSOutput output;
	for (int i = 0; i < 4; i++)
	{
		float4 offset = g_positions[i];
		offset *= 0.005;
		offset.y *= ratio;
		output.pos = viewPos + offset;
		outputStream.Append(output);
	}

	outputStream.RestartStrip();
}