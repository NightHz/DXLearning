#pragma once
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <ostream>
#include <fstream>
#include <unordered_map>
#include "Rehenz/type.h"
#include "Rehenz/window_fc.h"
#include "Rehenz/render_soft.h"

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
    // Rehenz Math <=====> DX Math
    inline XMVECTOR ToXmVector(Rehenz::Vector v)
    {
        XMFLOAT4 f4(v.x, v.y, v.z, v.w);
        return dxm::XMLoadFloat4(&f4);
    }
    inline XMMATRIX ToXmMatrix(const Rehenz::Matrix& m)
    {
        XMFLOAT4X4 f4x4(
            m(0, 0), m(0, 1), m(0, 2), m(0, 3),
            m(1, 0), m(1, 1), m(1, 2), m(1, 3),
            m(2, 0), m(2, 1), m(2, 2), m(2, 3),
            m(3, 0), m(3, 1), m(3, 2), m(3, 3));
        return dxm::XMLoadFloat4x4(&f4x4);
    }



    // cbuffer struct
    struct CBTransform
    {
        XMFLOAT4X4 world;
        XMFLOAT4X4 view;
        XMFLOAT4X4 proj;
        XMFLOAT4X4 world_view;
        XMFLOAT4X4 view_proj;
        XMFLOAT4X4 world_view_proj;
    };



    class UtilDx12;
    class DeviceDx12;
    template<typename T> class CBufferDx12;
    class PipelineStateDx12;
    class MeshDx12;
    class ObjectDx12;



    class UtilDx12
    {
    public:
        UtilDx12() = delete;
        inline static D3D12_RESOURCE_BARRIER GetTransitionStruct(ID3D12Resource2* rc, D3D12_RESOURCE_STATES start, D3D12_RESOURCE_STATES end)
        {
            D3D12_RESOURCE_BARRIER rc_barr;
            rc_barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            rc_barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            rc_barr.Transition.pResource = rc;
            rc_barr.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            rc_barr.Transition.StateBefore = start;
            rc_barr.Transition.StateAfter = end;
            return rc_barr;
        }

        inline static D3D12_RESOURCE_DESC GetBufferRcDesc(UINT64 buffer_size)
        {
            D3D12_RESOURCE_DESC rc_desc;
            rc_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            rc_desc.Alignment = 0;
            rc_desc.Width = buffer_size;
            rc_desc.Height = 1;
            rc_desc.DepthOrArraySize = 1;
            rc_desc.MipLevels = 1;
            rc_desc.Format = DXGI_FORMAT_UNKNOWN;
            rc_desc.SampleDesc.Count = 1;
            rc_desc.SampleDesc.Quality = 0;
            rc_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            rc_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
            return rc_desc;
        }

        inline static D3D12_HEAP_PROPERTIES GetHeapProperties(D3D12_HEAP_TYPE type)
        {
            D3D12_HEAP_PROPERTIES heap_prop;
            heap_prop.Type = type;
            heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_prop.CreationNodeMask = 1;
            heap_prop.VisibleNodeMask = 1;
            return heap_prop;
        }

        static bool CreateDefaultBuffer(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list, const void* data, UINT64 size,
            ComPtr<ID3D12Resource2>& buffer, ComPtr<ID3D12Resource2>& uploader);

        inline static UINT AlignCBuffer(UINT size)
        {
            return (size + 255) & ~255; // (size+255)/256*256
        }

        static ComPtr<ID3DBlob> LoadShaderFile(const std::wstring& filename);
        // shader_type : vs hs ds gs ps cs
        static ComPtr<ID3DBlob> CompileShaderFile(const std::wstring& filename, const std::string& shader_type);
    };

    class DeviceDx12
    {
    public:
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

        // view buffer
        std::vector<ComPtr<ID3D12Resource2>> rtv_buffers;
        ComPtr<ID3D12Resource2> dsv_buffer;

        // cbv_heap & cbuffer
        std::shared_ptr<CBufferDx12<CBTransform>> cbuffer;

        // root sig
        ComPtr<ID3D12RootSignature> root_sig;

        // viewport & scissor rect
        D3D12_VIEWPORT vp;
        D3D12_RECT sr;

        // camera info
        Rehenz::Transform camera_trans;
        Rehenz::Projection camera_proj;


        // get descriptor & view buffer
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetRtv(UINT i)
        {
            auto dh = rtv_heap->GetCPUDescriptorHandleForHeapStart();
            dh.ptr += rtv_size * static_cast<unsigned long long>(i);
            return dh;
        };
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDsv(UINT i)
        {
            auto dh = dsv_heap->GetCPUDescriptorHandleForHeapStart();
            dh.ptr += dsv_size * static_cast<unsigned long long>(i);
            return dh;
        };
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

        // reset command list & queue
        bool ResetCmd();
        // execute command
        bool FinishCmd();

        // create device
        static std::shared_ptr<DeviceDx12> CreateDevice(Rehenz::SimpleWindow* window);

        // ready present
        bool ReadyPresent();
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

    template<typename T>
    class CBufferDx12
    {
    public:
        UINT cbv_size;
        ComPtr<ID3D12DescriptorHeap> cbv_heap;
        UINT count;
        UINT cbuffer_size;
        ComPtr<ID3D12Resource2> cbuffer;
        BYTE* data;
        UINT current_slot;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCbv(UINT i)
        {
            auto dh = cbv_heap->GetCPUDescriptorHandleForHeapStart();
            dh.ptr += cbv_size * static_cast<unsigned long long>(i);
            return dh;
        };
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGpu(UINT i)
        {
            auto dh = cbv_heap->GetGPUDescriptorHandleForHeapStart();
            dh.ptr += cbv_size * static_cast<UINT64>(i);
            return dh;
        };

    public:
        CBufferDx12(UINT _count = 160)
        {
            cbv_size = 0;
            count = _count;
            cbuffer_size = UtilDx12::AlignCBuffer(sizeof(T));
            data = nullptr;
            current_slot = 0;
        }
        CBufferDx12(const CBufferDx12&) = delete;
        CBufferDx12& operator=(const CBufferDx12&) = delete;
        ~CBufferDx12()
        {
        }

        bool CreateCBuffer(ID3D12Device8* device)
        {
            HRESULT hr = S_OK;

            cbv_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            // create cbv heap
            D3D12_DESCRIPTOR_HEAP_DESC dh_desc;
            dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            dh_desc.NumDescriptors = count;
            dh_desc.NodeMask = 0;
            hr = device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(cbv_heap.GetAddressOf()));
            if (FAILED(hr))
                return false;

            // create cbuffer and cbv
            auto rc_desc = UtilDx12::GetBufferRcDesc(count * static_cast<UINT64>(cbuffer_size));
            auto heap_prop = UtilDx12::GetHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
            hr = device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &rc_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(cbuffer.GetAddressOf()));
            if (FAILED(hr))
                return false;
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
            for (UINT i = 0; i < count; i++)
            {
                cbv_desc.BufferLocation = cbuffer->GetGPUVirtualAddress() + i * static_cast<UINT64>(cbuffer_size);
                cbv_desc.SizeInBytes = cbuffer_size;
                device->CreateConstantBufferView(&cbv_desc, GetCbv(i));
            }

            return true;
        }

        bool Map()
        {
            HRESULT hr = cbuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
            if (FAILED(hr))
                return false;
            current_slot = 0;
            return true;
        }
        void Unmap()
        {
            cbuffer->Unmap(0, nullptr);
            data = nullptr;
            current_slot = 0;
        }

        bool SetCBuffer(DeviceDx12* device, const T& cb_struct)
        {
            if (data == nullptr || current_slot >= count)
                return false;

            // copy data
            auto data2 = data + current_slot * static_cast<UINT64>(cbuffer_size);
            std::memcpy(data2, &cb_struct, sizeof(T));

            // set cbv
            device->cmd_list->SetGraphicsRootDescriptorTable(0, GetCbvGpu(current_slot));

            current_slot++;

            return true;
        }
    };

    class PipelineStateDx12
    {
    public:
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
        ComPtr<ID3D12PipelineState> pso;

    public:
        PipelineStateDx12();
        PipelineStateDx12(const PipelineStateDx12&) = delete;
        PipelineStateDx12& operator=(const PipelineStateDx12&) = delete;
        ~PipelineStateDx12();

        inline void SetRootSignature(ID3D12RootSignature* rs) { pso_desc.pRootSignature = rs; }
        inline void SetVS(ID3DBlob* blob) { pso_desc.VS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.VS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetPS(ID3DBlob* blob) { pso_desc.PS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.PS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetDS(ID3DBlob* blob) { pso_desc.DS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.DS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetHS(ID3DBlob* blob) { pso_desc.HS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.HS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetGS(ID3DBlob* blob) { pso_desc.GS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.GS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& desc)
        { pso_desc.InputLayout.pInputElementDescs = &desc[0]; pso_desc.InputLayout.NumElements = static_cast<UINT>(desc.size()); }

        bool CreatePSO(DeviceDx12* device);
    };

    class MeshDx12
    {
    public:
        ComPtr<ID3D12Resource2> vb, ib;
        ComPtr<ID3D12Resource2> vb_uploader, ib_uploader;
        D3D12_VERTEX_BUFFER_VIEW vbv;
        D3D12_INDEX_BUFFER_VIEW ibv;
        int v_count;
        int i_count;

    private:
        MeshDx12();
    public:
        MeshDx12(const MeshDx12&) = delete;
        MeshDx12& operator=(const MeshDx12&) = delete;
        ~MeshDx12();

        void FreeUploader();

        static std::shared_ptr<MeshDx12> CreateCube(DeviceDx12* device);
        static std::shared_ptr<MeshDx12> CreateFromRehenzMesh(DeviceDx12* device, std::shared_ptr<Rehenz::Mesh> mesh);
    };

    class ObjectDx12
    {
    public:
    public:
        Rehenz::Transform transform;
        std::shared_ptr<MeshDx12> mesh;

        ObjectDx12(std::shared_ptr<MeshDx12> _mesh);
        ObjectDx12(const ObjectDx12&) = delete;
        ObjectDx12& operator=(const ObjectDx12&) = delete;
        ~ObjectDx12();

        bool Draw(DeviceDx12* device);
    };
}
