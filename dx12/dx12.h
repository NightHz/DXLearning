#pragma once
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Rehenz/type.h"
#include <ostream>
#include "Rehenz/window_fc.h"

namespace Dx12
{
    // smart pointer for COM
    using Microsoft::WRL::ComPtr;
    // DirectX Math
    namespace dxm
    {
        using namespace DirectX;
        using namespace DirectX::PackedVector;
    }
    // DX Math Vector & Matrix
    using dxm::XMVECTOR;
    using dxm::FXMVECTOR; // 1,2,3
    using dxm::GXMVECTOR; // 4
    using dxm::HXMVECTOR; // 5,6
    using dxm::CXMVECTOR; // other
    using dxm::XMFLOAT4;
    using dxm::XMFLOAT3;
    using dxm::XMFLOAT2;
    using dxm::XMMATRIX;
    using dxm::FXMMATRIX; // 1
    using dxm::CXMMATRIX; // other
    using dxm::XMFLOAT4X4;



    class DeviceDx12;



    class DeviceDx12
    {
    private:
        // device & fence
        ComPtr<ID3D12Device8> device;
        ComPtr<ID3D12Fence1> fence;

        // command queue & list & allocator
        ComPtr<ID3D12CommandQueue> cmd_queue;
        ComPtr<ID3D12CommandAllocator> cmd_alloc;
        ComPtr<ID3D12GraphicsCommandList6> cmd_list;

        // swap chain
        DXGI_FORMAT format;
        UINT sc_buffer_count;
        ComPtr<IDXGISwapChain4> sc;

        // descriptor heap
        UINT rtv_size, dsv_size, cbv_size;
        ComPtr<ID3D12DescriptorHeap> rtv_heap;
        ComPtr<ID3D12DescriptorHeap> dsv_heap;
        //ComPtr<ID3D12DescriptorHeap> cbv_heap;

        // view buffer
        std::vector<ComPtr<ID3D12Resource2>> rtv_buffers;
        ComPtr<ID3D12Resource2> dsv_buffer;

        // viewport & scissor rect
        D3D12_VIEWPORT vp;
        D3D12_RECT sr;

        // get descriptor & view buffer
        D3D12_CPU_DESCRIPTOR_HANDLE GetRtv(UINT i = 0);
        D3D12_CPU_DESCRIPTOR_HANDLE GetDsv(UINT i = 0);
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtv() { return GetRtv(sc->GetCurrentBackBufferIndex()); }
        inline ID3D12Resource2* GetCurrentRtvBuffer() { return rtv_buffers[sc->GetCurrentBackBufferIndex()].Get(); }

        // flush command queue
        bool FlushCmdQueue();

    private:
        DeviceDx12();
    public:
        DeviceDx12(const DeviceDx12&) = delete;
        DeviceDx12& operator=(const DeviceDx12&) = delete;
        ~DeviceDx12();

        // create device
        static std::shared_ptr<DeviceDx12> CreateDevice(Rehenz::SimpleWindow* window);

        // present
        bool Present();

        // check & print feature
        D3D_FEATURE_LEVEL CheckMaxFeatureLevel();
        UINT CheckMsaa4xQuality();
        void PrintSupportInfo(std::ostream& out);

        // print adapter info
        static void PrintAdapterInfo(std::wostream& out);
        static void PrintAdapterOutputInfo(std::wostream& out);
        static void PrintAdapterOutputDisplayInfo(std::wostream& out);
    };
}
