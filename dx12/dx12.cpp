#include "dx12.h"
#include <wincodec.h>

namespace Dx12
{
    bool UtilDx12::CreateDefaultBuffer(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list,
        const void* data, UINT64 size, ComPtr<ID3D12Resource2>& buffer, ComPtr<ID3D12Resource2>& uploader)
    {
        HRESULT hr = S_OK;

        // create buffer
        D3D12_RESOURCE_DESC rc_desc;
        D3D12_HEAP_PROPERTIES heap_prop;
        rc_desc = GetBufferRcDesc(size);
        heap_prop = GetHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
        hr = device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(buffer.GetAddressOf()));
        if (FAILED(hr))
            return false;

        // create upload
        heap_prop = GetHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        hr = device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(uploader.GetAddressOf()));
        if (FAILED(hr))
            return false;

        // map data to upload
        void* data2;
        hr = uploader->Map(0, nullptr, &data2);
        if (FAILED(hr))
            return false;
        std::memcpy(data2, data, size);
        uploader->Unmap(0, nullptr);

        // copy data
        auto rc_barr = UtilDx12::GetTransitionStruct(buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        cmd_list->ResourceBarrier(1, &rc_barr);
        cmd_list->CopyBufferRegion(buffer.Get(), 0, uploader.Get(), 0, size);
        rc_barr = UtilDx12::GetTransitionStruct(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        cmd_list->ResourceBarrier(1, &rc_barr);

        return true;
    }

    ComPtr<ID3DBlob> UtilDx12::LoadShaderFile(const std::wstring& filename)
    {
        HRESULT hr = S_OK;

        ComPtr<ID3DBlob> shader_blob;

        // open file
        std::ifstream fin(filename, std::ios::binary);
#ifdef _DEBUG
        if (!fin)
            fin.open(L"../x64/Debug/" + filename, std::ios::binary);
#endif
        if (!fin)
            return nullptr;

        // get size
        fin.seekg(0, std::ios_base::end);
        std::ifstream::pos_type size = (int)fin.tellg();
        fin.seekg(0, std::ios_base::beg);

        // read to blob
        hr = D3DCreateBlob(size, shader_blob.GetAddressOf());
        if (FAILED(hr))
        {
            fin.close();
            return nullptr;
        }
        fin.read(static_cast<char*>(shader_blob->GetBufferPointer()), size);
        fin.close();

        return shader_blob;
    }

    ComPtr<ID3DBlob> UtilDx12::CompileShaderFile(const std::wstring& filename, const std::string& shader_type, const std::vector<std::string>& macro)
    {
        HRESULT hr = S_OK;

        ComPtr<ID3DBlob> shader_blob;

        // get defines
        std::vector<D3D_SHADER_MACRO> defines;
        for (const auto& m : macro)
            defines.push_back(D3D_SHADER_MACRO{ m.c_str(),"1" });
        defines.push_back(D3D_SHADER_MACRO{ nullptr,nullptr });

        // compile shader
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ComPtr<ID3DBlob> error_blob;
        hr = D3DCompileFromFile(filename.c_str(), &defines[0], D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", (shader_type + "_5_1").c_str(),
            flags, 0, shader_blob.GetAddressOf(), error_blob.GetAddressOf());
        if (FAILED(hr))
        {
#ifdef _DEBUG
            if (error_blob)
                OutputDebugString(static_cast<char*>(error_blob->GetBufferPointer()));
#endif
            return nullptr;
        }

        return shader_blob;
    }

    ComPtr<ID3DBlob> UtilDx12::LoadImageFile(const std::wstring& filename, UINT& width, UINT& height, DXGI_FORMAT& format)
    {
        HRESULT hr = S_OK;

        // create wic factory
        ComPtr<IWICImagingFactory2> wic_factory;
        hr = CoInitialize(nullptr);
        if (FAILED(hr))
            return nullptr;
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(wic_factory.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // load image from file
        ComPtr<IWICBitmapDecoder> bitmap_src;
        ComPtr<IWICBitmapFrameDecode> frame;
        hr = wic_factory->CreateDecoderFromFilename(filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, bitmap_src.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        hr = bitmap_src->GetFrame(0, frame.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        // convert file format
        ComPtr<IWICFormatConverter> format_converter;
        WICPixelFormatGUID wic_format_in;
        WICPixelFormatGUID wic_format_out = GUID_WICPixelFormat32bppRGBA;
        BOOL can_convert;
        hr = frame->GetPixelFormat(&wic_format_in);
        if (FAILED(hr))
            return nullptr;
        hr = wic_factory->CreateFormatConverter(format_converter.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        hr = format_converter->CanConvert(wic_format_in, wic_format_out, &can_convert);
        if (FAILED(hr))
            return nullptr;
        if (!can_convert)
            return nullptr;
        hr = format_converter->Initialize(frame.Get(), wic_format_out, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);

        // load info
        hr = format_converter->GetSize(&width, &height);
        if (FAILED(hr))
            return nullptr;
        format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // create blob
        UINT size = width * height * sizeof(UINT);
        ComPtr<ID3DBlob> blob;
        hr = D3DCreateBlob(size, blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        // load pixels
        BYTE* data = static_cast<BYTE*>(blob->GetBufferPointer());
        hr = format_converter->CopyPixels(nullptr, width * sizeof(UINT), size, data);
        if (FAILED(hr))
            return nullptr;

        CoUninitialize();

        return blob;
    }

    bool UtilDx12::WaitFenceValue(ID3D12Fence1* fence, UINT64 value)
    {
        UINT64 completed_value = fence->GetCompletedValue();
        if (completed_value < value)
        {
            //OutputDebugString((std::string() + "CPU wait " + std::to_string(value) + " and GPU is " + std::to_string(completed_value) + ".\n").c_str());
            HANDLE event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
            if (!event)
                return false;
            HRESULT hr = fence->SetEventOnCompletion(value, event);
            if (FAILED(hr))
                return false;
            WaitForSingleObject(event, INFINITE);
            CloseHandle(event);
        }
        //else OutputDebugString((std::string() + "CPU get " + std::to_string(value) + " and GPU is " + std::to_string(completed_value) + ".\n").c_str());
        return true;
    }

    bool DeviceDx12::FlushCmdQueue()
    {
        HRESULT hr = S_OK;

        current_fence_v++;
        hr = cmd_queue->Signal(fence.Get(), current_fence_v);
        if (FAILED(hr))
            return false;
        //OutputDebugString((std::string() + "Flush command and CPU mark " + std::to_string(current_fence_v) + ".\n").c_str());
        if (!UtilDx12::WaitFenceValue(fence.Get(), current_fence_v))
            return false;

        return true;
    }

    DeviceDx12::DeviceDx12()
    {
        current_fence_v = 0;
        sc_format = DXGI_FORMAT_UNKNOWN;
        sc_buffer_count = 0;
        rtv_size = 0;
        dsv_size = 0;
        cbv_size = 0;
        sampler_size = 0;
        cbv_heap_i = 0;
        sampler_heap_i = 0;
        ZeroMemory(&vp, sizeof(vp));
        ZeroMemory(&sr, sizeof(sr));
        current_frame_i = 0;
    }

    DeviceDx12::~DeviceDx12()
    {
    }

    bool DeviceDx12::CheckCurrentCmdState()
    {
        return GetCurrentFrameResource().fence_v <= fence->GetCompletedValue();
    }

    bool DeviceDx12::ResetCmd()
    {
        HRESULT hr = S_OK;

        auto& frc = GetCurrentFrameResource();

        // wait gpu if current frame resource is being used
        if (!FlushCurrentCmdQueue())
            return false;

        // reset command queue
        hr = frc.cmd_alloc->Reset();
        if (FAILED(hr))
            return false;
        hr = cmd_list->Reset(frc.cmd_alloc.Get(), nullptr);
        if (FAILED(hr))
            return false;

        return true;
    }

    bool DeviceDx12::FinishCmd()
    {
        HRESULT hr = S_OK;

        // finish command list
        hr = cmd_list->Close();
        if (FAILED(hr))
            return false;

        // execute commands
        ID3D12CommandList* lists[]{ cmd_list.Get() };
        cmd_queue->ExecuteCommandLists(1, lists);

        // mark in fence
        current_fence_v++;
        auto& frc = GetCurrentFrameResource();
        frc.fence_v = current_fence_v;
        hr = cmd_queue->Signal(fence.Get(), frc.fence_v);
        if (FAILED(hr))
            return false;
        //OutputDebugString((std::string() + "CPU mark " + std::to_string(frc.fence_v) + ".\n").c_str());

        return true;
    }

    bool DeviceDx12::FlushCurrentCmdQueue()
    {
        if (!UtilDx12::WaitFenceValue(fence.Get(), GetCurrentFrameResource().fence_v))
            return false;
        return true;
    }

    void DeviceDx12::SetRootParameter0(D3D12_GPU_VIRTUAL_ADDRESS gpu_loc)
    {
        cmd_list->SetGraphicsRootConstantBufferView(0, gpu_loc);
    }

    void DeviceDx12::SetRootParameter1(UINT dh_slot)
    {
        cmd_list->SetGraphicsRootDescriptorTable(1, GetCbvGpu(dh_slot));
    }

    void DeviceDx12::SetRootParameter2(UINT dh_slot)
    {
        cmd_list->SetGraphicsRootDescriptorTable(2, GetCbvGpu(dh_slot));
    }

    void DeviceDx12::SetRootParameter3(UINT dh_slot)
    {
        cmd_list->SetGraphicsRootDescriptorTable(3, GetSamplerGpu(dh_slot));
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
        p->current_fence_v = p->fence->GetCompletedValue();

        // create frame resource
        p->frame_rcs = std::make_unique<FrameResourceDx12[]>(p->frame_rc_count);
        for (int i = 0; i < p->frame_rc_count; i++)
        {
            auto& frame_rc = p->frame_rcs[i];
            if (!frame_rc.Create(p->device.Get()))
                return nullptr;
            frame_rc.fence_v = p->current_fence_v;
        }
        p->current_frame_i = 0;

        // create command queue and list
        D3D12_COMMAND_QUEUE_DESC cmd_queue_desc;
        cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cmd_queue_desc.Priority = 0;
        cmd_queue_desc.NodeMask = 0;
        hr = p->device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(p->cmd_queue.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        hr = p->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p->GetCurrentFrameResource().cmd_alloc.Get(), nullptr, IID_PPV_ARGS(p->cmd_list.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // create swap chain
        p->sc_format = DXGI_FORMAT_R8G8B8A8_UNORM;
        p->sc_buffer_count = 2;
        DXGI_SWAP_CHAIN_DESC sc_desc;
        sc_desc.BufferDesc.Width = window->GetWidth();
        sc_desc.BufferDesc.Height = window->GetHeight();
        sc_desc.BufferDesc.RefreshRate.Numerator = 60;
        sc_desc.BufferDesc.RefreshRate.Denominator = 1;
        sc_desc.BufferDesc.Format = p->sc_format;
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
        p->sampler_size = p->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
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
        dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        dh_desc.NumDescriptors = cbv_heap_size;
        dh_desc.NodeMask = 0;
        hr = p->device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(p->cbv_heap.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        p->cbv_heap_i = 0;
        dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        dh_desc.NumDescriptors = sampler_heap_size;
        dh_desc.NodeMask = 0;
        hr = p->device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(p->sampler_heap.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // get render target buffer and create render target view
        p->rtv_buffers.resize(p->sc_buffer_count);
        for (UINT i = 0; i < p->sc_buffer_count; i++)
        {
            hr = p->sc->GetBuffer(i, IID_PPV_ARGS(p->rtv_buffers[i].GetAddressOf()));
            if (FAILED(hr))
                return nullptr;
            p->device->CreateRenderTargetView(p->rtv_buffers[i].Get(), nullptr, p->GetRtv(i));
        }

        // create depth stencil buffer
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
        rc_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        D3D12_HEAP_PROPERTIES heap_prop;
        heap_prop = UtilDx12::GetHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_CLEAR_VALUE clear_val;
        clear_val.Format = rc_desc.Format;
        clear_val.DepthStencil.Depth = 1.0f;
        clear_val.DepthStencil.Stencil = 0;
        hr = p->device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_val, IID_PPV_ARGS(p->dsv_buffer.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // create depth stencil view
        p->device->CreateDepthStencilView(p->dsv_buffer.Get(), nullptr, p->GetDsv(0));

        // create cbv for frame resource
        for (int i = 0; i < p->frame_rc_count; i++)
            p->frame_rcs[i].CreateCbv(p.get());

        // set viewport
        p->vp.TopLeftX = 0;
        p->vp.TopLeftY = 0;
        p->vp.Width = static_cast<float>(window->GetWidth());
        p->vp.Height = static_cast<float>(window->GetHeight());
        p->vp.MinDepth = 0;
        p->vp.MaxDepth = 1;

        // set scissor rect
        p->sr.left = 0;
        p->sr.right = window->GetWidth();
        p->sr.top = 0;
        p->sr.bottom = window->GetHeight();

        // create root sig
        // 0 CBV : b0
        // 1 DT  : (b1 b2)
        // 2 DT  : (t0 t1)
        // 3 DT  : (s0 s1 s2 s3 s4 s5)
        D3D12_DESCRIPTOR_RANGE dr1[1];
        dr1[0] = UtilDx12::GetDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1); // b1 b2
        D3D12_DESCRIPTOR_RANGE dr2[1];
        dr2[0] = UtilDx12::GetDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0); // t0 t1
        D3D12_DESCRIPTOR_RANGE dr3[1];
        dr3[0] = UtilDx12::GetDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 6, 0); // s0 s1 s2 s3 s4 s5
        D3D12_ROOT_PARAMETER rp[4];
        rp[0] = UtilDx12::GetRootParameterCBV(0); // b0
        rp[1] = UtilDx12::GetRootParameterDT(_countof(dr1), dr1);
        rp[2] = UtilDx12::GetRootParameterDT(_countof(dr2), dr2);
        rp[3] = UtilDx12::GetRootParameterDT(_countof(dr3), dr3);
        D3D12_ROOT_SIGNATURE_DESC rs_desc;
        rs_desc.NumParameters = _countof(rp);
        rs_desc.pParameters = rp;
        rs_desc.NumStaticSamplers = 0;
        rs_desc.pStaticSamplers = nullptr;
        rs_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        ComPtr<ID3DBlob> root_sig_blob;
        hr = D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1, root_sig_blob.GetAddressOf(), nullptr);
        if (FAILED(hr))
            return nullptr;
        hr = p->device->CreateRootSignature(0, root_sig_blob->GetBufferPointer(), root_sig_blob->GetBufferSize(), IID_PPV_ARGS(p->root_sig.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;

        // finish command list
        if (!p->FinishCmd())
            return nullptr;

        return p;
    }

    bool DeviceDx12::ReadyPresent()
    {
        // reset command
        if (!ResetCmd())
            return false;

        // change rtv -> render target
        auto rc_barr = UtilDx12::GetTransitionStruct(GetCurrentRtvBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmd_list->ResourceBarrier(1, &rc_barr);

        // set viewport & scissor rect
        cmd_list->RSSetViewports(1, &vp);
        cmd_list->RSSetScissorRects(1, &sr);

        // clear
        float bg_color[4]{ 0.7804f,0.8627f,0.4078f,0 };
        cmd_list->ClearRenderTargetView(GetCurrentRtv(), bg_color, 0, nullptr);
        cmd_list->ClearDepthStencilView(GetDsv(0), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        // set render target
        auto rtv = GetCurrentRtv();
        auto dsv = GetDsv(0);
        cmd_list->OMSetRenderTargets(1, &rtv, true, &dsv);

        // set heaps
        ID3D12DescriptorHeap* dhs[]{ cbv_heap.Get(), sampler_heap.Get() };
        cmd_list->SetDescriptorHeaps(_countof(dhs), dhs);

        // map cbuffer
        auto& frc = GetCurrentFrameResource();
        if (!frc.cb_obj->Map())
            return false;
        if (!frc.cb_frame->Map())
            return false;
        if (!frc.cb_light->Map())
            return false;

        // set root sig
        cmd_list->SetGraphicsRootSignature(root_sig.Get());

        return true;
    }

    bool DeviceDx12::Present()
    {
        HRESULT hr = S_OK;

        // unmap cbuffer
        auto& frc = GetCurrentFrameResource();
        frc.cb_obj->Unmap();
        frc.cb_frame->Unmap();
        frc.cb_light->Unmap();

        // change rtv -> present
        auto rc_barr = UtilDx12::GetTransitionStruct(GetCurrentRtvBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        cmd_list->ResourceBarrier(1, &rc_barr);

        // finish command
        if (!FinishCmd())
            return false;

        // swap
        hr = sc->Present(0, 0);
        if (FAILED(hr))
            return false;

        // next frame
        NextFrame();

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
        levels.Format = sc_format;
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

    FrameResourceDx12::FrameResourceDx12()
    {
        fence_v = 0;
        cb_frame_dh_slot = 0;
    }

    FrameResourceDx12::~FrameResourceDx12()
    {
    }

    bool FrameResourceDx12::Create(ID3D12Device8* device)
    {
        HRESULT hr = S_OK;

        // create command allocator
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmd_alloc.GetAddressOf()));
        if (FAILED(hr))
            return false;

        // create cbuffer
        cb_obj = std::make_shared<CBufferDx12<CBObj>>(obj_max_count);
        if (!cb_obj->Create(device))
            return false;
        cb_frame = std::make_shared<CBufferDx12<CBFrame>>(1);
        if (!cb_frame->Create(device))
            return false;
        cb_light = std::make_shared<CBufferDx12<CBLight>>(1);
        if (!cb_light->Create(device))
            return false;

        return true;
    }

    void FrameResourceDx12::CreateCbv(DeviceDx12* device)
    {
        cb_frame_dh_slot = device->GetCbvSlot(2);
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = cb_frame->GetCbvDesc(0);
        device->device->CreateConstantBufferView(&cbv_desc, device->GetCbv(cb_frame_dh_slot));
        cbv_desc = cb_light->GetCbvDesc(0);
        device->device->CreateConstantBufferView(&cbv_desc, device->GetCbv(cb_frame_dh_slot + 1));
    }

    CBufferBaseDx12::CBufferBaseDx12(UINT _count, UINT _struct_size)
    {
        count = _count;
        struct_size = _struct_size;
        cbuffer_size = UtilDx12::AlignCBuffer(struct_size);
        data = nullptr;
    }

    CBufferBaseDx12::~CBufferBaseDx12()
    {
    }

    bool CBufferBaseDx12::Create(ID3D12Device8* device)
    {
        HRESULT hr = S_OK;

        // create cbuffer
        auto rc_desc = UtilDx12::GetBufferRcDesc(count * static_cast<UINT64>(cbuffer_size));
        auto heap_prop = UtilDx12::GetHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        hr = device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(cbuffer.GetAddressOf()));
        if (FAILED(hr))
            return false;

        return true;
    }

    bool CBufferBaseDx12::Map()
    {
        HRESULT hr = cbuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
        if (FAILED(hr))
            return false;
        return true;
    }

    void CBufferBaseDx12::Unmap()
    {
        cbuffer->Unmap(0, nullptr);
        data = nullptr;
    }

    bool CBufferBaseDx12::CopyData(UINT slot, const void* cb_struct)
    {
        if (data == nullptr || slot >= count)
            return false;
        auto data2 = data + slot * static_cast<UINT64>(cbuffer_size);
        std::memcpy(data2, cb_struct, struct_size);
        return true;
    }

    PipelineStateCreatorDx12::PipelineStateCreatorDx12()
    {
        ::memset(&pso_desc, 0, sizeof(pso_desc));
        pso_desc.pRootSignature = nullptr;

        pso_desc.VS.pShaderBytecode = nullptr;
        pso_desc.PS.pShaderBytecode = nullptr;
        pso_desc.DS.pShaderBytecode = nullptr;
        pso_desc.HS.pShaderBytecode = nullptr;
        pso_desc.GS.pShaderBytecode = nullptr;
        pso_desc.VS.BytecodeLength = 0;
        pso_desc.PS.BytecodeLength = 0;
        pso_desc.DS.BytecodeLength = 0;
        pso_desc.HS.BytecodeLength = 0;
        pso_desc.GS.BytecodeLength = 0;

        pso_desc.StreamOutput.pSODeclaration = nullptr;
        pso_desc.StreamOutput.NumEntries = 0;
        pso_desc.StreamOutput.pBufferStrides = nullptr;
        pso_desc.StreamOutput.NumStrides = 0;
        pso_desc.StreamOutput.RasterizedStream = 0;

        pso_desc.BlendState.AlphaToCoverageEnable = false;
        pso_desc.BlendState.IndependentBlendEnable = false;
        pso_desc.BlendState.RenderTarget[0].BlendEnable = false;
        pso_desc.BlendState.RenderTarget[0].LogicOpEnable = false;
        pso_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        pso_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        pso_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        pso_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        pso_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        pso_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        pso_desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        pso_desc.SampleMask = UINT_MAX;

        pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        pso_desc.RasterizerState.FrontCounterClockwise = false;
        pso_desc.RasterizerState.DepthBias = 0;
        pso_desc.RasterizerState.DepthBiasClamp = 0;
        pso_desc.RasterizerState.SlopeScaledDepthBias = 0;
        pso_desc.RasterizerState.DepthClipEnable = true;
        pso_desc.RasterizerState.MultisampleEnable = false;
        pso_desc.RasterizerState.AntialiasedLineEnable = false;
        pso_desc.RasterizerState.ForcedSampleCount = 0;
        pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        pso_desc.DepthStencilState.DepthEnable = true;
        pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        pso_desc.DepthStencilState.StencilEnable = false;
        pso_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        pso_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        pso_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        pso_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        pso_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        pso_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        pso_desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        pso_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        pso_desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        pso_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        pso_desc.InputLayout.pInputElementDescs = nullptr;
        pso_desc.InputLayout.NumElements = 0;

        pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.SampleDesc.Quality = 0;

        pso_desc.NodeMask = 0;
        pso_desc.CachedPSO.pCachedBlob = nullptr;
        pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    }

    PipelineStateCreatorDx12::~PipelineStateCreatorDx12()
    {
    }

    ComPtr<ID3D12PipelineState> PipelineStateCreatorDx12::CreatePSO(ID3D12Device8* device)
    {
        ComPtr<ID3D12PipelineState> pso;
        HRESULT hr = device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(pso.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        return pso;
    }

    MeshDx12::MeshDx12()
    {
        topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        v_size = 0;
        v_count = i_count = 0;
        std::memset(&vbv, 0, sizeof(vbv));
        std::memset(&ibv, 0, sizeof(ibv));
        start_pos = 0;
        vertex_offset = 0;
    }

    MeshDx12::~MeshDx12()
    {
    }

    bool MeshDx12::UploadToGpu(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list)
    {
        // set vertex offset and start pos
        start_pos = 0;
        vertex_offset = 0;

        // create vb & vbv
        if (!UtilDx12::CreateDefaultBuffer(device, cmd_list, vb_blob->GetBufferPointer(), vb_blob->GetBufferSize(), vb, vb_uploader))
            return false;
        vbv.BufferLocation = vb->GetGPUVirtualAddress();
        vbv.SizeInBytes = static_cast<UINT>(vb_blob->GetBufferSize());
        vbv.StrideInBytes = v_size;

        // create ib & ibv
        if (ib_blob)
        {
            if (!UtilDx12::CreateDefaultBuffer(device, cmd_list, ib_blob->GetBufferPointer(), ib_blob->GetBufferSize(), ib, ib_uploader))
                return false;
            ibv.BufferLocation = ib->GetGPUVirtualAddress();
            ibv.SizeInBytes = static_cast<UINT>(ib_blob->GetBufferSize());
            ibv.Format = DXGI_FORMAT_R16_UINT;
        }

        return true;
    }

    bool MeshDx12::MergeUploadToGpu(const std::vector<MeshDx12*> meshes, ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list)
    {
        if (meshes.size() == 0)
            return true;

        // check vertex size
        UINT v_size = meshes[0]->v_size;
        for (auto mesh : meshes)
        {
            if (mesh->v_size != v_size)
                return false;
        }

        // set vertex offset and start pos & compute sum count
        UINT v_count = 0;
        UINT i_count = 0;
        for (auto mesh : meshes)
        {
            mesh->vertex_offset = v_count;
            mesh->start_pos = (mesh->ib_blob ? i_count : v_count);
            v_count += mesh->v_count;
            i_count += (mesh->ib_blob ? mesh->i_count : 0);
        }

        // copy data
        UINT vs_size = v_size * v_count;
        UINT is_size = 2 * i_count;
        auto v_data = std::make_unique<BYTE[]>(vs_size);
        auto i_data = std::make_unique<BYTE[]>(is_size);
        BYTE* data = v_data.get();
        for (auto mesh : meshes)
        {
            ::memcpy(data, mesh->vb_blob->GetBufferPointer(), mesh->vb_blob->GetBufferSize());
            data += mesh->vb_blob->GetBufferSize();
        }
        data = i_data.get();
        for (auto mesh : meshes)
        {
            if (mesh->ib_blob)
            {
                ::memcpy(data, mesh->ib_blob->GetBufferPointer(), mesh->ib_blob->GetBufferSize());
                data += mesh->ib_blob->GetBufferSize();
            }
        }

        // create vb & vbv
        if (!UtilDx12::CreateDefaultBuffer(device, cmd_list, v_data.get(), vs_size, meshes[0]->vb, meshes[0]->vb_uploader))
            return false;
        meshes[0]->vbv.BufferLocation = meshes[0]->vb->GetGPUVirtualAddress();
        meshes[0]->vbv.SizeInBytes = vs_size;
        meshes[0]->vbv.StrideInBytes = v_size;

        // create ib & ibv
        if (is_size > 0)
        {
            if (!UtilDx12::CreateDefaultBuffer(device, cmd_list, i_data.get(), is_size, meshes[0]->ib, meshes[0]->ib_uploader))
                return false;
            meshes[0]->ibv.BufferLocation = meshes[0]->ib->GetGPUVirtualAddress();
            meshes[0]->ibv.SizeInBytes = is_size;
            meshes[0]->ibv.Format = DXGI_FORMAT_R16_UINT;
        }

        // copy to other mesh
        for (auto mesh : meshes)
        {
            mesh->vb = meshes[0]->vb;
            mesh->vbv = meshes[0]->vbv;
            if (mesh->ib_blob)
            {
                mesh->ib = meshes[0]->ib;
                mesh->ibv = meshes[0]->ibv;
            }
        }
        if (!meshes[0]->ib_blob)
            meshes[0]->ib = nullptr;

        return true;
    }

    void MeshDx12::FreeUploader()
    {
        vb_uploader = nullptr;
        ib_uploader = nullptr;
    }

    std::shared_ptr<MeshDx12> MeshDx12::CreateCube()
    {
        HRESULT hr = S_OK;

        std::shared_ptr<MeshDx12> p(new MeshDx12);

        // set data
        struct Vertex
        {
            XMFLOAT3 pos;
            XMFLOAT4 color;
        };
        std::vector<Vertex> vertices(8);
        vertices[0] = { XMFLOAT3(-1,-1,-1), XMFLOAT4(1,1,1,1) };
        vertices[1] = { XMFLOAT3(-1,+1,-1), XMFLOAT4(0,0,0,1) };
        vertices[2] = { XMFLOAT3(+1,+1,-1), XMFLOAT4(1,0,0,1) };
        vertices[3] = { XMFLOAT3(+1,-1,-1), XMFLOAT4(0,1,0,1) };
        vertices[4] = { XMFLOAT3(-1,-1,+1), XMFLOAT4(0,0,1,1) };
        vertices[5] = { XMFLOAT3(-1,+1,+1), XMFLOAT4(1,1,0,1) };
        vertices[6] = { XMFLOAT3(+1,+1,+1), XMFLOAT4(1,0,1,1) };
        vertices[7] = { XMFLOAT3(+1,-1,+1), XMFLOAT4(0,1,1,1) };
        int v_size = sizeof(Vertex);
        int v_count = static_cast<int>(vertices.size());
        int vs_size = v_size * v_count;
        std::vector<UINT16> indices
        {
            0,1,2,  0,2,3,
            4,6,5,  4,7,6,
            4,5,1,  4,1,0,
            3,2,6,  3,6,7,
            1,5,6,  1,6,2,
            4,0,3,  4,3,7
        };
        int i_count = static_cast<int>(indices.size());
        int is_size = 2 * i_count;

        // create vb blob & ib blob
        hr = D3DCreateBlob(vs_size, p->vb_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->vb_blob->GetBufferPointer(), &vertices[0], vs_size);
        hr = D3DCreateBlob(is_size, p->ib_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->ib_blob->GetBufferPointer(), &indices[0], is_size);

        // set size & count
        p->v_size = v_size;
        p->v_count = v_count;
        p->i_count = i_count;

        return p;
    }

    std::shared_ptr<MeshDx12> MeshDx12::CreateFromRehenzMesh(std::shared_ptr<Rehenz::Mesh> mesh)
    {
        HRESULT hr = S_OK;

        std::shared_ptr<MeshDx12> p(new MeshDx12);

        // set data
        struct Vertex
        {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT4 color;
            XMFLOAT2 uv;
            XMFLOAT2 uv2;
        };
        std::vector<Vertex> vertices;
        vertices.reserve(mesh->VertexCount());
        for (auto& v : mesh->GetVertices())
        {
            Vertex vv;
            vv.pos = XMFLOAT3(v.p.x, v.p.y, v.p.z);
            vv.normal = XMFLOAT3(v.n.x, v.n.y, v.n.z);
            vv.color = XMFLOAT4(v.c.x, v.c.y, v.c.z, v.c.w);
            vv.uv = XMFLOAT2(v.uv.x, v.uv.y);
            vv.uv2 = XMFLOAT2(v.uv2.x, v.uv2.y);
            vertices.push_back(vv);
        }
        int v_size = sizeof(Vertex);
        int v_count = static_cast<int>(vertices.size());
        int vs_size = v_size * v_count;
        std::vector<UINT16> indices;
        indices.reserve(mesh->IndexCount());
        for (auto i : mesh->GetTriangles())
            indices.push_back(static_cast<UINT16>(i));
        int i_count = static_cast<int>(indices.size());
        int is_size = 2 * i_count;

        // create vb blob & ib blob
        hr = D3DCreateBlob(vs_size, p->vb_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->vb_blob->GetBufferPointer(), &vertices[0], vs_size);
        hr = D3DCreateBlob(is_size, p->ib_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->ib_blob->GetBufferPointer(), &indices[0], is_size);

        // set size & count
        p->v_size = v_size;
        p->v_count = v_count;
        p->i_count = i_count;

        return p;
    }

    std::shared_ptr<MeshDx12> MeshDx12::CreateGrid(int xn, int yn)
    {
        HRESULT hr = S_OK;

        std::shared_ptr<MeshDx12> p(new MeshDx12);

        xn = Rehenz::Clamp(xn, 1, 240);
        yn = Rehenz::Clamp(yn, 1, 240);

        // set data
        struct Vertex
        {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT4 color;
            XMFLOAT2 uv;
            XMFLOAT2 uv2;
        };
        std::vector<Vertex> vertices;
        std::vector<UINT16> indices;
        for (int y = 0; y <= yn; y++)
        {
            for (int x = 0; x <= xn; x++)
            {
                float xf = x / static_cast<float>(xn), yf = y / static_cast<float>(yn);
                Vertex v;
                v.pos = XMFLOAT3(xf * 2 - 1, 0, yf * 2 - 1);
                v.normal = XMFLOAT3(0, 1, 0);
                v.color = XMFLOAT4(1, 1, 1, 1);
                v.uv = XMFLOAT2(xf, yf);
                v.uv2 = XMFLOAT2(0, 0);
                vertices.push_back(v);
            }
        }
        for (int y = 0; y < yn; y++)
        {
            for (int x = 0; x < xn; x++)
            {
                UINT16 v1 = static_cast<UINT16>(y * (xn + 1) + x);
                UINT16 v2 = v1 + 1;
                UINT16 v3 = static_cast<UINT16>(v1 + xn + 1);
                UINT16 v4 = v3 + 1;
                indices.push_back(v1); indices.push_back(v4); indices.push_back(v2);
                indices.push_back(v1); indices.push_back(v3); indices.push_back(v4);
            }
        }
        int v_size = sizeof(Vertex);
        int v_count = static_cast<int>(vertices.size());
        int vs_size = v_size * v_count;
        int i_count = static_cast<int>(indices.size());
        int is_size = 2 * i_count;

        // create vb blob & ib blob
        hr = D3DCreateBlob(vs_size, p->vb_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->vb_blob->GetBufferPointer(), &vertices[0], vs_size);
        hr = D3DCreateBlob(is_size, p->ib_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->ib_blob->GetBufferPointer(), &indices[0], is_size);

        // set size & count
        p->v_size = v_size;
        p->v_count = v_count;
        p->i_count = i_count;

        return p;
    }

    std::shared_ptr<MeshDx12> MeshDx12::CreatePoint()
    {
        HRESULT hr = S_OK;

        std::shared_ptr<MeshDx12> p(new MeshDx12);

        // set data
        p->topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        struct Vertex
        {
            XMFLOAT3 pos;
            XMFLOAT2 size;
        };
        Vertex vertex;
        vertex.pos = XMFLOAT3(0, 0, 0);
        vertex.size = XMFLOAT2(2, 2);
        int v_size = sizeof(Vertex);
        int v_count = 1;
        int vs_size = v_size * v_count;

        // create vb blob
        hr = D3DCreateBlob(vs_size, p->vb_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->vb_blob->GetBufferPointer(), &vertex, vs_size);

        // set size & count
        p->v_size = v_size;
        p->v_count = v_count;
        p->i_count = 0;

        return p;
    }

    std::shared_ptr<MeshDx12> MeshDx12::CreatePatchGrid(int xn, int yn)
    {
        HRESULT hr = S_OK;

        std::shared_ptr<MeshDx12> p(new MeshDx12);

        xn = Rehenz::Clamp(xn, 1, 240);
        yn = Rehenz::Clamp(yn, 1, 240);

        // set data
        p->topology = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
        struct Vertex
        {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT4 color;
            XMFLOAT2 uv;
            XMFLOAT2 uv2;
        };
        std::vector<Vertex> vertices;
        std::vector<UINT16> indices;
        for (int y = 0; y <= yn; y++)
        {
            for (int x = 0; x <= xn; x++)
            {
                float xf = x / static_cast<float>(xn), yf = y / static_cast<float>(yn);
                Vertex v;
                v.pos = XMFLOAT3(xf * 2 - 1, 0, yf * 2 - 1);
                v.normal = XMFLOAT3(0, 1, 0);
                v.color = XMFLOAT4(1, 1, 1, 1);
                v.uv = XMFLOAT2(xf, yf);
                v.uv2 = XMFLOAT2(0, 0);
                vertices.push_back(v);
            }
        }
        for (int y = 0; y < yn; y++)
        {
            for (int x = 0; x < xn; x++)
            {
                UINT16 v1 = static_cast<UINT16>(y * (xn + 1) + x);
                UINT16 v2 = v1 + 1;
                UINT16 v3 = static_cast<UINT16>(v1 + xn + 1);
                UINT16 v4 = v3 + 1;
                indices.push_back(v2); indices.push_back(v1);
                indices.push_back(v4); indices.push_back(v3);
            }
        }
        indices.clear();
        indices.push_back(xn);
        indices.push_back(0);
        indices.push_back(yn* (xn + 1) + xn);
        indices.push_back(yn* (xn + 1));
        int v_size = sizeof(Vertex);
        int v_count = static_cast<int>(vertices.size());
        int vs_size = v_size * v_count;
        int i_count = static_cast<int>(indices.size());
        int is_size = 2 * i_count;

        // create vb blob & ib blob
        hr = D3DCreateBlob(vs_size, p->vb_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->vb_blob->GetBufferPointer(), &vertices[0], vs_size);
        hr = D3DCreateBlob(is_size, p->ib_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->ib_blob->GetBufferPointer(), &indices[0], is_size);

        // set size & count
        p->v_size = v_size;
        p->v_count = v_count;
        p->i_count = i_count;

        return p;
    }

    MaterialDx12::MaterialDx12() : MaterialDx12(white)
    {
    }

    MaterialDx12::MaterialDx12(DirectX::XMFLOAT4 color)
    {
        tex_dh_slot = -1;
        sampler_dh_slot = -1;
        diffuse_albedo = XMFLOAT3(color.x, color.y, color.z);
        alpha = color.w;
        fresnel_r0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
        roughness = 0.5f;
        emissive = XMFLOAT3(0, 0, 0);
    }

    MaterialDx12::~MaterialDx12()
    {
    }

    XMFLOAT4 MaterialDx12::white(XMFLOAT4(1, 1, 1, 1));
    XMFLOAT4 MaterialDx12::black(XMFLOAT4(0, 0, 0, 1));
    XMFLOAT4 MaterialDx12::red(XMFLOAT4(1, 0, 0, 1));
    XMFLOAT4 MaterialDx12::green(XMFLOAT4(0, 1, 0, 1));
    XMFLOAT4 MaterialDx12::blue(XMFLOAT4(0, 0, 1, 1));
    XMFLOAT4 MaterialDx12::yellow(XMFLOAT4(0.91f, 0.88f, 0.34f, 1));
    XMFLOAT4 MaterialDx12::orange(XMFLOAT4(1, 0.5f, 0.14f, 1));

    TextureDx12::TextureDx12()
    {
        dh_slot = -1;
        format = DXGI_FORMAT_UNKNOWN;
        pixel_size = 0;
        width = 0;
        height = 0;
    }

    TextureDx12::~TextureDx12()
    {
    }

    bool TextureDx12::UploadToGpu(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list)
    {
        HRESULT hr = S_OK;

        // compute row pitch
        UINT blob_pitch = width * pixel_size;
        UINT uploader_pitch = UtilDx12::Align(blob_pitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

        // create uploader
        D3D12_RESOURCE_DESC rc_desc;
        D3D12_HEAP_PROPERTIES heap_prop;
        rc_desc = UtilDx12::GetBufferRcDesc(static_cast<UINT64>(uploader_pitch) * height);
        heap_prop = UtilDx12::GetHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        hr = device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(rc_uploader.GetAddressOf()));
        if (FAILED(hr))
            return false;

        // map data to uploader
        void* data;
        hr = rc_uploader->Map(0, nullptr, &data);
        if (FAILED(hr))
            return false;
        for (UINT y = 0; y < height; y++)
        {
            void* dest = static_cast<BYTE*>(data) + y * static_cast<UINT64>(uploader_pitch);
            void* src = static_cast<BYTE*>(rc_blob->GetBufferPointer()) + y * static_cast<UINT64>(blob_pitch);
            memcpy(dest, src, blob_pitch);
        }
        rc_uploader->Unmap(0, nullptr);

        // create resource
        rc_desc = UtilDx12::GetTexture2DRcDesc(width, height, 1, format);
        heap_prop = UtilDx12::GetHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
        hr = device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(rc.GetAddressOf()));
        if (FAILED(hr))
            return false;

        // copy data
        D3D12_TEXTURE_COPY_LOCATION tex_copy_dest, tex_copy_src;
        tex_copy_dest.pResource = rc.Get();
        tex_copy_dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        tex_copy_dest.SubresourceIndex = 0;
        tex_copy_src.pResource = rc_uploader.Get();
        tex_copy_src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        tex_copy_src.PlacedFootprint.Offset = 0;
        tex_copy_src.PlacedFootprint.Footprint.Format = format;
        tex_copy_src.PlacedFootprint.Footprint.Width = width;
        tex_copy_src.PlacedFootprint.Footprint.Height = height;
        tex_copy_src.PlacedFootprint.Footprint.Depth = 1;
        tex_copy_src.PlacedFootprint.Footprint.RowPitch = uploader_pitch;
        cmd_list->CopyTextureRegion(&tex_copy_dest, 0, 0, 0, &tex_copy_src, nullptr);
        auto rc_barr = UtilDx12::GetTransitionStruct(rc.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        cmd_list->ResourceBarrier(1, &rc_barr);

        return true;
    }

    void TextureDx12::CreateSrv(UINT _dh_slot, DeviceDx12* device)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
        srv_desc.Format = format;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Texture2D.MostDetailedMip = 0;
        srv_desc.Texture2D.MipLevels = rc->GetDesc().MipLevels;
        srv_desc.Texture2D.PlaneSlice = 0;
        srv_desc.Texture2D.ResourceMinLODClamp = 0;
        dh_slot = _dh_slot;
        device->device->CreateShaderResourceView(rc.Get(), &srv_desc, device->GetCbv(dh_slot));
    }

    void TextureDx12::FreeUploader()
    {
        rc_uploader = nullptr;
    }

    std::shared_ptr<TextureDx12> TextureDx12::CreateTexturePlaid(UINT color1, UINT color2, UINT unit_pixel, UINT n)
    {
        HRESULT hr = S_OK;

        std::shared_ptr<TextureDx12> p(new TextureDx12());

        // generate texture
        unit_pixel = max(unit_pixel, 1);
        n = max(n, 1);
        int width = unit_pixel * 2 * n;
        int size = width * width;
        std::vector<UINT> plaid(size);
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < width; y++)
            {
                int i = y * width + x;
                int j = x / unit_pixel + y / unit_pixel;
                plaid[i] = (j % 2 == 0 ? color1 : color2);
            }
        }

        // create blob
        UINT blob_size = size * sizeof(UINT);
        hr = D3DCreateBlob(blob_size, p->rc_blob.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        ::memcpy(p->rc_blob->GetBufferPointer(), &plaid[0], blob_size);

        // set parameters
        p->format = DXGI_FORMAT_R8G8B8A8_UNORM;
        p->pixel_size = sizeof(UINT);
        p->width = width;
        p->height = width;

        return p;
    }

    std::shared_ptr<TextureDx12> TextureDx12::CreateTextureFromFile(const std::wstring& filename)
    {
        std::shared_ptr<TextureDx12> p(new TextureDx12());

        p->rc_blob = UtilDx12::LoadImageFile(filename, p->width, p->height, p->format);
        if (!p->rc_blob)
            return nullptr;
        p->pixel_size = sizeof(UINT);

        return p;
    }

    ObjectDx12::ObjectDx12(UINT _cb_slot, std::shared_ptr<MeshDx12> _mesh, std::shared_ptr<MaterialDx12> _mat)
        : cb_slot(_cb_slot), mesh(_mesh), material(_mat)
    {
    }

    ObjectDx12::~ObjectDx12()
    {
    }

    bool ObjectDx12::Draw(DeviceDx12* device)
    {
        auto cmd_list = device->cmd_list.Get();
        auto frc = &device->GetCurrentFrameResource();

        // IA
        cmd_list->IASetPrimitiveTopology(mesh->topology);
        cmd_list->IASetVertexBuffers(0, 1, &mesh->vbv);
        if (mesh->ib)
            cmd_list->IASetIndexBuffer(&mesh->ibv);

        // copy data
        CBObj cb_struct;
        XMMATRIX world = ToXmMatrix(transform.GetTransformMatrix());
        dxm::XMStoreFloat4x4(&cb_struct.world, dxm::XMMatrixTranspose(world));
        XMMATRIX inv_world = ToXmMatrix(transform.GetInverseTransformMatrix());
        dxm::XMStoreFloat4x4(&cb_struct.inv_world, dxm::XMMatrixTranspose(inv_world));
        XMMATRIX uv_tf = ToXmMatrix(uv_transform.GetTransformMatrix());
        dxm::XMStoreFloat4x4(&cb_struct.uv_tf, dxm::XMMatrixTranspose(uv_tf));
        cb_struct.mat.diffuse_albedo = material->diffuse_albedo;
        cb_struct.mat.alpha = material->alpha;
        cb_struct.mat.fresnel_r0 = material->fresnel_r0;
        cb_struct.mat.roughness = material->roughness;
        cb_struct.mat.emissive = material->emissive;
        XMMATRIX tex_tf = ToXmMatrix(material->tex_transform.GetTransformMatrix());
        dxm::XMStoreFloat4x4(&cb_struct.mat.tex_tf, dxm::XMMatrixTranspose(tex_tf));
        if (!frc->cb_obj->CopyData(cb_slot, cb_struct))
            return false;

        // set cbuffer
        device->SetRootParameter0(frc->GetCBObjGpuLocation(cb_slot));

        // set texture
        if (material->tex_dh_slot >= 0)
            device->SetRootParameter2(material->tex_dh_slot);

        // set sampler
        if (material->sampler_dh_slot >= 0)
            device->SetRootParameter3(material->sampler_dh_slot);

        // draw
        if (!mesh->ib)
            cmd_list->DrawInstanced(mesh->v_count, 1, mesh->start_pos, 0);
        else
            cmd_list->DrawIndexedInstanced(mesh->i_count, 1, mesh->start_pos, mesh->vertex_offset, 0);

        return true;
    }

}
