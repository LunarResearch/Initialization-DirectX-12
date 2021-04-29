#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl\client.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")

using namespace Microsoft::WRL;

ComPtr<ID3D12Device8> device8;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12GraphicsCommandList6> commandList6;
ComPtr<ID3D12DescriptorHeap> descriptorHeap;

ComPtr<IDXGIFactory7> factory7;
ComPtr<IDXGISwapChain1> swapChain1;

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
	device8.ReleaseAndGetAddressOf();
	commandQueue.ReleaseAndGetAddressOf();
	commandAllocator.ReleaseAndGetAddressOf();
	pipelineState.ReleaseAndGetAddressOf();
	commandList6.ReleaseAndGetAddressOf();
	descriptorHeap.ReleaseAndGetAddressOf();
	factory7.ReleaseAndGetAddressOf();
	swapChain1.ReleaseAndGetAddressOf();
}

void Update()
{

}

void Render()
{
	const float Color[] = { 0.4f, 0.6f, 0.9f, 1.0f };
	commandList6->ClearRenderTargetView(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), Color, 0, nullptr);
	commandList6->Close();

	ID3D12CommandList* pCommandList[] = { commandList6.Get() };
	commandQueue->ExecuteCommandLists(_countof(pCommandList), pCommandList);

	swapChain1->Present(1, 0);
}

void Init(_In_ HWND hWnd)
{
	if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device8))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device8))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device8))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device8))))
		goto Continue;
	else {
		MessageBox(nullptr, TEXT("Your GPU doesn't support D3D_FEATURE_LEVEL_11_0 or higher."), TEXT("Error"), MB_ICONERROR);
		exit(-1);
	}

Continue:
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	device8->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

	DXGI_SWAP_CHAIN_DESC1 desc1{};
	desc1.BufferCount = 2;
	desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc1.SampleDesc.Count = 1;

	CreateDXGIFactory1(IID_PPV_ARGS(&factory7));
	factory7->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc1, nullptr, nullptr, &swapChain1);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device8->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	ComPtr<ID3D12Resource2> resource2;
	swapChain1->GetBuffer(NULL, IID_PPV_ARGS(&resource2));
	device8->CreateRenderTargetView(resource2.Get(), nullptr, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	resource2.ReleaseAndGetAddressOf();

	device8->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	device8->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

	device8->CreateCommandList(NULL, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList6));
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

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PTCHAR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = TEXT("WindowClass");
	RegisterClassEx(&wc);

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
