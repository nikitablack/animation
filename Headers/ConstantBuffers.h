#pragma once

struct ConstantBufferImmutable
{
	XMFLOAT4 checkboardColors[2];
};

struct ConstantBufferProjectionMatrix
{
	XMFLOAT4X4 projMat;
};

struct ConstantBufferPerFrame
{
	XMFLOAT4X4 viewMat;
	float timePassed;
	char padding[12];
};

struct ConstantBufferPerObject
{
	XMFLOAT4X4 wvpMat;
	XMFLOAT4X4 wMat;
};