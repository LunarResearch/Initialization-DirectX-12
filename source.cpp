#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl\client.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")

using namespace Microsoft::WRL;

ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12DescriptorHeap> descriptorHeap;

ComPtr<IDXGIFactory4> factory4;

ComPtr<IDXGISwapChain> swapChain;
ComPtr<IDXGISwapChain3> swapChain3;

void KeyDown(UINT8 key) {
	if (GetAsyncKeyState(VK_ESCAPE))
		exit(0);
}

void KeyUp(UINT8 key) {

}

void Clear() {
	device.ReleaseAndGetAddressOf();
	commandQueue.ReleaseAndGetAddressOf();
	commandAllocator.ReleaseAndGetAddressOf();
	pipelineState.ReleaseAndGetAddressOf();
	commandList.ReleaseAndGetAddressOf();
	descriptorHeap.ReleaseAndGetAddressOf();
	factory4.ReleaseAndGetAddressOf();
	swapChain.ReleaseAndGetAddressOf();
	swapChain3.ReleaseAndGetAddressOf();
}

void Update() {

}

void Render() {
	const float Color[] = { 0.4f, 0.6f, 0.9f, 1.0f };
	commandList->ClearRenderTargetView(descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		Color, 0, nullptr);
	commandList->Close();

	ID3D12CommandList *pCommandList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(pCommandList), pCommandList);

	DXGI_PRESENT_PARAMETERS pres = {};
	swapChain3->Present1(1, 0, &pres);
}

void Init(_In_ HWND hWnd) {
	if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device))))
		goto Continue;
	else if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
		goto Continue;
	else {
		MessageBox(NULL, "Your GPU doesn't support D3D_FEATURE_LEVEL_11_0 or higher.",
			"Error", MB_ICONERROR);
		exit(-1);
	}

Continue:
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferCount = 2;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.OutputWindow = hWnd;
	desc.SampleDesc.Count = 1;
	desc.Windowed = TRUE;

	CreateDXGIFactory1(IID_PPV_ARGS(&factory4));
	factory4->CreateSwapChain(commandQueue.Get(), &desc, &swapChain);
	
	factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	ComPtr<ID3D12Resource> resource;
	swapChain.As(&swapChain3);
	swapChain3->GetBuffer(0, IID_PPV_ARGS(&resource));
	device->CreateRenderTargetView(resource.Get(), nullptr,
		descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	resource.ReleaseAndGetAddressOf();

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(),
		pipelineState.Get(), IID_PPV_ARGS(&commandList));
}

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	switch (uMsg) {
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
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "WindowClass";
	RegisterClassEx(&wc);
	HWND hWnd = CreateWindow(wc.lpszClassName, "Test DirectX 12 for D3D12CreateDevice",
		WS_SYSMENU | WS_MINIMIZEBOX,
		(GetSystemMetrics(SM_CXSCREEN) - 1024) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - 576) / 2,
		1024, 576, nullptr, nullptr, hInstance, 0);
	ShowWindow(hWnd, nCmdShow);
	MSG msg = {};
	while (msg.message != WM_QUIT)
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	return static_cast<char>(msg.wParam);
}
