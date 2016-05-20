#include "Headers\Window.h"
#include "Headers\Graphics.h"
#include "Headers\Mesh.h"
#include "Headers\ShaderFactory.h"
#include "Headers\Object.h"
#include <Windows.h>
#include <dxgi1_2.h>
#include "Headers\GeometryUtils.h"
#include "Headers\MeshAsciiParser.h"
#include "Headers\DDSTextureLoader.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

const float CAMERA_STEP{ 0.1f };
float cameraX{ 0.0f };
float cameraY{ 1.0f };

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

template<typename T>
void updateResource(ID3D11DeviceContext* context, ID3D11Resource* resource, const T& data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	T* cbPerObjDataPtr{ static_cast<T*>(mappedResource.pData) };
	*cbPerObjDataPtr = data;

	context->Unmap(resource, 0);
}

void renderPlane(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffers[4], shared_ptr<Object> plane, shared_ptr<ShaderDataCollection> planeShaderDataCollection)
{
	const vector<ID3D11Buffer*>& vertexBuffers{ plane->getVertexBuffers() };

	context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), plane->getVertexStrides().data(), plane->getVertexOffsets().data());
	context->IASetIndexBuffer(plane->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->IASetInputLayout(planeShaderDataCollection->vertexShaderData->getInputLayout());
	context->VSSetShader(planeShaderDataCollection->vertexShaderData->getShader(), nullptr, 0);
	context->PSSetShader(planeShaderDataCollection->pixelShaderData->getShader(), nullptr, 0);
	//context->RSSetState(obj->getRasterizerState());

	XMVECTOR vecCamPosition(XMVectorSet(cameraX, cameraY, -5.0f, 0.0f));
	XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
	XMVECTOR vecCamUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX matrixView(XMMatrixLookAtLH(vecCamPosition, vecCamLookAt, vecCamUp));
	XMMATRIX matrixProjection(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));

	static float rot{ 0.0f };
	plane->setRotation(0, rot, 0);
	rot += 0.01f;
	XMMATRIX matrixObj(XMLoadFloat4x4(&plane->getTransformGlobal()));

	ConstantBufferPerObject cbPerObject;
	XMStoreFloat4x4(&cbPerObject.wMat, XMMatrixTranspose(matrixObj));
	XMStoreFloat4x4(&cbPerObject.wvpMat, XMMatrixTranspose(matrixObj * matrixView * matrixProjection));

	updateResource(context, constantBuffers[3], cbPerObject);

	context->DrawIndexed(plane->getNumIndices(), 0, 0);
}

void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, IDXGISwapChain* swapChain, D3D11_VIEWPORT* viewport,
	ID3D11Buffer* constantBuffers[4], shared_ptr<Object> plane, shared_ptr<ShaderDataCollection> planeShaderDataCollection)
{
	static const FLOAT clearColor[]{ 0.4f, 0.4f, 0.8f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetViewports(1, viewport);

	/////////////////////////// constant buffer per frame /////////////////
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(constantBuffers[2], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	ConstantBufferPerFrame* cbPerFrameDataPtr{ static_cast<ConstantBufferPerFrame*>(mappedResource.pData) };

	XMVECTOR vecCamPosition(XMVectorSet(cameraX, cameraY, -5.0f, 0.0f));
	XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
	XMVECTOR vecCamUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX matrixView(XMMatrixLookAtLH(vecCamPosition, vecCamLookAt, vecCamUp));
	XMMATRIX matrixProjection(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));

	ConstantBufferPerFrame cbPerFrame;
	XMStoreFloat4x4(&cbPerFrame.viewMat, XMMatrixTranspose(matrixView));
	cbPerFrame.timePassed = 0;

	cbPerFrameDataPtr->viewMat = cbPerFrame.viewMat;
	cbPerFrameDataPtr->timePassed = cbPerFrame.timePassed;

	context->Unmap(constantBuffers[2], 0);
	////////////////////////////////////////////////////////////////////////

	context->VSSetConstantBuffers(0, 4, constantBuffers);

	renderPlane(context, constantBuffers, plane, planeShaderDataCollection);

	swapChain->Present(0, 0);
}

/*void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, IDXGISwapChain* swapChain, D3D11_VIEWPORT* viewport,
	ID3D11Buffer* constantBuffers[4], vector<shared_ptr<Object3D>> objects, ID3D11ShaderResourceView* matcapView, ID3D11SamplerState* matcapSampler, ID3D11DepthStencilState* depthStencilState,
	shared_ptr<Object3D> kitana, shared_ptr<Object3D> kitanaSkeleton, ID3D11DepthStencilState* depthStencilState2,
	vector<XMFLOAT4X4>& bonesInverseMatrices, vector<shared_ptr<Object3D>>& bonesObj, ID3D11ShaderResourceView* bonesInverseBufferView, ID3D11ShaderResourceView* bonesCurrBufferView, ID3D11Buffer* bonesInverseBuffer, ID3D11Buffer* bonesCurrBuffer)
{
	static const FLOAT clearColor[]{ 0.4f, 0.4f, 0.8f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetViewports(1, viewport);

	/////////////////////////// constant buffer per frame /////////////////
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(constantBuffers[2], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	ConstantBufferPerFrame* cbPerFrameDataPtr{ static_cast<ConstantBufferPerFrame*>(mappedResource.pData) };

	XMVECTOR vecCamPosition(XMVectorSet(cameraX, cameraY, -5.0f, 0.0f));
	XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
	XMVECTOR vecCamUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX matrixView(XMMatrixLookAtLH(vecCamPosition, vecCamLookAt, vecCamUp));
	XMMATRIX matrixProjection(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));

	ConstantBufferPerFrame cbPerFrame;
	XMStoreFloat4x4(&cbPerFrame.viewMat, XMMatrixTranspose(matrixView));
	cbPerFrame.timePassed = 0;

	cbPerFrameDataPtr->viewMat = cbPerFrame.viewMat;
	cbPerFrameDataPtr->timePassed = cbPerFrame.timePassed;

	context->Unmap(constantBuffers[2], 0);
	////////////////////////////////////////////////////////////////////////

	context->VSSetConstantBuffers(0, 4, constantBuffers);
	context->GSSetConstantBuffers(0, 4, constantBuffers);

	context->PSSetShaderResources(0, 1, &matcapView);
	context->PSSetSamplers(0, 1, &matcapSampler);
	context->OMSetDepthStencilState(depthStencilState, 1);

	static float rot{ XM_PIDIV2 };
	kitana->setRotation(0, rot, 0);
	//rot += 0.01f;

	for (shared_ptr<Object3D>& obj : objects)
	{
		const vector<ID3D11Buffer*>& vertexBuffers{ obj->getVertexBuffers() };

		context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), obj->getVertexStrides().data(), obj->getVertexOffsets().data());

		context->IASetIndexBuffer(obj->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->IASetInputLayout(obj->getInputLayout());
		context->VSSetShader(obj->getVertexShader(), nullptr, 0);
		context->PSSetShader(obj->getPixelShader(), nullptr, 0);
		context->GSSetShader(nullptr, nullptr, 0);
		context->RSSetState(obj->getRasterizerState());

		/////////////////////////// constant buffer per object /////////////////
		if (FAILED(context->Map(constantBuffers[3], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw(runtime_error{ "Error mapping constant buffer" });
		}

		ConstantBufferPerObject* cbPerObjDataPtr{ static_cast<ConstantBufferPerObject*>(mappedResource.pData) };

		//obj->setRotation(0, XM_PI, 0);
		obj->setRotation(0, 0, 0);
		XMMATRIX matrixObj(XMLoadFloat4x4(&obj->getTransformGlobal()));

		ConstantBufferPerObject cbPerObject;
		XMStoreFloat4x4(&cbPerObject.wvpMat, XMMatrixTranspose(matrixObj * matrixView * matrixProjection));

		XMFLOAT4X4 tmp;
		XMStoreFloat4x4(&tmp, XMMatrixTranspose(XMLoadFloat4x4(&obj->getTransformGlobal())));
		cbPerObjDataPtr->wMat = tmp;
		cbPerObjDataPtr->wvpMat = cbPerObject.wvpMat;

		context->Unmap(constantBuffers[3], 0);
		////////////////////////////////////////////////////////////////////////

		/////////////////////////// constant buffer per object /////////////////
		if (FAILED(context->Map(bonesCurrBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw(runtime_error{ "Error mapping shader resource" });
		}

		XMFLOAT4X4* boneDataPtr{ static_cast<XMFLOAT4X4*>(mappedResource.pData) };
		XMFLOAT4X4 boneDataPtrTmp[212];

		for (int i{ 0 }; i < bonesObj.size(); ++i)
		{
			XMFLOAT4X4 tmp{ bonesObj[i]->getTransformGlobal() };
			XMMATRIX mat(XMLoadFloat4x4(&tmp));
			
			XMStoreFloat4x4(&boneDataPtrTmp[i], XMMatrixTranspose(mat));
		}

		memcpy(boneDataPtr, &boneDataPtrTmp, sizeof(XMFLOAT4X4) * 212);

		context->Unmap(bonesCurrBuffer, 0);
		////////////////////////////////////////////////////////////////////////

		ID3D11ShaderResourceView* srvs[2]{ bonesInverseBufferView, bonesCurrBufferView };
		context->VSSetShaderResources(0, 2, srvs);

		context->DrawIndexed(obj->getNumIndices(), 0, 0);
	}




	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->OMSetDepthStencilState(depthStencilState2, 1);
	const vector<ID3D11Buffer*>& vertexBuffers{ kitanaSkeleton->getVertexBuffers() };
	context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), kitanaSkeleton->getVertexStrides().data(), kitanaSkeleton->getVertexOffsets().data());
	context->IASetInputLayout(kitanaSkeleton->getInputLayout());
	context->VSSetShader(kitanaSkeleton->getVertexShader(), nullptr, 0);
	context->PSSetShader(kitanaSkeleton->getPixelShader(), nullptr, 0);
	context->GSSetShader(kitanaSkeleton->getGeometryShader(), nullptr, 0);
	context->RSSetState(kitanaSkeleton->getRasterizerState());

	/////////////////////////// constant buffer per object /////////////////
	if (FAILED(context->Map(constantBuffers[3], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	ConstantBufferPerObject* cbPerObjDataPtr{ static_cast<ConstantBufferPerObject*>(mappedResource.pData) };

	XMMATRIX matrixObj(XMLoadFloat4x4(&kitanaSkeleton->getTransformGlobal()));

	ConstantBufferPerObject cbPerObject;
	XMStoreFloat4x4(&cbPerObject.wvpMat, XMMatrixTranspose(matrixObj * matrixView * matrixProjection));

	XMFLOAT4X4 tmp;
	XMStoreFloat4x4(&tmp, XMMatrixTranspose(XMLoadFloat4x4(&kitanaSkeleton->getTransformGlobal())));
	cbPerObjDataPtr->wMat = tmp;
	cbPerObjDataPtr->wvpMat = cbPerObject.wvpMat;

	context->Unmap(constantBuffers[3], 0);
	////////////////////////////////////////////////////////////////////////

	context->Draw(14, 0);






	swapChain->Present(0, 0);
}*/

shared_ptr<Object> createPlaneData(BufferFactory& bufferFactory)
{
	CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 20, 20) };

	BufferDataCollection planeBufferDataCollection;
	planeBufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(checkboardPlaneMesh.positions));
	planeBufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(checkboardPlaneMesh.normals));
	planeBufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(checkboardPlaneMesh.colorIds));
	planeBufferDataCollection.indexBufferData = bufferFactory.createIndexBuffer(checkboardPlaneMesh.indices);

	shared_ptr<Mesh> planeMesh{ make_shared<Mesh>(planeBufferDataCollection) };

	return make_shared<Object>(planeMesh);
}

shared_ptr<ShaderDataCollection> createPlaneShaders(ShaderFactory& shaderFactory)
{
	shared_ptr<ShaderDataCollection> planeShaderDataCollection{ make_shared<ShaderDataCollection>() };
	planeShaderDataCollection->vertexShaderData = shaderFactory.createVertexShader(L"PlaneVertexShader.cso");
	planeShaderDataCollection->pixelShaderData = shaderFactory.createPixelShader(L"PlanePixelShader.cso");

	return planeShaderDataCollection;
}

vector<shared_ptr<Object>> createKitanaData(BufferFactory& bufferFactory, ComPtr<ID3D11Device> device, shared_ptr<Object> kitanaObj, vector<shared_ptr<Object>>& kitanaSkeleton, vector<XMFLOAT4X4>& bonesInverseMatrices, vector<shared_ptr<Object>>& bonesObj)
{
	vector<shared_ptr<Object>> kitana;

	vector<Bone> bones;
	vector<vector<Vertex>> meshes;
	vector<vector<uint32_t>> faces;
	MeshAsciiParser::read("kitana", bones, meshes, faces);

	vector<XMFLOAT3> bonesPositions;
	vector<int32_t> bonesParents;
	vector<uint32_t> bonesIndices;

	/*for (Bone& bone : bones)
	{
		XMFLOAT4X4 mat;
		bonesInverseMatrices.push_back(
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-bone.pos.x, -bone.pos.y, -bone.pos.z, 1.0f
		});

		shared_ptr<Object3D> obj{ make_shared<Object3D>() };
		bonesObj.push_back(obj);

		if (bone.parent != -1)
		{
			shared_ptr<Object3D> par{ bonesObj[bone.parent] };

			XMFLOAT3 tmp1{ par->getPosition() };
			XMFLOAT3 tmp2;
			XMStoreFloat3(&tmp2, XMLoadFloat3(&bone.pos) - XMLoadFloat3(&tmp1));

			obj->setPosition(tmp2);
			par->addChild(obj);
		}
	}*/




	/*static vector<string> bonesToUse{ "root hips", "pelvis", "leg left thigh", "leg left knee", "leg left ankle", "leg right thigh", "leg right knee", "leg right ankle", "spine 1", "spine 2", "spine 3", "spine 4", "head neck lower", "head neck upper" };

	for (string& boneName : bonesToUse)
	{
		bool found{ false };

		for (Bone& bone : bones)
		{
			if (bone.name == boneName)
			{
				bonesPositions.push_back(bone.pos);
				bonesParents.push_back(bone.parent);
				found = true;

				shared_ptr<Object3D> boneObj;

				break;
			}
		}

		if (!found)
		{
			throw runtime_error("wrong bone name");
		}
	}*/

	/*pair<vector<XMFLOAT3>, string> p{ bonesPositions, "POSITION" };
	auto t = make_tuple(p);
	shared_ptr<VertexBufferData<XMFLOAT3>> vertexBufferData{ make_shared<VertexBufferData<XMFLOAT3>>(device, t) };
	shared_ptr<Mesh<XMFLOAT3>> mesh{ make_shared<Mesh<XMFLOAT3>>(device, vertexBufferData, vector<uint32_t>{ 0, 1, 2 }) };
	shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData{ make_shared<ShaderData<ID3D11VertexShader>>(device, L"PointVertexShader.cso") };
	shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData{ make_shared<ShaderData<ID3D11PixelShader>>(device, L"PointPixelShader.cso") };
	shared_ptr<ShaderData<ID3D11GeometryShader>> geometryShaderData{ make_shared<ShaderData<ID3D11GeometryShader>>(device, L"PointGeometryShader.cso") };
	shared_ptr<Object3D> skeleton{ make_shared<Object3D>(device, mesh, vertexShaderData, pixelShaderData, geometryShaderData) };
	kitanaSkeleton.push_back(skeleton);
	kitanaObj->addChild(skeleton);*/

	for (int i{ 0 }; i < meshes.size(); ++i)
	{
		vector<Vertex> meshData{ meshes[i] };
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT3> normals;
		vector<XMINT3> bones;
		vector<XMFLOAT3> bonesWeights;
		for (auto& vertex : meshData)
		{
			vertices.push_back(vertex.pos);
			normals.push_back(vertex.normal);
			bones.push_back(vertex.bones);
			bonesWeights.push_back(vertex.bonesWeights);
		}

		vector<uint32_t> face{ faces[i] };
		vector<uint32_t> indices;
		for (uint32_t ind : face)
		{
			indices.push_back(ind);
		}

		/*pair<vector<XMFLOAT3>, string> p1{ vertices, "POSITION" };
		pair<vector<XMFLOAT3>, string> p2{ normals, "NORMAL" };
		pair<vector<XMINT3>, string> p3{ bones, "BONE" };
		pair<vector<XMFLOAT3>, string> p4{ bonesWeights, "BONE_WEIGHT" };
		auto t = make_tuple(p1, p2, p3, p4);
		shared_ptr<VertexBufferData<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>> vertexBufferData{ make_shared<VertexBufferData<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>>(device, t) };
		shared_ptr<Mesh<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>> mesh{ make_shared<Mesh<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>>(device, vertexBufferData, indices) };
		shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData{ make_shared<ShaderData<ID3D11VertexShader>>(device, L"MatcapVertexShader.cso") };
		shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData{ make_shared<ShaderData<ID3D11PixelShader>>(device, L"MatcapPixelShader.cso") };
		shared_ptr<Object3D> obj{ make_shared<Object3D>(device, mesh, vertexShaderData, pixelShaderData, nullptr) };

		if (i == 6 || i == 16 || i == 17 || i == 18 || i == 21 || i == 23 || i == 24 || i == 25 || i == 27)
		{
			kitana.push_back(obj);
			kitanaObj->addChild(obj);
		}*/




		CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 20, 20) };

		BufferDataCollection bufferDataCollection;
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(vertices));
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(normals));
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(bones));
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory.createVertexBuffer(bonesWeights));
		bufferDataCollection.indexBufferData = bufferFactory.createIndexBuffer(indices);

		shared_ptr<Mesh> mesh{ make_shared<Mesh>(bufferDataCollection) };

		return make_shared<Object>(mesh);
	}

	return kitana;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//MeshAsciiParser::parse("Kitana.mesh.ascii");
	//return 0;

	const LONG width{ 1280 };
	const LONG height{ 1024 };
	const UINT bufferCount{ 2 };

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	shared_ptr<Window> window;
	shared_ptr<Graphics> graphics;

	ComPtr<ID3D11Texture2D> matcapTexture;
	ComPtr<ID3D11ShaderResourceView> matcapView;
	ComPtr<ID3D11SamplerState> matcapSampler;
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	ComPtr<ID3D11DepthStencilState> depthStencilState2;

	std::shared_ptr<BufferDataBase> constBufferImmutable;
	std::shared_ptr<BufferDataBase> constBufferProjectionMatrix;
	std::shared_ptr<BufferDataBase> constBufferPerFrame;
	std::shared_ptr<BufferDataBase> constBufferPerObject;
	ComPtr<ID3D11Buffer> bonesInverseBuffer;
	ComPtr<ID3D11Buffer> bonesCurrBuffer;
	ComPtr<ID3D11ShaderResourceView> bonesInverseBufferView;
	ComPtr<ID3D11ShaderResourceView> bonesCurrBufferView;

	shared_ptr<Object> plane;
	shared_ptr<ShaderDataCollection> planeShaderDataCollection;
	/*shared_ptr<Object3D> kitana = make_shared<Object3D>();
	vector<shared_ptr<Object3D>> objects;
	vector<shared_ptr<Object3D>> kitanaSkeleton;
	vector<XMFLOAT4X4> bonesInverseMatrices;
	vector<shared_ptr<Object3D>> bonesObj;*/

	try
	{
		window = make_shared<Window>(width, height);
		graphics = make_shared<Graphics>(window->getHandle(), bufferCount);

		BufferFactory bufferFactory{ graphics->getDeviceCom() };
		ShaderFactory shaderFactory{ graphics->getDeviceCom() };

		ConstantBufferImmutable cbImmutable{ { XMFLOAT4{ 0.8f, 0.8f, 0.8f, 1.0f }, XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f } } };
		ConstantBufferProjectionMatrix cbProjectionMatrix;
		XMStoreFloat4x4(&cbProjectionMatrix.projMat, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), width / height, 1.0f, 100.0f)));

		CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 20, 20) };
		BoneArmatureMesh boneArmatureMesh{ GeometryGenerator::generateBone(0.5f) };

		constBufferImmutable = bufferFactory.createConstantBuffer(cbImmutable);
		constBufferProjectionMatrix = bufferFactory.createConstantBuffer<ConstantBufferProjectionMatrix>();
		constBufferPerFrame = bufferFactory.createConstantBuffer<ConstantBufferPerFrame>();
		constBufferPerObject = bufferFactory.createConstantBuffer<ConstantBufferPerObject>();

		/*auto bonesInverseBufferAndView = createStructuredBufferAndView(graphics->getDevice(), { XMFLOAT4X4{} }, false);
		bonesInverseBuffer = bonesInverseBufferAndView.first;
		bonesInverseBufferView = bonesInverseBufferAndView.second;
		auto bonesCurrBufferAndView = createStructuredBufferAndView(graphics->getDevice(), { XMFLOAT4X4{} }, true);
		bonesCurrBuffer = bonesCurrBufferAndView.first;
		bonesCurrBufferView = bonesCurrBufferAndView.second;*/

		HRESULT hr = CreateDDSTextureFromFile(graphics->getDevice(), L"textures/matcap2.dds", (ID3D11Resource**)matcapTexture.ReleaseAndGetAddressOf(), matcapView.ReleaseAndGetAddressOf());
		
		D3D11_SAMPLER_DESC matcapSamplerDesc;
		ZeroMemory(&matcapSamplerDesc, sizeof(matcapSamplerDesc));
		matcapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		matcapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		matcapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		matcapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		matcapSamplerDesc.MipLODBias = 0.0f;
		matcapSamplerDesc.MaxAnisotropy = 1;
		matcapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		matcapSamplerDesc.BorderColor[0] = 0;
		matcapSamplerDesc.BorderColor[1] = 0;
		matcapSamplerDesc.BorderColor[2] = 0;
		matcapSamplerDesc.BorderColor[3] = 0;
		matcapSamplerDesc.MinLOD = 0;
		matcapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = graphics->getDevice()->CreateSamplerState(&matcapSamplerDesc, matcapSampler.ReleaseAndGetAddressOf());

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		hr = graphics->getDevice()->CreateDepthStencilState(&depthStencilDesc, depthStencilState.ReleaseAndGetAddressOf());

		depthStencilDesc.DepthEnable = false;
		hr = graphics->getDevice()->CreateDepthStencilState(&depthStencilDesc, depthStencilState2.ReleaseAndGetAddressOf());

		// plane
		plane = createPlaneData(bufferFactory);
		planeShaderDataCollection = createPlaneShaders(shaderFactory);

		// kitana
		//objects = createKitana(device, kitana, kitanaSkeleton, bonesInverseMatrices, bonesObj);
		//objects.push_back(plane);
	}
	catch (runtime_error& err)
	{
		MessageBox(nullptr, err.what(), "Error", MB_OK);
		return 0;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		BOOL r{ PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) };
		if (r == 0)
		{
			try
			{
				UINT presentCount;
				graphics->getSwapChain()->GetLastPresentCount(&presentCount);

				ID3D11Buffer* constantBuffers[4]{ constBufferImmutable->getBuffer(), constBufferProjectionMatrix->getBuffer(), constBufferPerFrame->getBuffer(), constBufferPerObject->getBuffer() };

				render(graphics->getContext(), graphics->getRenderTargetView(presentCount % bufferCount), graphics->getDepthStencilView(), graphics->getSwapChain(), &viewport,
					constantBuffers, plane, planeShaderDataCollection);
			}
			catch (runtime_error& err)
			{
				MessageBox(nullptr, err.what(), "Error", MB_OK);
				return 0;
			}
		}
		else
		{
			DispatchMessage(&msg);
		}
	}

	return 0;
}