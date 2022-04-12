#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Rehenz/window.h"
#include <memory>
#include <vector>
#include <string>

namespace Dx11
{
    using Microsoft::WRL::ComPtr;

    struct Infrastructure
    {
        ComPtr<ID3D11Device5> device;
        ComPtr<ID3D11DeviceContext4> context;
        D3D_FEATURE_LEVEL feature_level;
        ComPtr<IDXGISwapChain> sc;
        ComPtr<ID3D11RenderTargetView> rtv;
        Infrastructure() { feature_level = D3D_FEATURE_LEVEL_1_0_CORE; }
    };

    std::shared_ptr<Infrastructure> CreateSimpleD3d11Device(Rehenz::SimpleWindow* window);
    std::vector<std::wstring> GetAdapterDescs();

    class Mesh
    {
        friend class Object;
    private:
        ComPtr<ID3D11Buffer> vb;
        unsigned int vb_size;
        unsigned int vertex_count;
        unsigned int vertex_size;
        std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_desc;
        ComPtr<ID3D11Buffer> ib;
        unsigned int index_count;

        Mesh();

    public:
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        ~Mesh();

        static std::shared_ptr<Mesh> CreateTriangleXYZ(ID3D11Device5* device);
    };

    class VertexShader
    {
        friend class Object;
    private:
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3D11VertexShader> vs;

        VertexShader();

    public:
        VertexShader(const VertexShader&) = delete;
        VertexShader& operator=(const VertexShader&) = delete;
        ~VertexShader();

        static std::shared_ptr<VertexShader> CompileVS(ID3D11Device5* device, const std::wstring filename);
    };

    class PixelShader
    {
        friend class Object;
    private:
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3D11PixelShader> ps;

        PixelShader();

    public:
        PixelShader(const PixelShader&) = delete;
        PixelShader& operator=(const PixelShader&) = delete;
        ~PixelShader();

        static std::shared_ptr<PixelShader> CompilePS(ID3D11Device5* device, const std::wstring filename);
    };

    class Object
    {
    private:
        ComPtr<ID3D11InputLayout> il;

    public:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<VertexShader> vs;
        std::shared_ptr<PixelShader> ps;

        Object();
        Object(ID3D11Device5* device, std::shared_ptr<Mesh> _mesh,
            std::shared_ptr<VertexShader> _vs, std::shared_ptr<PixelShader> _ps);
        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        ~Object();

        bool UpdateInputLayout(ID3D11Device5* device);

        bool Draw(ID3D11DeviceContext4* context);
    };

    class Camera
    {
    private:
        ComPtr<ID3D11RenderTargetView> rtv;
        ComPtr<ID3D11DepthStencilView> dsv;

    };

}
