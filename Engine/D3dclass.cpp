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
	
	// vsync(수직동기화) 설정을 저장합니다. 
	m_vsync_enabled = vsync; 
	
	// DX 그래픽 인터페이스 팩토리를 만듭니다.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// 팩토리 객채를 사용하여 첫번째 그래픽 카드 인터페이스에 대한 아답터를 만듭니다.
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// 출력(모니터)에 대한 첫번째 아답터를 나열합니다.
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포맷에 맞는 모드의 개수를 구합니다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 이제 모든 디스플레이 모드에 대해 화면 너비/높이에 맞는 디스플레이 모드를 찾습니다.
	// 적합한 것을 찾으면 모니터의 새로고침 비율의 분모와 분자 값을 저장합니다.
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

	// 어댑터(그래픽카드)의 description을 가져옵니다.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// 현재 그래픽카드의 메모리 용량을 메가바이트 단위로 저장합니다.
	m_videoCardMemory = (int / )(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// 그래픽카드의 이름을 char형 문자열 배열로 바꾼 뒤 저장합니다.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// 디스플레이 모드 리스트의 할당을 해제합니다.
	delete[] displayModeList;
	displayModeList = 0;

	// 출력 아답터를 할당 해제합니다.
	adapterOutput->Release();
	adapterOutput = 0;

	// 아답터를 할당 해제합니다.
	adapter->Release();
	adapter = 0;

	// 팩토리 객체를 할당 해제합니다.
	factory->Release();
	factory = 0;

	// 스왑 체인 decription을 초기화합니다.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// 하나의 백버퍼만을 사용하도록 합니다.
	swapChainDesc.BufferCount = 1;

	// 백버퍼의 너비와 높이를 설정합니다.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// 백버퍼로 일반적인 32bit의 서페이스를 지정합니다.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 백버퍼의 새로고침 비율을 설정합니다.
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

	// 백버퍼의 용도를 설정합니다.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 렌더링이 이루어질 윈도우의 핸들을 설정합니다.
	swapChainDesc.OutputWindow = hwnd;

	// 멀티샘플링을 끕니다.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// 윈도우 모드 또는 풀스크린 모드를 설정합니다.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// 스캔라인의 정렬과 스캔라이닝을 지정하지 않음으로 설정합니다.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 출력된 이후의 백버퍼의 내용을 버리도록 합니다.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 추가 옵션 플래그를 사용하지 않습니다.
	swapChainDesc.Flags = 0;

	// 피쳐 레밸을 DX 11로 설정합니다.
	featureLevel = D3D_FEATURE_LEVEL_11_0;
}


