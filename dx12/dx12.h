#pragma once
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>
#include <ostream>

namespace Dx12
{
    using Microsoft::WRL::ComPtr;
    namespace dxm // DirectX Math
    {
        using namespace DirectX;
        using namespace DirectX::PackedVector;
    }
    using dxm::XMVECTOR;
    using dxm::FXMVECTOR;
    using dxm::GXMVECTOR;
    using dxm::HXMVECTOR;
    using dxm::CXMVECTOR;
    using dxm::XMFLOAT4;
    using dxm::XMFLOAT3;
    using dxm::XMFLOAT2;
    using dxm::XMMATRIX;
    using dxm::FXMMATRIX;
    using dxm::CXMMATRIX;
    using dxm::XMFLOAT4X4;



    class DeviceDx12;



    class DeviceDx12
    {
    private:
        ComPtr<ID3D12Device8> device;
        ComPtr<IDXGISwapChain4> sc;

    public:
        DeviceDx12();
        DeviceDx12(const DeviceDx12&) = delete;
        DeviceDx12& operator=(const DeviceDx12&) = delete;
        ~DeviceDx12();

        static void PrintAdapterInfo(std::wostream& out);
        static void PrintAdapterOutputInfo(std::wostream& out);
        static void PrintAdapterOutputDisplayInfo(std::wostream& out);
    };
}
