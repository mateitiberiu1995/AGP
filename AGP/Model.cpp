#include "Model.h"
///////////
//Global variable
//////
ID3D11VertexShader* m_pVertexShader;
ID3D11PixelShader* m_pPixelShader;
ID3D11InputLayout* m_pInputLayout;
ID3D11Buffer* m_pConstantBuffer0;
struct MODEL_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection;
}; // 64 bytes
Model::Model(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pImmediateContext)
{
	m_pD3DDevice = pD3DDevice;
	m_pImmediateContext = pImmediateContext;
	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;
	m_xangle = 0.0f;
	m_yangle = 0.0f;
	m_zangle = 0.0f;
	m_scale = 1.0f;
}
float Model::LoadObjModel(char* filename)
{
	HRESULT hr = S_OK;
	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);
	if (m_pObject->filename == "FILE NOT LOADED")
		return S_FALSE;
	//Create constant buffer
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresource() to update
	constant_buffer_desc.ByteWidth = 64; //MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer0);

	if (FAILED(hr))
		return hr;

	//Load and compile pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);
	if (error != 0) //Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) // dont fail if error is just a warning
		{
			return hr;
		}
	}
	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);
	if (error != 0) //Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) // dont fail if error is just a warning
		{
			return hr;
		}
	}
	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPixelShader);
	if (FAILED(hr))
	{
		return hr;
	}
	//Set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVertexShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPixelShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{ "POSITION", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0 , 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);
	if (FAILED(hr))
	{
		return hr;
	}
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	CalculateModelCentrePoint();
	CalculateBoundingSphereRadius();
	return S_OK;
}
void Model::Draw(XMMATRIX view, XMMATRIX projection)
{
	MODEL_CONSTANT_BUFFER model_cb_values;
	
	XMMATRIX world;
	world = XMMatrixRotationRollPitchYaw(m_xangle, m_yangle, m_zangle)*XMMatrixScaling(m_scale,m_scale,m_scale)*XMMatrixTranslation(m_x,m_y,m_z);
	model_cb_values.WorldViewProjection = world*(view)*(projection);
	//set the constant buffer active
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer0);
	//send the new matrix to the constant buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer0, 0, 0, &model_cb_values, 0, 0);
	//set to active vertex shader, pixel shader and imput layout
	m_pImmediateContext->VSSetShader(m_pVertexShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPixelShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	m_pObject->Draw();

}
void Model::setX(float x)
{
	m_x = x;
}
void Model::setY(float y)
{
	m_y = y;
}
void Model::setZ(float z)
{
	m_z = z;
}
void Model::setXangle(float xangle)
{
	m_xangle = xangle;
}
void Model::setYangle(float yangle)
{
	m_yangle = yangle;
}
void Model::setZangle(float zangle)
{
	m_zangle = zangle;
}
void Model::setScale(float scale)
{
	m_scale = scale;
}
float Model::getX(void)
{
	return m_x;
}
float Model::getY(void)
{
	return m_y;
}
float Model::getZ(void)
{
	return m_z;
}
float Model::getXangle(void)
{
	return m_xangle;
}
float Model::getYangle(void)
{
	return m_yangle;
}
float Model::getZangle(void)
{
	return m_zangle;
}
float Model::getScale(void)
{
	return m_scale;
}
void Model::IncX(float incx)
{
	m_x += incx;
}
void Model::IncY(float incy)
{
	m_y += incy;
}
void Model::IncZ(float incz)
{
	m_z += incz;
}
void Model::IncXangle(float incxangle)
{
	m_xangle += incxangle;
}
void Model::IncYangle(float incyangle)
{
	m_yangle += incyangle;
}
void Model::IncZangle(float inczangle)
{
	m_zangle += inczangle;
}
void Model::IncScale(float incscale)
{
	m_scale += incscale;
}
void Model::LookAt_XZ(float x, float z)
{
	float dx = x - m_x;
	float dz = z - m_z;
	m_yangle = atan2(dx, dz) * (180.0 / XM_PI);
}
void Model::MoveForward(float distance)
{
	m_x += sin(m_yangle * (XM_PI / 180.0)) * distance;
	m_z += cos(m_yangle * (XM_PI / 180.0)) * distance;
	if (m_x > 97.0)
		m_x -= 0.1;
	if (m_x<-97.0)
		m_x += 0.1;
	if (m_z > 97.0)
		m_z -= 0.1;
	if (m_z<-97.0)
		m_z += 0.1;
}
void Model::CalculateModelCentrePoint()  // calculate the centre
{
	float min_x = m_pObject->vertices[0].Pos.x;
	float max_x = m_pObject->vertices[0].Pos.x;
	float min_y = m_pObject->vertices[0].Pos.y;
	float max_y = m_pObject->vertices[0].Pos.y;
	float min_z = m_pObject->vertices[0].Pos.z;
	float max_z = m_pObject->vertices[0].Pos.z;

	for (int i = 1; i < m_pObject->numverts; i++)
	{
		if (min_x > m_pObject->vertices[i].Pos.x)
			min_x = m_pObject->vertices[i].Pos.x;
		if (max_x < m_pObject->vertices[i].Pos.x)
			max_x = m_pObject->vertices[i].Pos.x;
		if (min_y > m_pObject->vertices[i].Pos.y)
			min_y = m_pObject->vertices[i].Pos.y;
		if (max_y < m_pObject->vertices[i].Pos.y)
			max_y = m_pObject->vertices[i].Pos.y;
		if (min_z > m_pObject->vertices[i].Pos.z)
			min_z = m_pObject->vertices[i].Pos.z;
		if (max_z < m_pObject->vertices[i].Pos.z)
			max_z = m_pObject->vertices[i].Pos.z;
	}
	m_bounding_sphere_centre_x = (min_x + max_x) / 2;
	m_bounding_sphere_centre_y = (min_y + max_y) / 2;
	m_bounding_sphere_centre_z = (min_z + max_z) / 2;
}
void Model::CalculateBoundingSphereRadius() // we calculate the radius
{
	float max_distance_squared = (m_bounding_sphere_centre_x - m_pObject->vertices[0].Pos.x)*(m_bounding_sphere_centre_x - m_pObject->vertices[0].Pos.x) + (m_bounding_sphere_centre_y - m_pObject->vertices[0].Pos.y)*(m_bounding_sphere_centre_y - m_pObject->vertices[0].Pos.y) + (m_bounding_sphere_centre_z - m_pObject->vertices[0].Pos.z)*(m_bounding_sphere_centre_z - m_pObject->vertices[0].Pos.z);
	for (int i = 1; i < m_pObject->numverts; i++)
	{
		if (max_distance_squared < (m_bounding_sphere_centre_x - m_pObject->vertices[i].Pos.x)*(m_bounding_sphere_centre_x - m_pObject->vertices[i].Pos.x) + (m_bounding_sphere_centre_y - m_pObject->vertices[i].Pos.y)*(m_bounding_sphere_centre_y - m_pObject->vertices[i].Pos.y) + (m_bounding_sphere_centre_z - m_pObject->vertices[i].Pos.z)*(m_bounding_sphere_centre_z - m_pObject->vertices[i].Pos.z))
			max_distance_squared = (m_bounding_sphere_centre_x - m_pObject->vertices[i].Pos.x)*(m_bounding_sphere_centre_x - m_pObject->vertices[i].Pos.x) + (m_bounding_sphere_centre_y - m_pObject->vertices[i].Pos.y)*(m_bounding_sphere_centre_y - m_pObject->vertices[i].Pos.y) + (m_bounding_sphere_centre_z - m_pObject->vertices[i].Pos.z)*(m_bounding_sphere_centre_z - m_pObject->vertices[i].Pos.z);
	}
	m_bounding_sphere_radius = sqrt(max_distance_squared);
}
XMVECTOR Model::GetBoundingSphereWorldSpacePosition()
{
	XMMATRIX world;
	XMVECTOR offset;
	world = XMMatrixRotationRollPitchYaw(m_xangle, m_yangle, m_zangle)*XMMatrixScaling(m_scale, m_scale, m_scale)*XMMatrixTranslation(m_x, m_y, m_z);
	offset = XMVectorSet(m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, 0.0);
	offset = XMVector3Transform(offset, world);
	return offset;
}
float Model::GetBoundingSphereRadius()
{
	return m_scale*m_bounding_sphere_radius;
}
boolean Model::CheckCollision(Model* checkModel)
{
	if (this->m_pObject == checkModel->m_pObject) // checks collision with another sphere
		return false;
	XMVECTOR current_model, parameter_model;
	current_model = GetBoundingSphereWorldSpacePosition();
	parameter_model = checkModel->GetBoundingSphereWorldSpacePosition();
	float x1 = XMVectorGetX(current_model);
	float x2 = XMVectorGetX(parameter_model);
	float y1 = XMVectorGetY(current_model);
	float y2 = XMVectorGetY(parameter_model);
	float z1 = XMVectorGetZ(current_model);
	float z2 = XMVectorGetZ(parameter_model);
	float distance = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
	if (distance < GetBoundingSphereRadius() + checkModel->GetBoundingSphereRadius())
		return true;
	else
		return false;
}
boolean Model::CheckCollisionPoint(float x, float y,float z)
{
	XMVECTOR current_model;
	current_model = GetBoundingSphereWorldSpacePosition();
	float x1 = XMVectorGetX(current_model);
	float y1 = XMVectorGetY(current_model);  // check collision with a point 
	float z1 = XMVectorGetZ(current_model);
	float distance = sqrt((x1 - x)*(x1 - x) + (y1 - y)*(y1 - y) + (z1 - z)*(z1 - z));
	if (distance < GetBoundingSphereRadius())
		return true;
	else
		return false;
}
Model::~Model()
{
	delete m_pObject;
	if (m_pInputLayout) m_pInputLayout->Release();//03-01
	if (m_pVertexShader) m_pVertexShader->Release();//03-01 
	if (m_pPixelShader) m_pPixelShader->Release();
	if (m_pConstantBuffer0) m_pConstantBuffer0->Release();   // deconstructor
}