#include "GameApplication.h"

struct Vertex
{
	D3DXVECTOR3 Pos;
};

CGameApplication::CGameApplication(void)
{
	m_pWindow=NULL;
	m_pD3D10Device=NULL;
	m_pRenderTargetView=NULL;
	m_pSwapChain=NULL;
	m_pVertexBuffer=NULL;
}

CGameApplication::~CGameApplication(void)
{
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	if(m_pVertexBuffer)
		m_pVertexBuffer->Release();

	if(m_pEffect)
		m_pEffect->Release();

	if(m_pRenderTargetView)
		m_pRenderTargetView->Release();

	if(m_pSwapChain)
		m_pSwapChain->Release();

	if(m_pD3D10Device)
		m_pD3D10Device->Release();

	if(m_pVertexLayout)
		m_pVertexLayout->Release();

	if(m_pWindow)
	{
		delete m_pWindow;
		m_pWindow = NULL;
	}
}

bool CGameApplication::init()
{
	if (!initWindow())
			return false;

	if (!initGraphics())
			return false;
	
	if(!initGame())
			return false;

	return true;
}

bool CGameApplication::initGame()
{
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex)*3;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined (DEBUG) ||defined(_DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	if (FAILED(D3DX10CreateEffectFromFile(TEXT("\\Effects\\ScreenSpace.fx"),
		NULL,NULL,"fx_4_0",dwShaderFlags,0,m_pD3D10Device,NULL,NULL,&m_pEffect,
		NULL,NULL )))
	{
		MessageBox(NULL,TEXT("The FX file cannot be loaded. Please run this executable from the directory that contains the FX file"),
			TEXT("Error"),MB_OK);
		return false;
	}

	m_pTechnique=m_pEffect->GetTechniqueByName("Render");

	Vertex vertices[] = 
	{
		D3DXVECTOR3(0.0f,0.5f,0.5f),
		D3DXVECTOR3(0.5f,-0.5f,0.5f),
		D3DXVECTOR3(-0.5,-0.5f,0.5f),
	};

	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;

	if (FAILED(m_pD3D10Device->CreateBuffer(&bd,&initData,&m_pVertexBuffer)))
			return false;

	D3D10_INPUT_ELEMENT_DESC layout[] = 
	{

		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
		D3D10_INPUT_PER_VERTEX_DATA,0},
	};

	UINT numElements = sizeof(layout)/sizeof(D3D10_INPUT_ELEMENT_DESC);
	D3D10_PASS_DESC PassDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	if (FAILED(m_pD3D10Device->CreateInputLayout(layout,numElements,PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize,&m_pVertexLayout)))
	{
		return false;
	}

	m_pD3D10Device->IASetInputLayout(m_pVertexLayout);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pD3D10Device->IASetVertexBuffers(0,1,&m_pVertexBuffer,&stride,&offset);

	m_pD3D10Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}



void CGameApplication::run()
{
	while (m_pWindow->running())
	{
		if (!m_pWindow->checkForWindowMessages())
		{
			update();
			render();
		}
	}
}

void CGameApplication::update()
{
}

bool CGameApplication::initGraphics()
{
	RECT windowRect;

	GetClientRect(m_pWindow->getHandleToWindow(),&windowRect);

	UINT width = windowRect.right-windowRect.left;
	UINT height = windowRect.bottom-windowRect.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof(sd));

		if (m_pWindow->isFullScreen())
			sd.BufferCount = 2;
		else
			sd.BufferCount = 1;

		sd.OutputWindow = m_pWindow->getHandleToWindow();
		sd.Windowed =(BOOL)(!m_pWindow->isFullScreen());
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator=60;
		sd.BufferDesc.RefreshRate.Denominator=1;

		
		if(FAILED(D3D10CreateDeviceAndSwapChain(NULL,D3D10_DRIVER_TYPE_HARDWARE,NULL,createDeviceFlags,
			D3D10_SDK_VERSION,&sd,&m_pSwapChain,&m_pD3D10Device)))
			return false;
		
		ID3D10Texture2D *pBackBuffer;
		if(FAILED(m_pSwapChain->GetBuffer(0,__uuidof(ID3D10Texture2D),(void**)&pBackBuffer)))
			return false;

		if(FAILED(m_pD3D10Device->CreateRenderTargetView(pBackBuffer,NULL,&m_pRenderTargetView)))
		{
			pBackBuffer->Release();
			return false;
		}
		pBackBuffer->Release();

		m_pD3D10Device->OMSetRenderTargets(1,&m_pRenderTargetView,NULL);

		D3D10_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0; 
		m_pD3D10Device->RSSetViewports(1,&vp);

	return true;
}

void CGameApplication::render()
{
	float clearColor[4] = {0.0f,0.125f,0.3f,1.0f};
	m_pD3D10Device->ClearRenderTargetView(m_pRenderTargetView,clearColor);


	//DRAW CALL'S MUST GO HERE BETWEEN
	m_pSwapChain->Present(0,0);
}

bool  CGameApplication::initWindow()
{
	m_pWindow=new CWin32Window();

	if (!m_pWindow->init(TEXT("Lab1-Create Device"),800,640,false))
		return false;


	return true;
}

