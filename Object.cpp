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

void Object::addPosition(float x, float y, float z)
{
	pos._41 += x;
	pos._42 += y;
	pos._43 += z;

	posInverse._41 += -x;
	posInverse._42 += -y;
	posInverse._43 += -z;

	setDirty();
}

void Object::addPosition(const DirectX::XMFLOAT3& position)
{
	addPosition(position.x, position.y, position.z);
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

void Object::addRotation(float x, float y, float z)
{
	XMMATRIX mDX{ XMMatrixRotationRollPitchYaw(x, y, z) };
	XMMATRIX mInverseDX{ XMMatrixRotationRollPitchYaw(-x, -y, -z) };

	XMStoreFloat4x4(&rot, XMLoadFloat4x4(&rot) * mDX);
	XMStoreFloat4x4(&rotInverse, mInverseDX * XMLoadFloat4x4(&rotInverse));

	setDirty();
}

void Object::addRotation(const DirectX::XMFLOAT3& axis, float ang)
{
	XMMATRIX mDX{ XMMatrixRotationAxis(XMLoadFloat3(&axis), ang) };
	XMMATRIX mInverseDX{ XMMatrixRotationAxis(XMLoadFloat3(&axis), -ang) };

	XMStoreFloat4x4(&rot, XMLoadFloat4x4(&rot) * mDX);
	XMStoreFloat4x4(&rotInverse, mInverseDX * XMLoadFloat4x4(&rotInverse));

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

const XMFLOAT4X4& Object::getTransform()
{
	update();
	return transform;
}

const XMFLOAT4X4& Object::getTransformGlobal()
{
	updateGlobal();
	return transformGlobal;
}

void Object::addChild(shared_ptr<Object> child)
{
	Object* p{ this };
	while (p != nullptr)
	{
		if (child.get() == p)
		{
			throw runtime_error{ "Can't add to itself or to child" };
		}

		p = p->parent;
	}

	if (child->parent != nullptr)
	{
		child->parent->removeChild(child);
	}

	child->parent = this;
	children.push_back(child);

	child->setDirty();
}

void Object::removeChild(shared_ptr<Object> child)
{
	if (child->parent != this)
	{
		throw runtime_error{ "Can't remove non existing child" };
	}

	child->parent = nullptr;

	auto it = find(children.begin(), children.end(), child);
	assert(it != children.end());

	children.erase(it);
}

shared_ptr<Object>& Object::removeChildAt(uint32_t index)
{
	shared_ptr<Object>& child{ getChildAt(index) };
	removeChild(child);

	return child;
}

void Object::removeAllChildren()
{
	for (auto& child : children)
	{
		child->parent = nullptr;
	}

	children.clear();
}

bool Object::hasChild(shared_ptr<Object> child)
{
	auto it = find(children.begin(), children.end(), child);

	return it != children.end();
}

shared_ptr<Object> Object::getChildAt(uint32_t pos)
{
	return children.at(pos);
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
	if (dirty)
	{
		XMMATRIX posDX{ XMLoadFloat4x4(&pos) };
		XMMATRIX rotDX{ XMLoadFloat4x4(&rot) };
		XMMATRIX scaleDX{ XMLoadFloat4x4(&scale) };

		XMStoreFloat4x4(&transform, scaleDX * rotDX * posDX);

		XMMATRIX posInverseDX{ XMLoadFloat4x4(&posInverse) };
		XMMATRIX rotInverseDX{ XMLoadFloat4x4(&rotInverse) };
		XMMATRIX scaleInverseleDX{ XMLoadFloat4x4(&scaleInverse) };

		XMStoreFloat4x4(&transformInverse, posInverseDX * rotInverseDX * scaleInverseleDX);

		dirty = false;
	}
}

void Object::updateGlobal()
{
	if (dirty)
	{
		update();

		posGlobal = pos;
		rotGlobal = rot;
		scaleGlobal = scale;
		transformGlobal = getTransform();

		if (parent != nullptr)
		{
			parent->updateGlobal();

			XMMATRIX transformGlobalDX{ XMLoadFloat4x4(&transformGlobal) };
			XMMATRIX parentTransformGlobalDX{ XMLoadFloat4x4(&parent->transformGlobal) };
			XMStoreFloat4x4(&transformGlobal, transformGlobalDX * parentTransformGlobalDX);

			// translation
			posGlobal._41 = -transformGlobal._41;
			posGlobal._42 = -transformGlobal._42;
			posGlobal._43 = -transformGlobal._43;

			// rotation
			XMMATRIX rotGlobalDX{ XMLoadFloat4x4(&rotGlobal) };
			XMMATRIX parentRotGlobalDX{ XMLoadFloat4x4(&parent->rotGlobal) };
			XMStoreFloat4x4(&rotGlobal, rotGlobalDX * parentRotGlobalDX);

			// scale
			scaleGlobal._11 *= parent->scaleGlobal._11;
			scaleGlobal._22 *= parent->scaleGlobal._22;
			scaleGlobal._33 *= parent->scaleGlobal._33;
		}

		// translation inverse
		posInverseGlobal._41 = -posGlobal._41;
		posInverseGlobal._42 = -posGlobal._42;
		posInverseGlobal._43 = -posGlobal._43;

		// rotation inverse
		rotInverseGlobal = rotGlobal;
		XMMATRIX rotInverseGlobalDX{ XMLoadFloat4x4(&rotInverseGlobal) };
		XMStoreFloat4x4(&rotInverseGlobal, XMMatrixTranspose(XMLoadFloat4x4(&rotInverseGlobal)));

		// scale inverse
		scaleInverseGlobal._11 = scaleGlobal._11 != 0 ? 1 / scaleGlobal._11 : 0;
		scaleInverseGlobal._22 = scaleGlobal._22 != 0 ? 1 / scaleGlobal._22 : 0;
		scaleInverseGlobal._33 = scaleGlobal._33 != 0 ? 1 / scaleGlobal._33 : 0;

		XMMATRIX posInverseGlobalDX{ XMLoadFloat4x4(&posInverseGlobal) };
		rotInverseGlobalDX = XMLoadFloat4x4(&rotInverseGlobal);
		XMMATRIX scaleInverseGlobalDX{ XMLoadFloat4x4(&scaleInverseGlobal) };

		XMStoreFloat4x4(&transformGlobalInverse, posInverseGlobalDX * rotInverseGlobalDX * scaleInverseGlobalDX);
	}
}

void Object::setDirty()
{
	if (dirty)
	{
		return;
	}

	dirty = true;
	for (auto& child : children)
	{
		child->setDirty();
	}
}