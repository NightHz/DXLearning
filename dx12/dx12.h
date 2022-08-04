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
    struct MaterialForCB
    {
        XMFLOAT3 diffuse_albedo;
        float alpha;
        XMFLOAT3 fresnel_r0; // reflect percent when theta = 0
        float roughness;
        XMFLOAT3 emissive;
        float _pad1;
        XMFLOAT4X4 tex_tf;
    };
    struct CBObj // b0
    {
        XMFLOAT4X4 world;
        XMFLOAT4X4 inv_world;
        XMFLOAT4X4 uv_tf;
        MaterialForCB mat;
    };
    struct CBFrame // b1
    {
        XMFLOAT4X4 view;
        XMFLOAT4X4 inv_view;
        XMFLOAT4X4 proj;
        XMFLOAT4X4 view_proj;
        XMFLOAT3 eye_pos;
        float time;
        XMFLOAT3 eye_at;
        float deltatime;
        XMFLOAT2 screen_size;
        float fog_start;
        float fog_end;
        XMFLOAT3 fog_color;
        float _pad1;
    };
    const float light_type_directional = 1;
    const float light_type_point = 2;
    const float light_type_spot = 3;
    struct LightForCB
    {
        float type; // 0: disable  1: directional light  2: point light  3: spot light
        XMFLOAT3 intensity;
        XMFLOAT3 direction; // for directional light and spot light
        float falloff_begin; // for point light and spot light
        float falloff_end; // for point light and spot light
        XMFLOAT3 position; // for point light and spot light
        float spot_divergence; // for spot light
        XMFLOAT3 _pad1;
    };
    struct CBLight // b2
    {
        float light_enable; // 0: disable  1: enable
        XMFLOAT3 light_ambient;
        LightForCB lights[16];
    };



    class UtilDx12;
    class DeviceDx12;
    class DescriptorHeapsDx12;
    class FrameResourceDx12;
    class CBufferBaseDx12;
    template<typename T> class CBufferDx12;
    class PipelineStateCreatorDx12;
    class MeshDx12;
    class MaterialDx12;
    class TextureDx12;
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

        inline static D3D12_RESOURCE_DESC GetTexture2DRcDesc(UINT width, UINT height, UINT16 mip_levels, DXGI_FORMAT format)
        {
            D3D12_RESOURCE_DESC rc_desc;
            rc_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            rc_desc.Alignment = 0;
            rc_desc.Width = width;
            rc_desc.Height = height;
            rc_desc.DepthOrArraySize = 1;
            rc_desc.MipLevels = mip_levels;
            rc_desc.Format = format;
            rc_desc.SampleDesc.Count = 1;
            rc_desc.SampleDesc.Quality = 0;
            rc_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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

        inline static D3D12_DESCRIPTOR_RANGE GetDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT count, UINT register_i)
        {
            D3D12_DESCRIPTOR_RANGE dr;
            dr.RangeType = type;
            dr.NumDescriptors = count;
            dr.BaseShaderRegister = register_i;
            dr.RegisterSpace = 0;
            dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            return dr;
        }

        inline static D3D12_ROOT_PARAMETER GetRootParameterDT(UINT count, D3D12_DESCRIPTOR_RANGE* pdr)
        {
            D3D12_ROOT_PARAMETER rp;
            rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rp.DescriptorTable.NumDescriptorRanges = count;
            rp.DescriptorTable.pDescriptorRanges = pdr;
            rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            return rp;
        }

        inline static D3D12_ROOT_PARAMETER GetRootParameterCBV(UINT register_i)
        {
            D3D12_ROOT_PARAMETER rp;
            rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rp.Descriptor.ShaderRegister = register_i;
            rp.Descriptor.RegisterSpace = 0;
            rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            return rp;
        }

        inline static D3D12_SAMPLER_DESC GetSamplerDesc(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE address,
            UINT anisotropy = 1, const std::vector<float> border_color = { 0,0,0,1 })
        {
            D3D12_SAMPLER_DESC samp_desc;
            samp_desc.Filter = filter;
            samp_desc.AddressU = samp_desc.AddressV = samp_desc.AddressW = address;
            samp_desc.MipLODBias = 0;
            samp_desc.MaxAnisotropy = anisotropy;
            samp_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            samp_desc.BorderColor[0] = border_color[0];
            samp_desc.BorderColor[1] = border_color[1];
            samp_desc.BorderColor[2] = border_color[2];
            samp_desc.BorderColor[3] = border_color[3];
            samp_desc.MinLOD = 0;
            samp_desc.MaxLOD = D3D12_FLOAT32_MAX;
            return samp_desc;
        }

        static bool CreateDefaultBuffer(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list, const void* data, UINT64 size,
            ComPtr<ID3D12Resource2>& buffer, ComPtr<ID3D12Resource2>& uploader);

        inline static UINT Align(UINT size, UINT aligned)
        {
            return (size + aligned - 1) / aligned * aligned;
        }

        inline static UINT AlignCBuffer(UINT size)
        {
            return (size + 255) & ~255; // (size+255)/256*256
        }

        static ComPtr<ID3DBlob> LoadShaderFile(const std::wstring& filename);
        // shader_type : vs hs ds gs ps cs
        static ComPtr<ID3DBlob> CompileShaderFile(const std::wstring& filename, const std::string& shader_type, const std::vector<std::string>& macro = {});

        // output format must be DXGI_FORMAT_R8G8B8A8_UNORM
        static ComPtr<ID3DBlob> LoadImageFile(const std::wstring& filename, UINT& width, UINT& height, DXGI_FORMAT& format);

        // forbid value > current fense value
        static bool WaitFenceValue(ID3D12Fence1* fence, UINT64 value);
    };

    class DeviceDx12
    {
    public:
        // device & fence
        ComPtr<ID3D12Device8> device;
        ComPtr<ID3D12Fence1> fence;
        UINT64 current_fence_v;

        // command queue & list
        ComPtr<ID3D12CommandQueue> cmd_queue;
        ComPtr<ID3D12GraphicsCommandList6> cmd_list;

        // swap chain
        DXGI_FORMAT sc_format;
        UINT sc_buffer_count;
        ComPtr<IDXGISwapChain4> sc;

        // descriptor heap
        std::unique_ptr<DescriptorHeapsDx12> heaps;

        // render target buffer
        std::vector<ComPtr<ID3D12Resource2>> rtv_buffers;
        ComPtr<ID3D12Resource2> dsv_buffer;

        // viewport & scissor rect
        D3D12_VIEWPORT vp;
        D3D12_RECT sr;


        // frame resource
        static const int frame_rc_count = 3;
        std::unique_ptr<FrameResourceDx12[]> frame_rcs;
        int current_frame_i;


        // root sig
        ComPtr<ID3D12RootSignature> root_sig;


    private:
        DeviceDx12();
    public:
        DeviceDx12(const DeviceDx12&) = delete;
        DeviceDx12& operator=(const DeviceDx12&) = delete;
        ~DeviceDx12();

        // check current command list can / not can be reset
        bool CheckCurrentCmdState();
        // reset command list & queue based current frame resource
        bool ResetCmd();
        // execute command
        bool FinishCmd();
        // flush current command queue
        bool FlushCurrentCmdQueue();
        // flush command queue
        bool FlushCmdQueue();

        // get descriptor heap
        inline DescriptorHeapsDx12* GetDescriptorHeap() { return heaps.get(); }

        // get current render target buffer
        inline ID3D12Resource2* GetCurrentRtvBuffer() { return rtv_buffers[sc->GetCurrentBackBufferIndex()].Get(); }
        // get depth stencil buffer
        inline ID3D12Resource2* GetDsvBuffer() { return dsv_buffer.Get(); }

        // switch to next frame
        inline void NextFrame() { current_frame_i = (current_frame_i + 1) % frame_rc_count; }
        // get current frame resource
        inline FrameResourceDx12& GetCurrentFrameResource() { return frame_rcs[current_frame_i]; }

        // set root parameter 0 : CBV : b0
        void SetRootParameter0(D3D12_GPU_VIRTUAL_ADDRESS gpu_loc);
        // set root parameter 1 : DT  : (b1 b2)
        void SetRootParameter1(UINT dh_slot);
        // set root parameter 2 : DT  : (t0 t1)
        void SetRootParameter2(UINT dh_slot);
        // set root parameter 3 : DT  : (s0 s1 s2 s3 s4 s5)
        void SetRootParameter3(UINT dh_slot);


        // create device
        static std::shared_ptr<DeviceDx12> CreateDevice(Rehenz::SimpleWindow* window);

        // ready present with current frame resource
        bool ReadyPresent();
        // present and next frame
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

    // to do : smart allocate slots with resize
    //         support to recycle slot
    class DescriptorHeapsDx12
    {
    private:
        ComPtr<ID3D12Device8> device;

        // descriptor size
        UINT rtv_size, dsv_size, cbv_size, sampler_size;
        // descriptor heap size
        UINT rtv_heap_size, dsv_heap_size, cbv_heap_size, sampler_heap_size;
        static const UINT rtv_heap_default_size = 10, dsv_heap_default_size = 10, cbv_heap_default_size = 200, sampler_heap_default_size = 20;
        // descriptor heap
        ComPtr<ID3D12DescriptorHeap> rtv_heap;
        ComPtr<ID3D12DescriptorHeap> dsv_heap;
        ComPtr<ID3D12DescriptorHeap> cbv_heap; // also srv heap and uav heap
        ComPtr<ID3D12DescriptorHeap> sampler_heap;
        // descriptor allocate counter
        int rtv_heap_i, dsv_heap_i, cbv_heap_i, sampler_heap_i;

        // get cpu descriptor handle
        inline static D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptor(ID3D12DescriptorHeap* heap, UINT descriptor_size, UINT i)
        {
            auto dh = heap->GetCPUDescriptorHandleForHeapStart();
            dh.ptr += descriptor_size * static_cast<SIZE_T>(i);
            return dh;
        }
        // get gpu descriptor handle
        inline static D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptor(ID3D12DescriptorHeap* heap, UINT descriptor_size, UINT i)
        {
            auto dh = heap->GetGPUDescriptorHandleForHeapStart();
            dh.ptr += descriptor_size * static_cast<UINT64>(i);
            return dh;
        }

        // get descriptor heap description struct
        inline static D3D12_DESCRIPTOR_HEAP_DESC GetDescriptorHeapDesc(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag, UINT heap_size)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc{};
            desc.Type = type;
            desc.Flags = flag;
            desc.NumDescriptors = heap_size;
            desc.NodeMask = 0; // for multi-adapter systems
            return desc;
        }

    public:
        DescriptorHeapsDx12();
        DescriptorHeapsDx12(const DescriptorHeapsDx12&) = delete;
        DescriptorHeapsDx12& operator=(const DescriptorHeapsDx12&) = delete;
        ~DescriptorHeapsDx12();

        // initialization, set 0 size to disable heap
        HRESULT Create(ComPtr<ID3D12Device8> _device, UINT _rtv_heap_size = rtv_heap_default_size, UINT _dsv_heap_size = dsv_heap_default_size,
            UINT _cbv_heap_size = cbv_heap_default_size, UINT _sampler_heap_size = sampler_heap_default_size);
        // clean up
        void Free();

        // get cbv / srv / uav heap
        inline ID3D12DescriptorHeap* GetCbvHeap() { return cbv_heap.Get(); }
        // get sampler heap
        inline ID3D12DescriptorHeap* GetSamplerHeap() { return sampler_heap.Get(); }

        // get rtv descriptor
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetRtv(UINT i) { return GetCpuDescriptor(rtv_heap.Get(), rtv_size, i); }
        // get dsv descriptor
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDsv(UINT i) { return GetCpuDescriptor(dsv_heap.Get(), dsv_size, i); }
        // get cbv / srv / uav descriptor
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCbv(UINT i) { return GetCpuDescriptor(cbv_heap.Get(), cbv_size, i); }
        // get cbv / srv / uav gpu descriptor
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGpu(UINT i) { return GetGpuDescriptor(cbv_heap.Get(), cbv_size, i); }
        // get cbv / srv / uav descriptor
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(UINT i) { return GetCbv(i); }
        // get cbv / srv / uav gpu descriptor
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpu(UINT i) { return GetCbvGpu(i); }
        // get cbv / srv / uav descriptor
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetUav(UINT i) { return GetCbv(i); }
        // get cbv / srv / uav gpu descriptor
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetUavGpu(UINT i) { return GetCbvGpu(i); }
        // get sampler descriptor
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetSampler(UINT i) { return GetCpuDescriptor(sampler_heap.Get(), sampler_size, i); }
        // get sampler gpu descriptor
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerGpu(UINT i) { return GetGpuDescriptor(sampler_heap.Get(), sampler_size, i); }

        // allocate empty rtv slots
        inline UINT AllocateRtvSlot() { return rtv_heap_i++; }
        // allocate empty dsv slots
        inline UINT AllocateDsvSlot() { return dsv_heap_i++; }
        // allocate continuous empty cbv / srv / uav slots, return first slot
        inline UINT AllocateCbvSlots(UINT count = 1) { UINT slot = cbv_heap_i; cbv_heap_i += count; return slot; }
        // allocate continuous empty cbv / srv / uav slots, return first slot
        inline UINT AllocateSrvSlots(UINT count = 1) { return AllocateCbvSlots(count); }
        // allocate continuous empty cbv / srv / uav slots, return first slot
        inline UINT AllocateUavSlots(UINT count = 1) { return AllocateCbvSlots(count); }
        // allocate continuous empty sampler slots, return first slot
        inline UINT AllocateSamplerSlots(UINT count = 1) { UINT slot = sampler_heap_i; sampler_heap_i += count; return slot; }
    };

    class FrameResourceDx12
    {
    public:
        UINT64 fence_v;
        ComPtr<ID3D12CommandAllocator> cmd_alloc;

        static const int obj_max_count = 80;
        std::shared_ptr<CBufferDx12<CBObj>> cb_obj;
        UINT cb_frame_dh_slot;
        std::shared_ptr<CBufferDx12<CBFrame>> cb_frame;
        std::shared_ptr<CBufferDx12<CBLight>> cb_light;

    public:
        FrameResourceDx12();
        FrameResourceDx12(const FrameResourceDx12&) = delete;
        FrameResourceDx12& operator=(const FrameResourceDx12&) = delete;
        ~FrameResourceDx12();

        bool Create(ID3D12Device8* device);
        void CreateCbv(ID3D12Device8* device, DescriptorHeapsDx12* heaps);

        inline D3D12_GPU_VIRTUAL_ADDRESS GetCBObjGpuLocation(UINT cb_slot);
    };

    class CBufferBaseDx12
    {
    public:
        UINT count;
        UINT struct_size;
        UINT cbuffer_size;
        ComPtr<ID3D12Resource2> cbuffer;
        BYTE* data;

    public:
        CBufferBaseDx12(UINT _count, UINT _struct_size);
        CBufferBaseDx12(const CBufferBaseDx12&) = delete;
        CBufferBaseDx12& operator=(const CBufferBaseDx12&) = delete;
        ~CBufferBaseDx12();

        bool Create(ID3D12Device8* device);

        inline D3D12_GPU_VIRTUAL_ADDRESS GetCbGpuLocation(UINT cb_slot)
        {
            return cbuffer->GetGPUVirtualAddress() + cb_slot * static_cast<UINT64>(cbuffer_size);
        }
        inline D3D12_CONSTANT_BUFFER_VIEW_DESC GetCbvDesc(UINT cb_slot)
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
            cbv_desc.BufferLocation = GetCbGpuLocation(cb_slot);
            cbv_desc.SizeInBytes = cbuffer_size;
            return cbv_desc;
        }

        bool Map();
        void Unmap();

        bool CopyData(UINT slot, const void* cb_struct);
    };

    template<typename T>
    class CBufferDx12 : public CBufferBaseDx12
    {
    public:
        CBufferDx12(UINT _count) : CBufferBaseDx12(_count, sizeof(T)) {}
        CBufferDx12(const CBufferDx12&) = delete;
        CBufferDx12& operator=(const CBufferDx12&) = delete;
        ~CBufferDx12() {}

        inline bool CopyData(UINT slot, const T& cb_struct)
        {
            return CBufferBaseDx12::CopyData(slot, &cb_struct);
        }
    };

    inline D3D12_GPU_VIRTUAL_ADDRESS FrameResourceDx12::GetCBObjGpuLocation(UINT cb_slot) { return cb_obj->GetCbGpuLocation(cb_slot); }

    class PipelineStateCreatorDx12
    {
    public:
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;

    public:
        PipelineStateCreatorDx12();
        ~PipelineStateCreatorDx12();

        inline void SetRootSignature(ID3D12RootSignature* rs) { pso_desc.pRootSignature = rs; }
        inline void SetVS(ID3DBlob* blob) { pso_desc.VS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.VS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetPS(ID3DBlob* blob) { pso_desc.PS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.PS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetDS(ID3DBlob* blob) { pso_desc.DS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.DS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetHS(ID3DBlob* blob) { pso_desc.HS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.HS.BytecodeLength = blob->GetBufferSize(); }
        inline void SetGS(ID3DBlob* blob) { pso_desc.GS.pShaderBytecode = blob->GetBufferPointer(); pso_desc.GS.BytecodeLength = blob->GetBufferSize(); }
        inline void ResetDS() { pso_desc.DS.pShaderBytecode = nullptr; pso_desc.DS.BytecodeLength = 0; }
        inline void ResetHS() { pso_desc.HS.pShaderBytecode = nullptr; pso_desc.HS.BytecodeLength = 0; }
        inline void ResetGS() { pso_desc.GS.pShaderBytecode = nullptr; pso_desc.GS.BytecodeLength = 0; }
        inline void SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& desc)
        {
            pso_desc.InputLayout.pInputElementDescs = &desc[0]; pso_desc.InputLayout.NumElements = static_cast<UINT>(desc.size());
        }
        inline void SetInputTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
        {
            pso_desc.PrimitiveTopologyType = topology;
        }

        inline void SetRSFillMode(D3D12_FILL_MODE mode = D3D12_FILL_MODE_SOLID) { pso_desc.RasterizerState.FillMode = mode; }
        inline void SetRSCullMode(D3D12_CULL_MODE mode = D3D12_CULL_MODE_BACK) { pso_desc.RasterizerState.CullMode = mode; }
        inline void SetBSAlpha()
        {
            pso_desc.BlendState.RenderTarget[0].BlendEnable = true;
            pso_desc.BlendState.RenderTarget[0].LogicOpEnable = false;
            pso_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            pso_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            pso_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            pso_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            pso_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            pso_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            pso_desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
            pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }
        inline void ResetBS()
        {
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
        }
        inline void SetBSNoWrite()
        {
            pso_desc.BlendState.RenderTarget[0].BlendEnable = false;
            pso_desc.BlendState.RenderTarget[0].LogicOpEnable = false;
            pso_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
            pso_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
            pso_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            pso_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            pso_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            pso_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            pso_desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
            pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
        }

        ComPtr<ID3D12PipelineState> CreatePSO(ID3D12Device8* device);
    };

    class MeshDx12
    {
    public:
        D3D12_PRIMITIVE_TOPOLOGY topology;

        ComPtr<ID3DBlob> vb_blob, ib_blob;
        UINT v_size;
        UINT v_count;
        UINT i_count;

        ComPtr<ID3D12Resource2> vb, ib;
        ComPtr<ID3D12Resource2> vb_uploader, ib_uploader;
        D3D12_VERTEX_BUFFER_VIEW vbv;
        D3D12_INDEX_BUFFER_VIEW ibv;
        UINT start_pos;
        int vertex_offset;

    private:
        MeshDx12();
    public:
        MeshDx12(const MeshDx12&) = delete;
        MeshDx12& operator=(const MeshDx12&) = delete;
        ~MeshDx12();

        bool UploadToGpu(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list);
        static bool MergeUploadToGpu(const std::vector<MeshDx12*> meshes, ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list);

        void FreeUploader();

        static std::shared_ptr<MeshDx12> CreateCube();
        static std::shared_ptr<MeshDx12> CreateFromRehenzMesh(std::shared_ptr<Rehenz::Mesh> mesh);
        static std::shared_ptr<MeshDx12> CreateGrid(int xn, int yn);
        static std::shared_ptr<MeshDx12> CreatePoint();
        static std::shared_ptr<MeshDx12> CreatePatchGrid(int xn, int yn);
        static std::shared_ptr<MeshDx12> CreatePatchBezier();
    };

    class MaterialDx12 : public MaterialForCB
    {
    public:
        int tex_dh_slot;
        int sampler_dh_slot;

        Rehenz::Transform tex_transform;

        MaterialDx12();
        MaterialDx12(DirectX::XMFLOAT4 color);
        ~MaterialDx12();

        static DirectX::XMFLOAT4 white, black, red, green, blue, yellow, orange;
    };

    class TextureDx12
    {
    public:
        int dh_slot;

        ComPtr<ID3DBlob> rc_blob;
        DXGI_FORMAT format;
        UINT pixel_size;
        UINT width;
        UINT height;

        ComPtr<ID3D12Resource2> rc, rc_uploader;

    private:
        TextureDx12();
    public:
        TextureDx12(const TextureDx12&) = delete;
        TextureDx12& operator=(const TextureDx12&) = delete;
        ~TextureDx12();

        bool UploadToGpu(ID3D12Device8* device, ID3D12GraphicsCommandList6* cmd_list);
        void CreateSrv(UINT _dh_slot, ID3D12Device8* device, DescriptorHeapsDx12* heaps);

        void FreeUploader();

        // color1/color2 format : ABGR = (R,G,B,A)(byte)
        static std::shared_ptr<TextureDx12> CreateTexturePlaid(UINT color1 = 0xffffffff, UINT color2 = 0xffe8a200, UINT unit_pixel = 16, UINT n = 4);
        static std::shared_ptr<TextureDx12> CreateTextureFromFile(const std::wstring& filename);
    };

    class ObjectDx12
    {
    public:
    public:
        UINT cb_slot;
        Rehenz::Transform transform;
        Rehenz::Transform uv_transform;

        std::shared_ptr<MeshDx12> mesh;
        std::shared_ptr<MaterialDx12> material;

        ObjectDx12(UINT _cb_slot, std::shared_ptr<MeshDx12> _mesh, std::shared_ptr<MaterialDx12> _mat);
        ObjectDx12(const ObjectDx12&) = delete;
        ObjectDx12& operator=(const ObjectDx12&) = delete;
        ~ObjectDx12();

        bool Draw(DeviceDx12* device);
    };
}
