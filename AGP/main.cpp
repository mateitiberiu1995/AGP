#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
//#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include "camera.h"
#include "text2D.h"
#include "Model.h"
#include <dinput.h>
//////////////////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pD3DDevice = NULL;
ID3D11DeviceContext* g_pImmediateContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;ID3D11RenderTargetView* g_pBackBufferRTView = NULL;ID3D11Buffer* g_pVertexBuffer;
ID3D11VertexShader* g_pVertexShader;
ID3D11PixelShader* g_pPixelShader;
ID3D11InputLayout* g_pInputLayout;
ID3D11Buffer* g_pConstantBuffer0;
ID3D11DepthStencilView* g_pZBuffer;
ID3D11ShaderResourceView* g_pTexture0;
ID3D11ShaderResourceView* g_pTexture1;
ID3D11ShaderResourceView* g_pTexture2;
ID3D11SamplerState* g_pSampler0;
Text2D* g_2DText;
Camera* m_camera;
Camera* second_camera;
Camera* first_camera;
XMVECTOR g_directional_light_shines_from;
XMVECTOR g_directional_light_colour;
XMVECTOR g_ambient_light_colour;
Model* entity;
Model* entity2;
Model* entity3;
Model* bullet;
Model* power;
IDirectInput8* g_direct_input;
IDirectInputDevice8* g_keyboard_device;
unsigned char g_keyboard_keys_state[256];
int okay = 0; // variable to check if the player started shooting so we can initiate the bullet
float speed = 0.01f; // the speed of the entities
boolean switchbetweenCamera = true;
struct POS_COL_TEX_NORM_VERTEX
{
	XMFLOAT3 Pos;
	XMFLOAT4 Col;
	XMFLOAT2 Texture0;
	XMFLOAT3 Normal;
};

//Const buffer structs. Pack to 80 bytes. Don't let any single element cross a 16 byte boundary
struct CONSTANT_BUFFER0
{
	XMMATRIX WorldViewProjection; // 64 bytes
	XMVECTOR directional_light_vector; // 16 bytes
	XMVECTOR directional_light_colour; // 16 bytes
	XMVECTOR ambient_light_colour; // 16 bytes
	float scaleAmount;// 4 bytes
	float RedAmount; // 4 bytes
	XMFLOAT2 packing_bytes; // 2*4 = 8 bytes
	

}; // 128 bytes in total
// Rename for each tutorial
char g_TutorialName[100] = "Tiberiu Matei";
//////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
void ReadInputStates();
boolean IsKeyPressed(unsigned char DI_keycode);
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitialiseKeyboard();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
void ShutdownD3D();
void RenderFrame(void);
HRESULT InitialiseGraphics(void);
//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		//DXTRACE_MSG("Failed to create Window");
		return 0;
	}
	if (FAILED(InitialiseKeyboard()))
	{
		//DXTRACE_MSG("Failed to initialise Keyboard");
		return 0;
	}
	if (FAILED(InitialiseD3D()))
	{
		//DXTRACE_MSG("Failed to create Device");
		return 0;
	}
	if (FAILED(InitialiseGraphics()))
	{
		//DXTRACE_MSG("Failed to initialise graphics");
		return 0;
	}
	// Main message loop
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RenderFrame();
				// do something
		}
	}
	return (int)msg.wParam;
}
//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "Tiberiu";
	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	// wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;
	if (!RegisterClassEx(&wcex)) return E_FAIL;
	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;
	ShowWindow(g_hWnd, nCmdShow);
	
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == 'W')
			m_camera->Forward(1.0f);
		if (wParam == 'D')
			m_camera->Rotate(5.0f);
		if (wParam == 'A')
			m_camera->Rotate(-5.0f);
		if (wParam == 'S')
			m_camera->Forward(-1.0f);
		if (wParam == 'R')
		{
			bullet = new Model(g_pD3DDevice, g_pImmediateContext);
			bullet->LoadObjModel("assets/sphere.obj");
			bullet->setScale(0.2f);
			bullet->setX(m_camera->getX());
			bullet->setY(m_camera->getY());
			bullet->setZ(m_camera->getZ());
			bullet->setYangle(m_camera->getAngle()); // bullet model that set from the camera position.
			okay = 1;
		}
		if (wParam == 'Q')
		{
			if (switchbetweenCamera)
			{
				first_camera = m_camera;
				m_camera = second_camera;   // switches between Cameras
				switchbetweenCamera = false;
			}
			else
				if (switchbetweenCamera == false)
				{
					second_camera = m_camera;
					m_camera = first_camera;
					switchbetweenCamera = true;
				}
		}

		if(wParam == VK_ESCAPE)
			DestroyWindow(g_hWnd);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
	    D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	
		D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;
	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);
	if (FAILED(hr)) return hr;
	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&g_pBackBufferRTView);
	pBackBufferTexture->Release();
	if (FAILED(hr)) return hr;
	//Create a Zbuffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D *pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if (FAILED(hr))
		return hr;

	//Create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();

	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);
	// Set the viewport
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	g_pImmediateContext->RSSetViewports(1, &viewport);
	g_2DText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////////////////
// Keyboard
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseKeyboard()
{
	HRESULT hr;
	ZeroMemory(g_keyboard_keys_state, sizeof(g_keyboard_keys_state));

	hr = DirectInput8Create(g_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_direct_input, NULL);
	if (FAILED(hr)) return hr;

	hr = g_direct_input->CreateDevice(GUID_SysKeyboard, &g_keyboard_device, NULL);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->Acquire();
	if (FAILED(hr)) return hr;

	return S_OK;
}
//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_keyboard_device)
	{
		g_keyboard_device->Unacquire();
		g_keyboard_device->Release();
	}
	if (g_direct_input) g_direct_input->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();//03-01
	if (g_pInputLayout) g_pInputLayout->Release();//03-01
	if (g_pVertexShader) g_pVertexShader->Release();//03-01
	if (g_pPixelShader) g_pPixelShader->Release();//03-01
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pConstantBuffer0) g_pConstantBuffer0->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pZBuffer) g_pZBuffer->Release();
	if (g_pTexture0) g_pTexture0->Release();
	if (g_pTexture1) g_pTexture1->Release();
	if (g_pTexture2) g_pTexture2->Release();
	if (g_pSampler0) g_pSampler0->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
	entity->~Model();
	entity2->~Model();
	entity3->~Model();
	bullet->~Model();
	power->~Model();
}//////////////////////////////////////////////////////////////////////////////////////
// Read input states
//////////////////////////////////////////////////////////////////////////////////////void ReadInputStates(){	HRESULT hr;	hr = g_keyboard_device->GetDeviceState(sizeof(g_keyboard_keys_state), (LPVOID)&g_keyboard_keys_state);	/*if (FAILED(hr))	{		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))		{			g_keyboard_device->Acquire();		}	}*/}boolean IsKeyPressed(unsigned char DI_keycode){	return g_keyboard_keys_state[DI_keycode] & 0x80;}// Render frame
////////////////////////////////////////
// Init Graphics
///////////////////////////////////////
HRESULT InitialiseGraphics()//03-01
{
	power = new Model(g_pD3DDevice, g_pImmediateContext);
	power->LoadObjModel("assets/sphere.obj");
	power->setX(90.0);
	power->setZ(90.0); // define the power model 
	power->setScale(1.1f);
	entity = new Model(g_pD3DDevice, g_pImmediateContext); // define entity model
	entity->LoadObjModel("assets/sphere.obj");
	entity2 = new Model(g_pD3DDevice, g_pImmediateContext);
	entity2->LoadObjModel("assets/sphere.obj");
	entity2->setX(-25.0f); // define entity2 model
	entity2->setY(0.0f);
	entity2->setZ(-5.0f);
	entity2->setScale(1.5f);
	entity3 = new Model(g_pD3DDevice, g_pImmediateContext);
	entity3->LoadObjModel("assets/sphere.obj"); // define entity 3 model
	entity3->setX(0.0);
	entity3->setZ(-96.0);
	
	HRESULT hr = S_OK;
	
	//Define vertices of a triangle - screen coordinates -1.0 to +1.0
	POS_COL_TEX_NORM_VERTEX vertices[]
	{
		//back face
		{XMFLOAT3(-100.0f, 100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f), XMFLOAT3(0.0f,0.0f,1.0f)},
		{ XMFLOAT3(-100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(100.0f, 100.0f,100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(100.0f, 100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(0.0f,0.0f,1.0f) },
		//front face
		{ XMFLOAT3(-100.0f, -100.0f,-100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,-100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(100.0f, -100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(0.0f,0.0f,-1.0f) },
		//left face

		{ XMFLOAT3(-100.0f, -100.0f,-100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(-1.0f,0.0f,0.0f) },
		//right face
		{ XMFLOAT3(100.0f, -100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(100.0f, -100.0f,-100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(100.0f, 100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(1.0f,0.0f,0.0f) },
		//bottom face
		{ XMFLOAT3(100.0f, -100.0f,-100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(100.0f, -100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-100.0f, -100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,-1.0f,0.0f) },
		//top face
		{ XMFLOAT3(100.0f, 100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(1.0f,1.0f) , XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,100.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f), XMFLOAT2(0.0f,0.0f) , XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(100.0f, 100.0f,100.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f), XMFLOAT2(1.0f,0.0f) , XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-100.0f, 100.0f,-100.0f), XMFLOAT4(0.0f,0.0f,1.0f,1.0f), XMFLOAT2(0.0f,1.0f) , XMFLOAT3(0.0f,1.0f,0.0f) },

		

	};
	//Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // used by GPu and cpu
	bufferDesc.ByteWidth = sizeof(vertices) ; // number of vertices
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use as a vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU access
	hr = g_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &g_pVertexBuffer); // Create the buffer 
	if (FAILED(hr))
	{
		return hr;
	}
	D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice, "assets/texture.bmp" , NULL, NULL, &g_pTexture0, NULL);
	D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice, "assets/texture1.bmp", NULL, NULL, &g_pTexture1, NULL);
	D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice, "assets/texture2.bmp", NULL, NULL, &g_pTexture2, NULL);
	//texture
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	g_pD3DDevice->CreateSamplerState(&sampler_desc, &g_pSampler0);
	//Create constant buffer
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT; //Can use UpdateSubresource() to update
	constant_buffer_desc.ByteWidth = 128; //MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //Use as a constant buffer

	hr = g_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &g_pConstantBuffer0);

	if (FAILED(hr))
		return hr;

	
	//Copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	//lock the buffer to allow writing
	g_pImmediateContext->Map(g_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	//Copy the data
	memcpy(ms.pData, vertices, sizeof(vertices));
	//Unlock the buffer
	g_pImmediateContext->Unmap(g_pVertexBuffer, NULL);
	//Load and compile pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &error, 0);
	if (error != 0) //Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) // dont fail if error is just a warning
		{
			return hr;
		}
	}
	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &error, 0);
	if (error != 0) //Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) // dont fail if error is just a warning
		{
			return hr;
		}
	}
	hr = g_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVertexShader);
	if(FAILED(hr))
	{
		return hr;
	}
	hr = g_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPixelShader);
	if (FAILED(hr))
	{
		return hr;
	}
	//Set the shader objects as active
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);

	//Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{"POSITION", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0 , 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0 , D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	}; 
	hr = g_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &g_pInputLayout);
	if (FAILED(hr))
	{
		return hr;
	}
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);
	m_camera = new Camera(-70.0, 0.0, 0.0, 90.0); // set us the cameras
	second_camera = new Camera(0.0, 0.0, -0.5, 0.0);
	
	return S_OK;
	
		
	
}
void RenderFrame(void)
{
	// set values for the light function
	g_directional_light_shines_from = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	g_directional_light_colour = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	g_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);
	// this needs to be called every frame because of the text
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);
	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.5f, 0.5f, 0.9f, 1.0f }; // colour you want for the background
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour); // puts the color on the background
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	//Select which primitive type to use // 03-01
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// i used constant buffers to create the map
	CONSTANT_BUFFER0 cb0_values;
	CONSTANT_BUFFER0 cb1;
	CONSTANT_BUFFER0 cb2;
	CONSTANT_BUFFER0 cb3;
	CONSTANT_BUFFER0 cb4;

	XMMATRIX projection, world, view, world2, world3, world4, world5, transpose;
	transpose = XMMatrixTranspose(world);

	// we create the environment
	cb0_values.directional_light_colour = g_directional_light_colour;
	cb0_values.ambient_light_colour = g_ambient_light_colour;
	cb0_values.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	cb0_values.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);

	cb2.directional_light_colour = g_directional_light_colour;
	cb2.ambient_light_colour = g_ambient_light_colour;
	cb2.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	cb2.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);

	cb3.directional_light_colour = g_directional_light_colour;
	cb3.ambient_light_colour = g_ambient_light_colour;
	cb3.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	cb3.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);

	cb4.directional_light_colour = g_directional_light_colour;
	cb4.ambient_light_colour = g_ambient_light_colour;
	cb4.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	cb4.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);

	world = XMMatrixRotationX(XMConvertToRadians(0));
	world *= XMMatrixTranslation(-200, 0, 0);
	
	world2 = XMMatrixRotationX(XMConvertToRadians(0));
	world2 *= XMMatrixTranslation(0, -105, 0);

	world3 = XMMatrixRotationX(XMConvertToRadians(0));
	world3 *= XMMatrixTranslation(200, 0, 0);

	world4 = XMMatrixRotationX(XMConvertToRadians(0));
	world4 *= XMMatrixTranslation(0, 0, -200);

	world5 = XMMatrixRotationX(XMConvertToRadians(0));
	world5 *= XMMatrixTranslation(0, 0, 200);

	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0 / 480.0, 1.0, 1000.0);
	view = m_camera->GetViewMatrix();
	
	
	cb0_values.WorldViewProjection = world *view *projection;
	cb1.WorldViewProjection = world2*view*projection;
	cb2.WorldViewProjection = world3*view*projection;
	cb3.WorldViewProjection = world4*view*projection;
	cb4.WorldViewProjection = world5*view*projection;

	// positions are from -1.0 to +1.0 for x and y, represents top left of string on screen
	//size is fraction of screen size
	g_2DText->AddText("Press R to shoot the enemy.", -1.0, +1.0, .05);
	

	//upload the new values for the constant buffer
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSampler0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture0);
	//Draw the vertex buffer to the back buffer 
	g_pImmediateContext->Draw(36, 0);
	//upload the new values for the constant buffe
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb1, 0, 0);
	//Draw the vertex buffer to the back buffer 
	g_pImmediateContext->Draw(36, 0);
	//upload the new values for the constant buffe
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb2, 0, 0);
	//Draw the vertex buffer to the back buffer 
	g_pImmediateContext->Draw(36, 0);
	//upload the new values for the constant buffe
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb3, 0, 0);
	//Draw the vertex buffer to the back buffer 
	g_pImmediateContext->Draw(36, 0);
	//upload the new values for the constant buffe
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb4, 0, 0);
	//Draw the vertex buffer to the back buffer 
	g_pImmediateContext->Draw(36, 0);

	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture1);
	entity->LookAt_XZ(m_camera->getX(), m_camera->getZ());
	if(entity->CheckCollisionPoint(m_camera->getX(),m_camera->getY(),m_camera->getZ()))
		DestroyWindow(g_hWnd);  // we check for collision with the player , if its true we close the game
	else
	{
		if (entity->CheckCollision(entity2) || entity->CheckCollision(entity3))
			entity->MoveForward(-speed); // we check for collision with the other entity
		else
			entity->MoveForward(speed);
	}
	entity2->LookAt_XZ(m_camera->getX(), m_camera->getZ()); // we put entity2 to look at player
	if (entity2->CheckCollisionPoint(m_camera->getX(), m_camera->getY(), m_camera->getZ()))
		DestroyWindow(g_hWnd);
	else
	{
		if (entity2->CheckCollision(entity) || entity->CheckCollision(entity3)) // check for collision
			entity2->MoveForward(-speed);
		else
			entity2->MoveForward(speed);
	}
	entity3->LookAt_XZ(m_camera->getX(), m_camera->getZ());
	if (entity3->CheckCollisionPoint(m_camera->getX(), m_camera->getY(), m_camera->getZ()))
		DestroyWindow(g_hWnd);
	else
	{
		if (entity3->CheckCollision(entity) || entity3->CheckCollision(entity2)) // check for collision
			entity3->MoveForward(-speed);
		else
			entity3->MoveForward(speed);
	}
	if (okay == 1)
	{
		bullet->MoveForward(0.3f);
		bullet->Draw(view, projection);
		if (bullet->CheckCollision(entity))
			entity->MoveForward(-0.2f);
		if (bullet->CheckCollision(entity2)) // shooting the bullets and check for collision
			entity2->MoveForward(-0.2f);
		if (bullet->CheckCollision(entity3))
			entity3->MoveForward(-0.2f);
	}
	if (power->CheckCollisionPoint(m_camera->getX(), m_camera->getY(), m_camera->getZ()) && power->getX() == 90.0)
	{
		power->setX(-90.0);
		power->setZ(-90.0);
		speed = 0.005f; // a powerup that will change the speed of the entities to half
	}
	else
	{
		if (power->CheckCollisionPoint(m_camera->getX(), m_camera->getY(), m_camera->getZ()) && power->getX() == -90.0)
		{
			power->setX(90.0);
			power->setZ(90.0);
			speed = 0.3f; // this is not a powerup , it will make the entities move faster
		}
	}

	entity->Draw( view, projection);
	entity2->Draw(view, projection);
	entity3->Draw(view, projection);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture2);
	power->Draw(view, projection);
	// text render
	g_2DText->RenderText();
	// RENDER HERE
	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);

}