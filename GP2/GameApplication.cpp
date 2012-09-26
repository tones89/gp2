#include "GameApplication.h"

struct Vertex
{
	D3DXVECTOR3 Pos;
};


//======THE CLASS' CONSTRUCTOR======
CGameApplication::CGameApplication(void)
{
	m_pWindow=NULL;
	m_pD3D10Device=NULL;
	m_pRenderTargetView=NULL;
	m_pSwapChain=NULL;
	m_pVertexBuffer=NULL;
	m_pDepthStencilView=NULL;
	m_pDepthStencilTexture=NULL;
}
//====================================

//======THE CLASS' DESTRUCTOR======
CGameApplication::~CGameApplication(void)
{
	

	if(m_pVertexBuffer)
		m_pVertexBuffer->Release();

	if(m_pEffect)
		m_pEffect->Release();

	if(m_pRenderTargetView)
		m_pRenderTargetView->Release();

	if(m_pDepthStencilTexture)
	m_pDepthStencilTexture->Release();


	if(m_pDepthStencilView)
		m_pDepthStencilView->Release();

	
	if(m_pSwapChain)
		m_pSwapChain->Release();

	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

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
//=======================================


//=========USED TO INITIALIZE THE GAME=========
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
//================================================


bool CGameApplication::initGame()
{
	D3D10_BUFFER_DESC bd;	
	bd.Usage = D3D10_USAGE_DEFAULT;		//DESC OF HOW THE BUFFER IS READ/WRITTEN TO, DEFAULT STIPULATES THAT THE RESOURCE WILL BE WRITTEN BY CPU
	bd.ByteWidth = sizeof(Vertex)*3;	//THE SIZE OF BUFFER( IN THIS CASE 3 VERTICES)
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;	//THE TYPE OF BUFFER, IN THIS CASE VERTEX BUFFER
	bd.CPUAccessFlags = 0;	//USED TO SPECIFY IF THE BUFFER CAN BE READ/WRITTEN BY CPU. ZERO MEANS THE CPU CANT ACCESS THE BUFFER ONCE CREATED
	bd.MiscFlags = 0;	//USED FOR ADDITIONAL OPTIONS, 0 MEANING NO ADDITIONAL OPTIONS

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined (DEBUG) ||defined(_DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
	ID3D10Blob* pErrors = NULL;

	if (FAILED(D3DX10CreateEffectFromFile(TEXT("Transform.fx"),
		NULL,NULL,"fx_4_0",dwShaderFlags,0,m_pD3D10Device,NULL,NULL,&m_pEffect,
		&pErrors,NULL )))
	{
		MessageBoxA(NULL,(char*)pErrors->GetBufferPointer(),
			"error",MB_OK);
		return false;
	}

	m_pTechnique=m_pEffect->GetTechniqueByName("Render");

	//=========DEFINES A SIMPLE ARRAY OF 3 VERTICES=========
	Vertex vertices[] = 
	{
		D3DXVECTOR3(0.0f,0.5f,0.5f),
		D3DXVECTOR3(0.5f,-0.5f,-0.5f),
		D3DXVECTOR3(-0.5,-0.5f,0.5f),
		
	};
	//=====================================================


	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;

	//=========CREATING THE BUFFER=========
	//1ST PARAM = POINTER TO BUFFER DESCRIPTION
	//2ND PARAM = POINTER TO RESOURCE DATA
	//3RD PARAM = A MEMORY ADDRESS OF A POINTER TO A BUFFER
	if (FAILED(m_pD3D10Device->CreateBuffer(&bd,&initData,&m_pVertexBuffer)))
			return false;
	//=======================================
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

	//=========SETTING UP THE CAMERA=========
	D3DXVECTOR3 cameraPos(0.0f,0.0f,-10.0f);
	D3DXVECTOR3 cameraLook(0.0f,0.0f,1.0f);
	D3DXVECTOR3 cameraUp(0.0f,0.1f,0.0f);
	//==========================================

	//THIS FUNCTION CALCULATES THE VIEW MATRIX USING THE CAM VARS FROM ABOVE;
	D3DXMatrixLookAtLH(&m_matView,&cameraPos,&cameraLook,&cameraUp);   
	//======================================================

	//=========USED TO GRAB THE VIEWPORT, WHICH HOLDS THE SCREEN DIMENSIONS=========
	D3D10_VIEWPORT vp;  
	UINT numViewPorts = 1;
	m_pD3D10Device->RSGetViewports(&numViewPorts,&vp);
	//=================================================================================

	//=========CREATES THE PROJECTION MATRIX=========
	//D3DXMATRIX IS A POINTER TO A MATRIX
	//2ND PARAM = THE FOV
	//3RD PARAM = THE WINDOW WIDTH AND HEIGHT
	//4TH PARAM = THE NEAR CLIP PLANE
	//5TH PARAM = THE FAR CLIP PLANE
	D3DXMatrixPerspectiveFovLH(&m_matProjection,(float)D3DX_PI*0.25f,vp.Width/(float)vp.Height,0.1f,100.0f);
	//===============================================


	//=========RETRIEVE THE EFFECT VARIABLES SO THEY CAN BE SENT TO TH EEFFECT
	m_pViewMatrixVariable = 
		m_pEffect->GetVariableByName("matView")->AsMatrix();
	m_pProjectionMatrixVariable =
		m_pEffect->GetVariableByName("matProjection")->AsMatrix();
	//===========================================================================

	m_pProjectionMatrixVariable->SetMatrix((float*)m_matView);
	
	//=========USED TO SET THE SET THE POS,ROT AND SCALE VECTORS OF THE OBJECT
	m_vecPosition = D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vecScale = D3DXVECTOR3(1.0f,1.0f,1.0f);
	m_vecRotation = D3DXVECTOR3(0.0f,0.0f,0.0f);
	//===========================================================================

	//=========USED TO RETRIEVE THE WORLD MATRIX FROM THE EFFECT AND SEND IT TO THE EFFECT.
	m_pWorldMatrixVariable=
		m_pEffect->GetVariableByName("matWorld")->AsMatrix();
	//=====================================================================================

	return true;


}
//====================================================================================

//=========THIS METHOD CONTAINS THE LOOP WHICH CHECKS FOR========= 
//=========WINDOWS MESSAGES, UPDATES AND RENDERS THE SCENE=========
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
//==================================================================


//=========USED TO UPDATE THE GAMESTATE=========
void CGameApplication::update()
{
	D3DXMatrixScaling(&m_matScale,m_vecScale.x,m_vecScale.y,m_vecScale.z);

	D3DXMatrixRotationYawPitchRoll(&m_matRotation,m_vecRotation.y,m_vecRotation.x,m_vecRotation.z);

	D3DXMatrixTranslation(&m_matTranslation,m_vecPosition.x,m_vecPosition.y,m_vecPosition.z);

	D3DXMatrixMultiply(&m_matWorld,&m_matScale,&m_matRotation);

	D3DXMatrixMultiply(&m_matWorld,&m_matWorld,&m_matTranslation);

}
//==============================================


//=========USED TO INITIALIZE DIRECT3D10=========
bool CGameApplication::initGraphics()
{
	RECT windowRect;

	//===============GETS THE SIZE OF THE CURRENT WINDOW=========
	GetClientRect(m_pWindow->getHandleToWindow(),&windowRect);
	//============================================================

	//=========STORES THE WIDTH AND HEIGHT OF THE WINDOW USING THE CURRENT DIMENSIONS=========
	UINT width = windowRect.right-windowRect.left;
	UINT height = windowRect.bottom-windowRect.top;
	//=======================================================================================


	//=========CREATING THE DEVICE========= 
	UINT createDeviceFlags = 0;		//USED TO HOLD FLAGS FOR DEVICE CREATION
#ifdef _DEBUG	//CHECKS TO SEE IF DEV ENVIRONMENT IS IN DEBUG MODE
		createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif
	//======================================


		DXGI_SWAP_CHAIN_DESC sd;	//HOLDS ALL OPTIONS FOR THE CREATION OF THE SWAP CHAIN
		ZeroMemory( &sd, sizeof(sd));	//TAKES IN A MEM ADDRESS AND SIZE OF VARIABLE AND DEFAULTS THIS TO ZERO

		//=========CHECKS THE STATE OF THE CURRENT WINDOW=========
		
		if (m_pWindow->isFullScreen())	//IF FULLCREEN THEN THERE ARE 2 BUFFERS- FRONT AND BACK
			sd.BufferCount = 2;
		else
			sd.BufferCount = 1;		// IF NOT THEN WE HAVE ONE- THE DESKTOP ACTS AS THE BACKBUFFER

		//=======================================================================================
		
		
		sd.OutputWindow = m_pWindow->getHandleToWindow();	//ASSIGNS WIN HANDLE TO THE SWAPCHAIN.
		sd.Windowed =(BOOL)(!m_pWindow->isFullScreen());	//SPECIFIES IF WE ARE IN WINDOWED MODE OR NOT USING THE NOT(!)OPERATOR.
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//STATES THAT BUFFER WILL BE USED AS A RENDER TARGET( CAN BE DRAWN TO.

		//=========ANTIALISING(MULTISAMPLING) PARAMS=========
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;	//SET TO 0 = OFF FOR PERFORMANCE ISSUES
		//===================================================

		//=========SETS THE OPTIONS FOR THE BUFFER(S) INSIDE THE SWAP CHAIN

		sd.BufferDesc.Width = width;	//SETS THE WIDTH OF THE BUFFER- EQUAL TO THE WINDOW
		sd.BufferDesc.Height = height;	//SETS THE HEIGHT OF THE BUFFER- EQUAL TO THE WINDOW
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//SETS THE FORMAT OF THE BUFFER,  8 BYTES FOR EACH (rgb)
		sd.BufferDesc.RefreshRate.Numerator=60;		// SETS THE REFRESH RATE TO 60 HERTZ USING AN UPDATE ON THE VERT BLANK
		sd.BufferDesc.RefreshRate.Denominator=1;	//""	""	""	""	""	""	""	
		//==================================================================
		

		//=========THIS FUNCTION WILL CREATE A SWAP CHAIN AND DEVICE WITHIN ONE CALL, SURROUNDING IN IF-FAILED TO CHECK FOR FAILURE=========

		//1ST PARAM = A POINTER TO AN IDXGIAPADTER, WHICH IS ANALOGOUS TO A PHYS GRAPH CARDS, NULL WILL USE THE DEFAULT.
		//2ND PARAM = THE TYPE OF DRIVER FLAG- IN THIS CASE THE HARDWARE DEVICE.
		//3RD PARAM = A HANDLE TO A MODULE(DLL)CONTAINING SOFT IMPLEMENATION OF D3D10, ALMST ALWAYS NULL
		//4TH PARAM = OPTIONAL, USED TO GIVE ADDITIONAL OPTIONS UPON DEVICE CREATION, IN THIS INSTANCE PUTTING DEVICE INTO DEBUG.
		//5TH PARAM =  THE VERSION OF D3D10 WE ARE USING
		//6TH PARAM = A POINTER TO THE SWAP CHAIN DESC' USING MEMORY ADD OPERATOR(&). HOLDS THE OPTIONS FOR THE SWAP CHAIN CREATION
		//7TH PARAM = AN ADDRESS OF A POINTER TO THE SWAP CHAIN INTERFACE , USED TO INITIALIZE THE IDXGISWAP CHAIN POINTER
		//8th PARAM = AN ""		""	""	""	""	""	""	""D3D10 DEVICE, ""		""		""	THE ID3D10DEVICE POINTER
		if(FAILED(D3D10CreateDeviceAndSwapChain(NULL,D3D10_DRIVER_TYPE_HARDWARE,NULL,createDeviceFlags,
			D3D10_SDK_VERSION,&sd,&m_pSwapChain,&m_pD3D10Device)))
			return false;
		//=================================================================================================================================

		//=========ASSOCIATE A BUFFER FROM THE SWAP CHAIN WITH THE RENDER TARGET VIEW =========

		ID3D10Texture2D *pBackBuffer;	//THE BUFFER IS OF TYPE TEXTURE 2D, A 3D ARRAY OF TEXELS, (TEXTURE PIXELS).
		
		//=========GET BUFFER FUNCTION IS USED TO RETRIEVE THE AFORMENTIONED BUFFER=========
		//1ST PARAM = INDEX OF THE BUFFER WITHIN SWAP CHAIN, ZERO WILL RETRIEVE BACKBUFFER
		//2ND PARAM = ID OF THE INTERCFACE WE ARE RETRIEVING FROM SWAP CHAIN, uuiodf FUNCTION RETRIEVES THE UNIQUE ID OF AN INTERFACE
		//3RD PARAM = A POINTER TO ADDRESS OF A BUFFER, VOID BECAUSE THE FUNCTION CAN TAKE ANY TYPE DEPENDING ON VALUE OF 2ND PARAM
		if(FAILED(m_pSwapChain->GetBuffer(0,__uuidof(ID3D10Texture2D),(void**)&pBackBuffer)))
			return false;
		//=======================================================================================


		//=========CREATES THE RENDER TARGET VIEW=========
		//1ST PARAM = POINTER TO A RESOURCE, TEXTURE2D INTERFACE INHERITS FROM THIS SO IT CAN BE PASSED TO THIS FUNCTION.
		//2ND PARAM = POINTER TO A STRUCT WHICH DEFINES OPTIONS FOR ACCESSING COMPONENTS OF RENDER TARGET I.E SUB AREAS
		//3RD PARAM =POINTER TO ADDRESS OF RENDER TARGET VIEW
		if(FAILED(m_pD3D10Device->CreateRenderTargetView(pBackBuffer,NULL,&m_pRenderTargetView)))
		{
			pBackBuffer->Release();
			return false;
		}
		pBackBuffer->Release();


		D3D10_TEXTURE2D_DESC descDepth;

		//CREATING THE DEPTH BUFFER-  USED TO MOVE TO 3D SCENE. DEPTH STENCIL IS USED TO BIND THIS TO THE PIPELINE

		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;  //Format of the texture--- will hold 32bit floating point
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D10_USAGE_DEFAULT;
		descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;  //how the buffer will be bound to the pipeline.
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;

		//Used to Create the texture
		if(FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,&m_pDepthStencilTexture)))
			return false;

		D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		
		if(FAILED(m_pD3D10Device->CreateDepthStencilView(
			m_pDepthStencilTexture,&descDSV,&m_pDepthStencilView)))
			return false;



		///=========BINDS AN ARRAY OF RENDER TARGETS TO OUTPUT MERGER STAGE=========
		//1ST PARAM = THE AMOUNT OF RENDER TARGETS TO BE BOUND TO PIPELINE
		//2ND PARAM = A POINTER TO AN ARRAY OF RENDER TARGETS
		//3RD PARAM = POINTER TO DEPTH STENCIL VIEW, USED TO HOLD DEPTH INFO FOR SCENCE
		m_pD3D10Device->OMSetRenderTargets(1,&m_pRenderTargetView,m_pDepthStencilView);
		//==============================================================================

		//=========SETTING UP A VIEWPORT ALLOWING OBJECTS TO BE ALLIGNED CORRECTLY=========
		D3D10_VIEWPORT vp;	//INSTANCE OF D3D10VIEWPORT
		vp.Width = width;	//SAME AS WINDOW
		vp.Height = height;	//SAM AS WINDOW
		vp.MinDepth = 0.0f;	
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0; 
		m_pD3D10Device->RSSetViewports(1,&vp);	//SETS VIEWPORT WHICH IS BOUND TO PIPELINE
		//==============================================================================
	return true;
}
//===============================================


//=========USED TO INSTANCIATE AND MANAGE THE DRAWING=========
void CGameApplication::render()
{

	float clearColor[4] = {0.0f,0.125f,0.3f,1.0f};	//SETTING AN FLOAT ARRAY OF COLOURS(rgb&ALPHA)WITH VALUES FROM 0-1 FOR EACH
	m_pD3D10Device->ClearRenderTargetView(m_pRenderTargetView,clearColor);	//USES THE VAR ABOVE TO SET THE RENDER TARGET COLOUR

	//=========USED TO CLEAR THE DEPTH BUFFER=========
	//1ST PARAM = A POINTER TO THE STENCIL VIEW
	//2ND PARAM = THE CLEAR FLAGS
	//3RD PARAM = THE VAL TO CLEAR DEPTH BUFFER WITH
	//4TH PARAM = THE VAL TO CLEAR THE STENC BUFFER WITH
	m_pD3D10Device->ClearDepthStencilView(m_pDepthStencilView,D3D10_CLEAR_DEPTH,1.0f,0);
	//==================================================

	m_pD3D10Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//=========DRAW CALL'S MUST GO WITHIN HERE=========

	m_pProjectionMatrixVariable->SetMatrix((float*)m_matView);	//SENDS THE VIEW MATRIX TO THE EFFECT

	m_pWorldMatrixVariable->SetMatrix((float*)m_matWorld);

	D3D10_TECHNIQUE_DESC techniqueDesc;
	m_pTechnique->GetDesc(&techniqueDesc);

	for(UINT p = 0; p<techniqueDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0);
		m_pD3D10Device->Draw(3,0);
	}
	m_pSwapChain->Present(0,0);	//USED TO FLIP THE SWAP CHAIN

	
}
//============================================================

//=========USED TO INITIALIZE  A WINDOW=========
bool  CGameApplication::initWindow()
{
	
	m_pWindow=new CWin32Window();	//A new instanxe of the win32Window

	//=========CALLING THE INIT FUNCTION TO CREATE THE WINDOW=========
	if (!m_pWindow->init(TEXT("Lab1-Create Device"),800,640,false))
		return false;
	//==================================================================

	return true;
}
//=============================================

