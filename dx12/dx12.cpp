#include "dx12.h"

namespace Dx12
{
    D3D12_CPU_DESCRIPTOR_HANDLE DeviceDx12::GetRtv(UINT i)
    {
        auto dh = rtv_heap->GetCPUDescriptorHandleForHeapStart();
        dh.ptr += rtv_size * static_cast<unsigned long long>(i);
        return dh;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DeviceDx12::GetDsv(UINT i)
    {
        auto dh = dsv_heap->GetCPUDescriptorHandleForHeapStart();
        dh.ptr += dsv_size * static_cast<unsigned long long>(i);
        return dh;
    }

    bool DeviceDx12::FlushCmdQueue()
    {
        HRESULT hr = S_OK;

        UINT64 v = fence->GetCompletedValue() + 1;
        hr = cmd_queue->Signal(fence.Get(), v);
        if (FAILED(hr))
            return false;
        if (fence->GetCompletedValue() < v)
        {
            HANDLE event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
            if (!event)
                return false;
            hr = fence->SetEventOnCompletion(v, event);
            if (FAILED(hr))
                return false;
            WaitForSingleObject(event, INFINITE);
            CloseHandle(event);
        }

        return true;
    }

    DeviceDx12::DeviceDx12()
    {
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sc_buffer_count = 0;
        rtv_size = 0;
        dsv_size = 0;
        cbv_size = 0;
        ZeroMemory(&vp, sizeof(vp));
        ZeroMemory(&sr, sizeof(sr));
    }

    DeviceDx12::~DeviceDx12()
    {
        FlushCmdQueue();
    }

    std::shared_ptr<DeviceDx12> DeviceDx12::CreateDevice(Rehenz::SimpleWindow* window)
    {
        HRESULT hr = S_OK;

        std::shared_ptr<DeviceDx12> p(new DeviceDx12);

#ifdef _DEBUG
        // enable debug layer
        ComPtr<ID3D12Debug3> debugger;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(debugger.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        debugger->EnableDebugLayer();
#endif

        // create factory
        ComPtr<IDXGIFactory7> factory;
#ifdef _DEBUG
        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(factory.GetAddressOf()));
#else
        hr = CreateDXGIFactory2(0, IID_PPV_ARGS(factory.GetAddressOf()));
#endif
        if (FAILED(hr))
            return nullptr;

        // create device
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(p->device.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // create fence
        hr = p->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(p->fence.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // create command queue and list
        D3D12_COMMAND_QUEUE_DESC cmd_queue_desc;
        cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cmd_queue_desc.Priority = 0;
        cmd_queue_desc.NodeMask = 0;
        hr = p->device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(p->cmd_queue.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        hr = p->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(p->cmd_alloc.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        hr = p->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p->cmd_alloc.Get(), nullptr, IID_PPV_ARGS(p->cmd_list.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // create swap chain
        p->format = DXGI_FORMAT_R8G8B8A8_UNORM;
        p->sc_buffer_count = 2;
        DXGI_SWAP_CHAIN_DESC sc_desc;
        sc_desc.BufferDesc.Width = window->GetWidth();
        sc_desc.BufferDesc.Height = window->GetHeight();
        sc_desc.BufferDesc.RefreshRate.Numerator = 60;
        sc_desc.BufferDesc.RefreshRate.Denominator = 1;
        sc_desc.BufferDesc.Format = p->format;
        sc_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sc_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sc_desc.SampleDesc.Count = 1;
        sc_desc.SampleDesc.Quality = 0;
        sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sc_desc.BufferCount = p->sc_buffer_count;
        sc_desc.OutputWindow = window->GetHwnd();
        sc_desc.Windowed = true;
        sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sc_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        ComPtr<IDXGISwapChain> sc0;
        hr = factory->CreateSwapChain(p->cmd_queue.Get(), &sc_desc, sc0.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        hr = sc0.As(&p->sc);
        if (FAILED(hr))
            return nullptr;

        // create descriptor heap
        p->rtv_size = p->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        p->dsv_size = p->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        p->cbv_size = p->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_DESCRIPTOR_HEAP_DESC dh_desc;
        dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dh_desc.NumDescriptors = p->sc_buffer_count;
        dh_desc.NodeMask = 0;
        hr = p->device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(p->rtv_heap.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dh_desc.NumDescriptors = 1;
        dh_desc.NodeMask = 0;
        hr = p->device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(p->dsv_heap.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // create render target view
        p->rtv_buffers.resize(p->sc_buffer_count);
        for (UINT i = 0; i < p->sc_buffer_count; i++)
        {
            hr = p->sc->GetBuffer(i, IID_PPV_ARGS(p->rtv_buffers[i].GetAddressOf()));
            if (FAILED(hr))
                return nullptr;
            p->device->CreateRenderTargetView(p->rtv_buffers[i].Get(), nullptr, p->GetRtv(i));
        }

        // create depth stencil view
        D3D12_RESOURCE_DESC rc_desc;
        rc_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rc_desc.Alignment = 0;
        rc_desc.Width = window->GetWidth();
        rc_desc.Height = window->GetHeight();
        rc_desc.DepthOrArraySize = 1;
        rc_desc.MipLevels = 1;
        rc_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        rc_desc.SampleDesc.Count = 1;
        rc_desc.SampleDesc.Quality = 0;
        rc_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        rc_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        D3D12_HEAP_PROPERTIES heap_prop;
        heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_prop.CreationNodeMask = 1;
        heap_prop.VisibleNodeMask = 1;
        D3D12_CLEAR_VALUE clear_val;
        clear_val.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        clear_val.DepthStencil.Depth = 1.0f;
        clear_val.DepthStencil.Stencil = 0;
        hr = p->device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_COMMON, &clear_val, IID_PPV_ARGS(p->dsv_buffer.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        p->device->CreateDepthStencilView(p->dsv_buffer.Get(), nullptr, p->GetDsv());

        // change dsv -> depth write
        D3D12_RESOURCE_BARRIER rc_barr;
        rc_barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rc_barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        rc_barr.Transition.pResource = p->dsv_buffer.Get();
        rc_barr.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        rc_barr.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        rc_barr.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        p->cmd_list->ResourceBarrier(1, &rc_barr);

        // set viewport
        p->vp.TopLeftX = 0;
        p->vp.TopLeftY = 0;
        p->vp.Width = static_cast<float>(window->GetWidth());
        p->vp.Height = static_cast<float>(window->GetHeight());
        p->vp.MinDepth = 0;
        p->vp.MaxDepth = 1;
        p->cmd_list->RSSetViewports(1, &p->vp);

        // set scissor rect
        p->sr.left = 10;
        p->sr.right = window->GetWidth() / 10;
        p->sr.top = 10;
        p->sr.bottom = window->GetHeight() / 10;
        p->cmd_list->RSSetScissorRects(1, &p->sr);

        // finish command list
        hr = p->cmd_list->Close();
        if (FAILED(hr))
            return nullptr;

        // execute commands
        ID3D12CommandList* lists[]{ p->cmd_list.Get() };
        p->cmd_queue->ExecuteCommandLists(1, lists);

        return p;
    }

    bool DeviceDx12::Present()
    {
        HRESULT hr = S_OK;

        // reset command queue
        if (!FlushCmdQueue())
            return false;
        hr = cmd_alloc->Reset();
        if (FAILED(hr))
            return false;
        hr = cmd_list->Reset(cmd_alloc.Get(), nullptr);
        if (FAILED(hr))
            return false;

        // change rtv -> render target
        D3D12_RESOURCE_BARRIER rc_barr;
        rc_barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rc_barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        rc_barr.Transition.pResource = GetCurrentRtvBuffer();
        rc_barr.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        rc_barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        rc_barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        cmd_list->ResourceBarrier(1, &rc_barr);

        // set viewport & scissor rect
        cmd_list->RSSetViewports(1, &vp);
        cmd_list->RSSetScissorRects(1, &sr);

        // clear
        float bg_color[4]{ 0.7804f,0.8627f,0.4078f,0 };
        cmd_list->ClearRenderTargetView(GetCurrentRtv(), bg_color, 0, nullptr);
        cmd_list->ClearDepthStencilView(GetDsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        // set render target
        auto rtv = GetCurrentRtv();
        auto dsv = GetDsv();
        cmd_list->OMSetRenderTargets(1, &rtv, true, &dsv);

        // change rtv -> present
        rc_barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rc_barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        rc_barr.Transition.pResource = GetCurrentRtvBuffer();
        rc_barr.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        rc_barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        rc_barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        cmd_list->ResourceBarrier(1, &rc_barr);

        // finish command list
        hr = cmd_list->Close();
        if (FAILED(hr))
            return false;

        // execute commands
        ID3D12CommandList* lists[]{ cmd_list.Get() };
        cmd_queue->ExecuteCommandLists(1, lists);

        // swap
        hr = sc->Present(0, 0);
        if (FAILED(hr))
            return false;

        return true;
    }

    D3D_FEATURE_LEVEL DeviceDx12::CheckMaxFeatureLevel()
    {
        HRESULT hr = S_OK;

        D3D12_FEATURE_DATA_FEATURE_LEVELS levels;
        std::vector<D3D_FEATURE_LEVEL> fls{
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_1_0_CORE };
        levels.pFeatureLevelsRequested = &fls[0];
        levels.NumFeatureLevels = static_cast<UINT>(fls.size());
        hr = device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &levels, sizeof(levels));
        if (FAILED(hr))
            return static_cast<D3D_FEATURE_LEVEL>(0);

        return levels.MaxSupportedFeatureLevel;
    }

    UINT DeviceDx12::CheckMsaa4xQuality()
    {
        HRESULT hr = S_OK;

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels;
        levels.Format = format;
        levels.SampleCount = 4;
        levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        hr = device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels));
        if (FAILED(hr))
            return 0;

        return levels.NumQualityLevels;
    }

    void DeviceDx12::PrintSupportInfo(std::ostream& out)
    {
        D3D_FEATURE_LEVEL level = CheckMaxFeatureLevel();
        out << "MaxSupportedFeatureLevel : ";
        switch (level)
        {
        case D3D_FEATURE_LEVEL_12_1: out << "D3D_FEATURE_LEVEL_12_1"; break;
        case D3D_FEATURE_LEVEL_12_0: out << "D3D_FEATURE_LEVEL_12_0"; break;
        case D3D_FEATURE_LEVEL_11_1: out << "D3D_FEATURE_LEVEL_11_1"; break;
        case D3D_FEATURE_LEVEL_11_0: out << "D3D_FEATURE_LEVEL_11_0"; break;
        case D3D_FEATURE_LEVEL_10_1: out << "D3D_FEATURE_LEVEL_10_1"; break;
        case D3D_FEATURE_LEVEL_10_0: out << "D3D_FEATURE_LEVEL_10_0"; break;
        case D3D_FEATURE_LEVEL_9_3: out << "D3D_FEATURE_LEVEL_9_3"; break;
        case D3D_FEATURE_LEVEL_9_2: out << "D3D_FEATURE_LEVEL_9_2"; break;
        case D3D_FEATURE_LEVEL_9_1: out << "D3D_FEATURE_LEVEL_9_1"; break;
        case D3D_FEATURE_LEVEL_1_0_CORE: out << "D3D_FEATURE_LEVEL_1_0_CORE"; break;
        default: out << "unknown or error";
        }
        out << std::endl;

        UINT multisample_level = CheckMsaa4xQuality();
        out << "Msaa4x NumQualityLevels : " << multisample_level << std::endl;
    }

    void DeviceDx12::PrintAdapterInfo(std::wostream& out)
    {
        HRESULT hr = 0;

        ComPtr<IDXGIFactory> factory;
        hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
        if (FAILED(hr))
            return;

        out << L"adapters :" << std::endl;
        for (int i = 0; true; i++)
        {
            ComPtr<IDXGIAdapter> adapter;
            DXGI_ADAPTER_DESC desc;
            hr = factory->EnumAdapters(i, adapter.GetAddressOf());
            if (hr == DXGI_ERROR_NOT_FOUND)
                break;
            if (FAILED(hr))
                return;
            hr = adapter->GetDesc(&desc);
            if (FAILED(hr))
                return;
            out << L"  " << desc.Description << std::endl;
        }
    }

    void DeviceDx12::PrintAdapterOutputInfo(std::wostream& out)
    {
        HRESULT hr = 0;

        ComPtr<IDXGIFactory> factory;
        hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
        if (FAILED(hr))
            return;

        out << L"adapters :" << std::endl;
        for (int i = 0; true; i++)
        {
            ComPtr<IDXGIAdapter> adapter;
            DXGI_ADAPTER_DESC desc;
            hr = factory->EnumAdapters(i, adapter.GetAddressOf());
            if (hr == DXGI_ERROR_NOT_FOUND)
                break;
            if (FAILED(hr))
                return;
            hr = adapter->GetDesc(&desc);
            if (FAILED(hr))
                return;
            out << L"  " << desc.Description << std::endl;
            for (int j = 0; true; j++)
            {
                ComPtr<IDXGIOutput> output;
                DXGI_OUTPUT_DESC desc2;
                hr = adapter->EnumOutputs(j, output.GetAddressOf());
                if (hr == DXGI_ERROR_NOT_FOUND)
                    break;
                if (FAILED(hr))
                    return;
                hr = output->GetDesc(&desc2);
                if (FAILED(hr))
                    return;
                out << L"    " << desc2.DeviceName << std::endl;
            }
        }
    }

    void DeviceDx12::PrintAdapterOutputDisplayInfo(std::wostream& out)
    {
        HRESULT hr = 0;

        ComPtr<IDXGIFactory> factory;
        hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
        if (FAILED(hr))
            return;

        out << L"adapters :" << std::endl;
        for (int i = 0; true; i++)
        {
            ComPtr<IDXGIAdapter> adapter;
            DXGI_ADAPTER_DESC desc;
            hr = factory->EnumAdapters(i, adapter.GetAddressOf());
            if (hr == DXGI_ERROR_NOT_FOUND)
                break;
            if (FAILED(hr))
                return;
            hr = adapter->GetDesc(&desc);
            if (FAILED(hr))
                return;
            out << L"  " << desc.Description << std::endl;
            for (int j = 0; true; j++)
            {
                ComPtr<IDXGIOutput> output;
                DXGI_OUTPUT_DESC desc2;
                hr = adapter->EnumOutputs(j, output.GetAddressOf());
                if (hr == DXGI_ERROR_NOT_FOUND)
                    break;
                if (FAILED(hr))
                    return;
                hr = output->GetDesc(&desc2);
                if (FAILED(hr))
                    return;
                out << L"    " << desc2.DeviceName << std::endl;

                DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
                UINT count = 0;
                hr = output->GetDisplayModeList(format, 0, &count, nullptr);
                if (FAILED(hr))
                    return;
                if (count > 0)
                {
                    std::vector<DXGI_MODE_DESC> descs3(count);
                    hr = output->GetDisplayModeList(format, 0, &count, &descs3[0]);
                    if (FAILED(hr))
                        return;
                    for (auto& desc3 : descs3)
                    {
                        out << L"      " << desc3.Height << L" x " << desc3.Width;
                        out << L" (" << desc3.RefreshRate.Numerator << L"/" << desc3.RefreshRate.Denominator << L")" << std::endl;
                    }
                }
            }
        }
    }

}
