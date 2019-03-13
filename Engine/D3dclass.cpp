/////////////////////////////
// Filename : d3dclass.cpp //
/////////////////////////////
#include "D3dclass.h"

D3DClass::D3DClass()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}

D3DClass::D3DClass(const D3DClass & other)
{

}

D3DClass::~D3DClass()
{

}

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	HRESULT result; 
	IDXGIFactory* factory; 
	IDXGIAdapter* adapter; 
	IDXGIOutput* adapterOutput; 
	unsigned int numModes, i, numerator, denominator, stringLength; 
	DXGI_MODE_DESC* displayModeList; 
	DXGI_ADAPTER_DESC adapterDesc; 
	int error; 
	DXGI_SWAP_CHAIN_DESC swapChainDesc; 
	D3D_FEATURE_LEVEL featureLevel; 
	ID3D11Texture2D* backBufferPtr; 
	D3D11_TEXTURE2D_DESC depthBufferDesc; 
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc; 
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; 
	D3D11_RASTERIZER_DESC rasterDesc; 
	D3D11_VIEWPORT viewport; 
	float fieldOfView, screenAspect; 
	
	// vsync(��������ȭ) ������ �����մϴ�. 
	m_vsync_enabled = vsync; 
	
	// DX �׷��� �������̽� ���丮�� ����ϴ�.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// ���丮 ��ä�� ����Ͽ� ù��° �׷��� ī�� �������̽��� ���� �ƴ��͸� ����ϴ�.
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// ���(�����)�� ���� ù��° �ƴ��͸� �����մϴ�.
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// DXGI_FORMAT_R8G8B8A8_UNORM ����� ��� ���÷��� ���˿� �´� ����� ������ ���մϴ�.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// ���� ��� ���÷��� ��忡 ���� ȭ�� �ʺ�/���̿� �´� ���÷��� ��带 ã���ϴ�.
	// ������ ���� ã���� ������� ���ΰ�ħ ������ �и�� ���� ���� �����մϴ�.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// �����(�׷���ī��)�� description�� �����ɴϴ�.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// ���� �׷���ī���� �޸� �뷮�� �ް�����Ʈ ������ �����մϴ�.
	m_videoCardMemory = (int / )(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// �׷���ī���� �̸��� char�� ���ڿ� �迭�� �ٲ� �� �����մϴ�.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// ���÷��� ��� ����Ʈ�� �Ҵ��� �����մϴ�.
	delete[] displayModeList;
	displayModeList = 0;

	// ��� �ƴ��͸� �Ҵ� �����մϴ�.
	adapterOutput->Release();
	adapterOutput = 0;

	// �ƴ��͸� �Ҵ� �����մϴ�.
	adapter->Release();
	adapter = 0;

	// ���丮 ��ü�� �Ҵ� �����մϴ�.
	factory->Release();
	factory = 0;

	// ���� ü�� decription�� �ʱ�ȭ�մϴ�.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// �ϳ��� ����۸��� ����ϵ��� �մϴ�.
	swapChainDesc.BufferCount = 1;

	// ������� �ʺ�� ���̸� �����մϴ�.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// ����۷� �Ϲ����� 32bit�� �����̽��� �����մϴ�.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// ������� ���ΰ�ħ ������ �����մϴ�.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// ������� �뵵�� �����մϴ�.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// �������� �̷���� �������� �ڵ��� �����մϴ�.
	swapChainDesc.OutputWindow = hwnd;

	// ��Ƽ���ø��� ���ϴ�.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// ������ ��� �Ǵ� Ǯ��ũ�� ��带 �����մϴ�.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// ��ĵ������ ���İ� ��ĵ���̴��� �������� �������� �����մϴ�.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// ��µ� ������ ������� ������ �������� �մϴ�.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// �߰� �ɼ� �÷��׸� ������� �ʽ��ϴ�.
	swapChainDesc.Flags = 0;

	// ���� ������ DX 11�� �����մϴ�.
	featureLevel = D3D_FEATURE_LEVEL_11_0;
}


