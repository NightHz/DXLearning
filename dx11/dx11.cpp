#include "dx11.h"
#include <d3dcompiler.h>

namespace Dx11
{
    std::shared_ptr<Infrastructure> CreateSimpleD3d11Device(Rehenz::SimpleWindow* window)
    {
        HRESULT hr = 0;

        // fill swap chain desc
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = window->GetWidth();
        sd.BufferDesc.Height = window->GetHeight();
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = window->GetHwnd();
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        // set allowed feature levels
        std::vector<D3D_FEATURE_LEVEL> feature_levels{ D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0 };

        // create device and swap chain
        std::shared_ptr<Infrastructure> infra(new Infrastructure);
        ComPtr<ID3D11Device> device0;
        ComPtr<ID3D11DeviceContext> context0;
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            &feature_levels[0], static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &sd,
            infra->sc.GetAddressOf(), device0.GetAddressOf(), &infra->feature_level, context0.GetAddressOf());
        if (FAILED(hr))
            return nullptr;
        hr = device0.As(&infra->device);
        if (FAILED(hr))
            return nullptr;
        hr = context0.As(&infra->context);
        if (FAILED(hr))
            return nullptr;

        // create render target view
        ComPtr<ID3D11Texture2D> sc_buffer;
        hr = infra->sc->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sc_buffer.GetAddressOf()));
        if (FAILED(hr))
            return nullptr;
        hr = infra->device->CreateRenderTargetView(sc_buffer.Get(), nullptr, infra->rtv.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        // set render target view
        infra->context->OMSetRenderTargets(1, infra->rtv.GetAddressOf(), nullptr);

        // set viewport
        D3D11_VIEWPORT vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = static_cast<float>(window->GetWidth());
        vp.Height = static_cast<float>(window->GetHeight());
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        infra->context->RSSetViewports(1, &vp);

        return infra;
    }

    std::vector<std::wstring> GetAdapterDescs()
    {
        HRESULT hr = 0;
        std::vector<std::wstring> descs;

        ComPtr<IDXGIFactory> factory;
        hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
        if (FAILED(hr))
            return descs;
        for (int i = 0; true; i++)
        {
            ComPtr<IDXGIAdapter> adapter;
            DXGI_ADAPTER_DESC desc;
            if (factory->EnumAdapters(i, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND)
                break;
            hr = adapter->GetDesc(&desc);
            if (FAILED(hr))
                return descs;
            descs.push_back(desc.Description);
        }

        return descs;
    }

    Mesh::Mesh()
    {
        vb_size = 0;
        vertex_count = 0;
        vertex_size = 0;
        index_count = 0;
    }

    Mesh::~Mesh()
    {
    }

    std::shared_ptr<Mesh> Mesh::CreateTriangleXYZ(ID3D11Device5* device)
    {
        std::shared_ptr<Mesh> mesh(new Mesh());

        // set vertex desc
        struct Vertex
        {
            DirectX::XMFLOAT3 pos;
        };
        mesh->vertex_size = sizeof(Vertex);
        mesh->vertex_desc.resize(1);
        mesh->vertex_desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };

        // set vertex data
        std::vector<Vertex> vertices(3);
        vertices[0].pos = DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f);
        vertices[1].pos = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);
        vertices[2].pos = DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f);

        // set vertex info
        mesh->vertex_count = static_cast<unsigned int>(vertices.size());
        mesh->vb_size = mesh->vertex_size * mesh->vertex_count;

        // fill buffer desc
        D3D11_BUFFER_DESC bd;
        bd.ByteWidth = mesh->vb_size;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;

        // fill subresource data
        D3D11_SUBRESOURCE_DATA sd;
        sd.pSysMem = &vertices[0];
        sd.SysMemPitch = 0;
        sd.SysMemSlicePitch = 0;

        // create vertex buffer
        HRESULT hr = device->CreateBuffer(&bd, &sd, mesh->vb.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return mesh;
    }

    VertexShader::VertexShader()
    {
    }

    VertexShader::~VertexShader()
    {
    }

    std::shared_ptr<VertexShader> VertexShader::CompileVS(ID3D11Device5* device, const std::wstring filename)
    {
        std::shared_ptr<VertexShader> vs(new VertexShader());

        HRESULT hr = 0;

        // compile shader
        hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", "vs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0, vs->blob.GetAddressOf(), nullptr);
        if (FAILED(hr))
            return nullptr;

        // create vertex shader
        hr = device->CreateVertexShader(vs->blob->GetBufferPointer(), vs->blob->GetBufferSize(),
            nullptr, vs->vs.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return vs;
    }

    PixelShader::PixelShader()
    {
    }

    PixelShader::~PixelShader()
    {
    }

    std::shared_ptr<PixelShader> PixelShader::CompilePS(ID3D11Device5* device, const std::wstring filename)
    {
        std::shared_ptr<PixelShader> ps(new PixelShader());

        HRESULT hr = 0;

        // compile shader
        hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", "ps_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0, ps->blob.GetAddressOf(), nullptr);
        if (FAILED(hr))
            return nullptr;

        // create pixel shader
        hr = device->CreatePixelShader(ps->blob->GetBufferPointer(), ps->blob->GetBufferSize(),
            nullptr, ps->ps.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return ps;
    }

    Object::Object()
    {
    }

    Object::Object(ID3D11Device5* device, std::shared_ptr<Mesh> _mesh,
        std::shared_ptr<VertexShader> _vs, std::shared_ptr<PixelShader> _ps)
        : mesh(_mesh), vs(_vs), ps(_ps)
    {
        UpdateInputLayout(device);
    }

    Object::~Object()
    {
    }

    bool Object::UpdateInputLayout(ID3D11Device5* device)
    {
        HRESULT hr = device->CreateInputLayout(&mesh->vertex_desc[0], static_cast<unsigned int>(mesh->vertex_desc.size()),
            vs->blob->GetBufferPointer(), vs->blob->GetBufferSize(), il.GetAddressOf());
        if (FAILED(hr))
            return false;

        return true;
    }

    bool Object::Draw(ID3D11DeviceContext4* context)
    {
        unsigned int zero = 0;

        // set vb & ib
        if (il)
            context->IASetInputLayout(il.Get());
        if (mesh->vb)
            context->IASetVertexBuffers(0, 1, mesh->vb.GetAddressOf(), &mesh->vertex_size, &zero);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (mesh->ib)
            context->IASetIndexBuffer(mesh->ib.Get(), DXGI_FORMAT_R16_UINT, 0);

        // set shader
        if (vs)
            context->VSSetShader(vs->vs.Get(), nullptr, 0);
        if (ps)
            context->PSSetShader(ps->ps.Get(), nullptr, 0);

        // draw
        if (mesh->ib)
            context->DrawIndexed(mesh->index_count, 0, 0);
        else
            context->Draw(mesh->vertex_count, 0);

        return true;
    }

}
