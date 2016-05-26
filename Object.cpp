#include "Headers/Object.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

Object::Object()
{

}

Object::Object(shared_ptr<Mesh> mesh) : mesh{ mesh }, offsets(mesh->getVertexBuffersNum(), 0)
{

}

bool Object::haveMesh()
{
	return mesh != nullptr;
}

const vector<ID3D11Buffer*>& Object::getVertexBuffers()
{
	return mesh->getVertexBuffers();
}

ID3D11Buffer* Object::getIndexBuffer()
{
	return mesh->getIndexBuffer();
}

UINT Object::getNumIndices()
{
	return mesh->getNumIndices();
}

const vector<UINT>& Object::getVertexStrides()
{
	return mesh->getVertexStrides();
}

const vector<UINT>& Object::getVertexOffsets()
{
	return offsets;
}

void Object::setPosition(float x, float y, float z)
{
	pos._41 = x;
	pos._42 = y;
	pos._43 = z;

	posInverse._41 = -x;
	posInverse._42 = -y;
	posInverse._43 = -z;

	setDirty();
}

void Object::setPosition(const DirectX::XMFLOAT3& position)
{
	setPosition(position.x, position.y, position.z);
}

DirectX::XMFLOAT3 Object::getPosition()
{
	return{ pos._41, pos._42, pos._43 };
}

void Object::setRotation(float x, float y, float z)
{
	XMStoreFloat4x4(&rot, XMMatrixRotationRollPitchYaw(x, y, z));
	XMStoreFloat4x4(&rotInverse, XMMatrixRotationRollPitchYaw(-x, -y, -z));

	setDirty();
}

void Object::setScale(float x, float y, float z)
{
	scale._11 = x;
	scale._22 = y;
	scale._33 = z;

	scaleInverse._11 = x != 0 ? 1 / x : 0;
	scaleInverse._22 = y != 0 ? 1 / y : 0;
	scaleInverse._33 = z != 0 ? 1 / z : 0;

	setDirty();
}

XMFLOAT4X4 Object::getTransform()
{
	if (dirty)
	{
		update();
	}

	return transform;
}

XMFLOAT4X4 Object::getTransformGlobal()
{
	if (dirty)
	{
		updateGlobal();
	}

	return transformGlobal;
}

void Object::addChild(shared_ptr<Object> child)
{
	children.push_back(child);
	child->parent = this;

	child->setDirty();
}

void Object::removeChild(shared_ptr<Object> child)
{

}

void Object::removeAllChildren()
{
	//children.clear();
}

bool Object::hasChild(shared_ptr<Object> child)
{
	return false;
}

shared_ptr<Object> Object::getChildAt(uint32_t pos)
{
	return children[pos];
}

uint32_t Object::getNumChildren()
{
	return static_cast<uint32_t>(children.size());
}

Object* Object::getParent()
{
	return parent;
}

void Object::update()
{
	XMMATRIX posDX{ XMLoadFloat4x4(&pos) };
	XMMATRIX rotDX{ XMLoadFloat4x4(&rot) };
	XMMATRIX scaleDX{ XMLoadFloat4x4(&scale) };

	XMStoreFloat4x4(&transform, scaleDX * rotDX * posDX);

	XMMATRIX posInverseDX{ XMLoadFloat4x4(&posInverse) };
	XMMATRIX rotInverseDX{ XMLoadFloat4x4(&rot) };
	XMMATRIX scaInverseleDX{ XMLoadFloat4x4(&scale) };

	XMStoreFloat4x4(&transformInverse, posInverseDX * scaInverseleDX * rotInverseDX);

	dirty = false;
}

void Object::updateGlobal()
{
	transformGlobal = getTransform();

	if (parent != nullptr)
	{
		XMFLOAT4X4 matParent{ parent->getTransformGlobal() };

		XMMATRIX matDX{ XMLoadFloat4x4(&transformGlobal) };
		XMMATRIX matParentDX{ XMLoadFloat4x4(&matParent) };

		XMStoreFloat4x4(&transformGlobal, matDX * matParentDX);
	}
}

void Object::setDirty()
{
	if (dirty) return;

	dirty = true;
	for (auto& child : children)
	{
		child->setDirty();
	}
}