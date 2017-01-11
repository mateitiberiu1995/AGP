#pragma once
#include "ObjFileModel.h"
class Model
{
public:
	Model(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pImmediateContext);
	~Model();
	float LoadObjModel(char* filename);
	void Draw(XMMATRIX view, XMMATRIX projection);
	void LookAt_XZ(float x, float z);
	void MoveForward(float distance);
	XMVECTOR GetBoundingSphereWorldSpacePosition();
	float GetBoundingSphereRadius();
	boolean CheckCollision(Model* checkmodel);
	boolean CheckCollisionPoint(float x, float y, float z);
	void setX(float x);
	void setY(float y);
	void setZ(float Z);
	void setXangle(float xangle);
	void setYangle(float yangle);
	void setZangle(float zangle);
	void setScale(float scale);
	float getX(void);
	float getY(void);
	float getZ(void);
	float getXangle(void);
	float getYangle(void);
	float getZangle(void);
	float getScale(void);
	void IncX(float incx);
	void IncY(float incy);
	void IncZ(float incz);
	void IncXangle(float incxangle);
	void IncYangle(float incyangle);
	void IncZangle(float inczangle);
	void IncScale(float incscale);



private:
	void CalculateModelCentrePoint();
	void CalculateBoundingSphereRadius();
	ID3D11Device* m_pD3DDevice;
	ID3D11DeviceContext* m_pImmediateContext;

	ObjFileModel* m_pObject;
	ID3D11VertexShader* m_pVShader;
	ID3D11PixelShader* m_pPShader;
	ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer* m_pConstantBuffer;

	float m_x, m_y, m_z;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;
	float m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, m_bounding_sphere_radius;
};
