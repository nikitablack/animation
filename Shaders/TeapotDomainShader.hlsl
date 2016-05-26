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

struct DS_OUTPUT
{
	float4 vPosition  : SV_POSITION;
};

// Output control point
struct DS_CONTROL_POINT_INPUT
{
	float3 vPosition : WORLDPOS; 
};

// Output patch constant data.
struct DS_CONSTANT_DATA_INPUT
{
	float EdgeTessFactor[4] : SV_TessFactor;
	float InsideTessFactor[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 16

float4 BernsteinBasis(float t)
{
	float invT = 1.0f - t;
	return float4(invT * invT * invT, // (1-t)3
		3.0f * t * invT * invT, // 3t(1-t)2
		3.0f * t * t * invT, // 3t2(1-t)
		t * t * t); // t3
}

float4 dBernsteinBasis(float t)
{
	float invT = 1.0f - t;
	return float4(-3 * invT * invT, // -3(1-t)2
		3 * invT * invT - 6 * t * invT, // 3(1-t)-6t(1-t)
		6 * t * invT - 3 * t * t, // 6t(1-t) – 3t2
		3 * t * t); // 3t2
}

float3 EvaluateBezier(const OutputPatch<DS_CONTROL_POINT_INPUT, NUM_CONTROL_POINTS> bezpatch, float4 BasisU, float4 BasisV)
{
	// This function essentially does this: Value(u,v) = S S Bm(u) Bn(v) G
	float3 Value = float3(0, 0, 0);
	Value = BasisV.x * (bezpatch[0].vPosition * BasisU.x +
		bezpatch[1].vPosition * BasisU.y + bezpatch[2].vPosition *
		BasisU.z + bezpatch[3].vPosition * BasisU.w);
	Value += BasisV.y * (bezpatch[4].vPosition * BasisU.x +
		bezpatch[5].vPosition * BasisU.y + bezpatch[6].vPosition *
		BasisU.z + bezpatch[7].vPosition * BasisU.w);
	Value += BasisV.z * (bezpatch[8].vPosition * BasisU.x +
		bezpatch[9].vPosition * BasisU.y + bezpatch[10].vPosition *
		BasisU.z + bezpatch[11].vPosition * BasisU.w);
	Value += BasisV.w * (bezpatch[12].vPosition * BasisU.x +
		bezpatch[13].vPosition * BasisU.y + bezpatch[14].vPosition *
		BasisU.z + bezpatch[15].vPosition * BasisU.w);
	return Value;
}

[domain("quad")]
DS_OUTPUT main(DS_CONSTANT_DATA_INPUT input, float2 domain : SV_DomainLocation, const OutputPatch<DS_CONTROL_POINT_INPUT, NUM_CONTROL_POINTS> patch)
{
	/*DS_OUTPUT Output;

	float3 topMidpoint = lerp(patch[0].vPosition, patch[3].vPosition, domain.x);
	float3 bottomMidpoint = lerp(patch[12].vPosition, patch[15].vPosition, domain.x);
	float4 pos = float4(lerp(topMidpoint, bottomMidpoint, domain.y), 1);

	//float4 pos = float4(patch[0].vPosition * domain.x + patch[1].vPosition * domain.y + patch[2].vPosition.z, 1);
	Output.vPosition = mul(pos, wvpMat);*/

	// Evaluate the basis functions at (u, v)
	float4 BasisU = BernsteinBasis(domain.x);
	float4 BasisV = BernsteinBasis(domain.y);
	float4 dBasisU = dBernsteinBasis(domain.x);
	float4 dBasisV = dBernsteinBasis(domain.y);
	// Evaluate the surface position for this vertex
	float3 WorldPos = EvaluateBezier(patch, BasisU, BasisV);
	// Evaluate the tangent space for this vertex (using derivatives)
	float3 Tangent = EvaluateBezier(patch, dBasisU, BasisV);
	float3 BiTangent = EvaluateBezier(patch, BasisU, dBasisV);
	float3 Norm = normalize(cross(Tangent, BiTangent));

	DS_OUTPUT Output;
	Output.vPosition = mul(float4(WorldPos, 1.0f), wvpMat);
	//Output.vWorldPos = WorldPos;
	//Output.vNormal = Norm;
	return Output;
}