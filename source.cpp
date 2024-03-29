// https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-1-features?redirectedfrom=MSDN#use-direct3d-in-session-0-processes
/*
* Use Direct3D in Session 0 processes
*
*	Starting with Windows 8 and Windows Server 2012, you can use most of the Direct3D APIs in Session 0 processes.
*
*	Note
*	These output, window, swap chain, and presentation-related APIs are not available in Session 0 processes because they don't apply to the Session 0 environment:
*		IDXGIFactory::CreateSwapChain
*		IDXGIFactory::GetWindowAssociation
*		IDXGIFactory::MakeWindowAssociation
*		IDXGIAdapter::EnumOutputs
*		ID3D11Debug::SetFeatureMask
*		ID3D11Debug::SetPresentPerRenderOpDelay
*		ID3D11Debug::SetSwapChain
*		ID3D10Debug::SetFeatureMask
*		ID3D10Debug::SetPresentPerRenderOpDelay
*		ID3D10Debug::SetSwapChain
*		ID3D10Debug::SetSwapChain
*		ID3D10Debug::SetSwapChain
*		D3D11CreateDeviceAndSwapChain
*
*	If you call one of the preceding APIs in a Session 0 process, it returns DXGI_ERROR_NOT_CURRENTLY_AVAILABLE.
*/

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl\client.h>

#pragma comment (lib, "d3d12")
#pragma comment (lib, "dxgi")

using namespace Microsoft::WRL;

#if defined(UNICODE) || defined(_UNICODE)
#define _tWinMain wWinMain
#else
#define _tWinMain WinMain
#endif


ComPtr<ID3D12Device9> device;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12GraphicsCommandList6> commandList;
ComPtr<ID3D12DescriptorHeap> descriptorHeap;

ComPtr<IDXGIFactory7> factory;
ComPtr<IDXGISwapChain1> swapChain;

void KeyDown(UINT8 key)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		exit(0);
}

void KeyUp(UINT8 key)
{

}

void Clear()
{
	device.ReleaseAndGetAddressOf();
	commandQueue.ReleaseAndGetAddressOf();
	commandAllocator.ReleaseAndGetAddressOf();
	pipelineState.ReleaseAndGetAddressOf();
	commandList.ReleaseAndGetAddressOf();
	descriptorHeap.ReleaseAndGetAddressOf();
	factory.ReleaseAndGetAddressOf();
	swapChain.ReleaseAndGetAddressOf();
}

void Update()
{

}

void Render()
{
	const float Color[] = { 0.4f, 0.6f, 0.9f, 1.0f };
	commandList->ClearRenderTargetView(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), Color, 0, nullptr);
	commandList->Close();

	ID3D12CommandList* pCommandList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(pCommandList), pCommandList);

	swapChain->Present(1, 0);
}

void Init(_In_ HWND hWnd)
{
	if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
		goto Continue;
	else {
		MessageBox(nullptr, TEXT("Your GPU doesn't support D3D_FEATURE_LEVEL_11_0 or higher."), TEXT("Error"), MB_ICONERROR);
		exit(-1);
	}

Continue:
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

	DXGI_SWAP_CHAIN_DESC1 desc1{};
	desc1.BufferCount = 2;
	desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc1.SampleDesc.Count = 1;

	CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc1, nullptr, nullptr, &swapChain);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	ComPtr<ID3D12Resource2> resource;
	swapChain->GetBuffer(NULL, IID_PPV_ARGS(&resource));
	device->CreateRenderTargetView(resource.Get(), nullptr, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	resource.ReleaseAndGetAddressOf();

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

	device->CreateCommandList(NULL, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList));
}

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		Init(hWnd);
		return 0;

	case WM_KEYDOWN:
		KeyDown(static_cast<UINT8>(wParam));
		return 0;

	case WM_KEYUP:
		KeyUp(static_cast<UINT8>(wParam));
		return 0;

	case WM_PAINT:
		Update();
		Render();
		return 0;

	case WM_DESTROY:
		Clear();
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PTCHAR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASS wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = TEXT("WindowClass");
	RegisterClass(&wc);

	auto hWnd = CreateWindow(wc.lpszClassName, TEXT("Test DirectX 12 for D3D12CreateDevice"),
		WS_SYSMENU | WS_MINIMIZEBOX,
		(GetSystemMetrics(SM_CXSCREEN) - 1024) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - 576) / 2,
		1024, 576, nullptr, nullptr, hInstance, NULL);

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	MSG msg{};
	while (msg.message != WM_QUIT)
		if (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	return (int)msg.wParam;
}
