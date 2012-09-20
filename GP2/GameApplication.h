#pragma once 

#include "Win32Window.h"
#include <D3D10.h>
#include <D3DX10.h>

class CGameApplication
{
public:
	CGameApplication(void);
	~CGameApplication(void);

	bool init();
	void run();
	bool initGame();

private:
	bool initGraphics();
	bool initWindow();
	void render();
	void update();

private: 
	ID3D10Device * m_pD3D10Device;
	IDXGISwapChain * m_pSwapChain;
	ID3D10RenderTargetView * m_pRenderTargetView;
	ID3D10EffectTechnique*	m_pTechnique;

	ID3D10DepthStencilView * m_pDepthStencilView;
	ID3D10Texture2D * m_pDepthStenciTexture;

	CWin32Window * m_pWindow;
	ID3D10Buffer*	m_pVertexBuffer;
	ID3D10InputLayout*	m_pVertexLayout;
	ID3D10Effect*	m_pEffect;
	

};