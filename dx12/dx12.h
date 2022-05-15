#pragma once
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>
#include <dxgi1_6.h>

namespace Dx12
{
    using Microsoft::WRL::ComPtr;
    using namespace DirectX;
    using namespace DirectX::PackedVector;

    ComPtr<ID3D12Device8> CreateSimpleD3d12Device()
    {
        return nullptr;
    }
}
