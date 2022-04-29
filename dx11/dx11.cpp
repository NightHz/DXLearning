#include "dx11.h"
#include <d3dcompiler.h>
#include <cmath>

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
        ib_size = 0;
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

    std::shared_ptr<Mesh> Mesh::CreateCubeColor(ID3D11Device5* device)
    {
        std::shared_ptr<Mesh> mesh(new Mesh());

        // set vertex desc
        struct Vertex
        {
            DirectX::XMFLOAT3 pos;
            DirectX::XMFLOAT4 color;
        };
        mesh->vertex_size = sizeof(Vertex);
        mesh->vertex_desc.resize(2);
        mesh->vertex_desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        mesh->vertex_desc[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

        // set vertex data
        std::vector<Vertex> vertices(8);
        // xyz
        vertices[0].pos = DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f);
        vertices[1].pos = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);
        vertices[2].pos = DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f);
        vertices[3].pos = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
        vertices[4].pos = DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f);
        vertices[5].pos = DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f);
        vertices[6].pos = DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f);
        vertices[7].pos = DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f);
        // color
        vertices[0].color = DirectX::XMFLOAT4(0, 0, 0, 1);
        vertices[1].color = DirectX::XMFLOAT4(1, 0, 0, 1);
        vertices[2].color = DirectX::XMFLOAT4(0, 1, 0, 1);
        vertices[3].color = DirectX::XMFLOAT4(1, 1, 0, 1);
        vertices[4].color = DirectX::XMFLOAT4(0, 0, 1, 1);
        vertices[5].color = DirectX::XMFLOAT4(1, 0, 1, 1);
        vertices[6].color = DirectX::XMFLOAT4(0, 1, 1, 1);
        vertices[7].color = DirectX::XMFLOAT4(1, 1, 1, 1);

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


        // set index data
        std::vector<WORD> indexes(36);
        indexes[0] = 0; indexes[1] = 1; indexes[2] = 3;
        indexes[3] = 0; indexes[4] = 3; indexes[5] = 2;
        indexes[6] = 4; indexes[7] = 6; indexes[8] = 7;
        indexes[9] = 4; indexes[10] = 7; indexes[11] = 5;
        indexes[12] = 0; indexes[13] = 4; indexes[14] = 5;
        indexes[15] = 0; indexes[16] = 5; indexes[17] = 1;
        indexes[18] = 2; indexes[19] = 3; indexes[20] = 7;
        indexes[21] = 2; indexes[22] = 7; indexes[23] = 6;
        indexes[24] = 1; indexes[25] = 5; indexes[26] = 7;
        indexes[27] = 1; indexes[28] = 7; indexes[29] = 3;
        indexes[30] = 0; indexes[31] = 2; indexes[32] = 6;
        indexes[33] = 0; indexes[34] = 6; indexes[35] = 4;

        // set index info
        mesh->index_count = static_cast<unsigned int>(indexes.size());
        mesh->ib_size = sizeof(WORD) * mesh->index_count;

        // fill buffer desc
        bd.ByteWidth = mesh->ib_size;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;

        // fill subresource data
        sd.pSysMem = &indexes[0];
        sd.SysMemPitch = 0;
        sd.SysMemSlicePitch = 0;

        // create index buffer
        hr = device->CreateBuffer(&bd, &sd, mesh->ib.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::CreateFromRehenzMesh(ID3D11Device5* device, std::shared_ptr<Rehenz::Mesh> _mesh, bool color_as_normal)
    {
        if (_mesh->GetVertices().empty() || _mesh->GetTriangles().empty())
            return nullptr;

        std::shared_ptr<Mesh> mesh(new Mesh());

        // set vertex desc
        struct Vertex
        {
            DirectX::XMFLOAT3 pos;
            DirectX::XMFLOAT4 normal;
            DirectX::XMFLOAT4 color;
            DirectX::XMFLOAT2 uv;
            DirectX::XMFLOAT2 uv2;
        };
        mesh->vertex_size = sizeof(Vertex);
        mesh->vertex_desc.resize(5);
        mesh->vertex_desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        mesh->vertex_desc[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        mesh->vertex_desc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        mesh->vertex_desc[3] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        mesh->vertex_desc[4] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

        // set vertex data
        std::vector<Vertex> vertices(_mesh->VertexCount());
        std::vector<int> v_repeat(_mesh->VertexCount(), 0);
        auto& _vertices = _mesh->GetVertices();
        for (int i = 0; i < vertices.size(); i++)
        {
            vertices[i].pos = DirectX::XMFLOAT3(_vertices[i].p.x, _vertices[i].p.y, _vertices[i].p.z);
            vertices[i].normal = DirectX::XMFLOAT4(0, 0, 0, 0);
            vertices[i].color = DirectX::XMFLOAT4(1, 1, 1, 1);
            if (color_as_normal)
                vertices[i].normal = DirectX::XMFLOAT4(_vertices[i].c.x, _vertices[i].c.y, _vertices[i].c.z, 0);
            else
                vertices[i].color = DirectX::XMFLOAT4(_vertices[i].c.x, _vertices[i].c.y, _vertices[i].c.z, _vertices[i].c.w);
            vertices[i].uv = DirectX::XMFLOAT2(_vertices[i].uv.x, _vertices[i].uv.y);
            vertices[i].uv2 = DirectX::XMFLOAT2(_vertices[i].uv2.x, _vertices[i].uv2.y);
        }

        // set vertex info
        mesh->vertex_count = static_cast<unsigned int>(vertices.size());
        mesh->vb_size = mesh->vertex_size * mesh->vertex_count;

        // set index data
        std::vector<WORD> indexes(_mesh->TriangleCount() * 3);
        auto& _indexes = _mesh->GetTriangles();
        auto float4_plus_equal = [](DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2) { v1.x += v2.x, v1.y += v2.y, v1.z += v2.z, v1.w += v2.w; };
        auto get_noraml = [](const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& p2, const DirectX::XMFLOAT3& p3)
        {
            DirectX::XMVECTOR v12 = DirectX::XMVectorSet(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z, 0);
            DirectX::XMVECTOR v13 = DirectX::XMVectorSet(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z, 0);
            DirectX::XMVECTOR cross = DirectX::XMVector3Cross(v12, v13);
            DirectX::XMFLOAT4 normal;
            DirectX::XMStoreFloat4(&normal, DirectX::XMVector3Normalize(cross));
            return normal;
        };
        for (size_t i = 0; i < indexes.size(); i += 3)
        {
            indexes[i] = static_cast<WORD>(_indexes[i]);
            indexes[i + 1] = static_cast<WORD>(_indexes[i + 1]);
            indexes[i + 2] = static_cast<WORD>(_indexes[i + 2]);
        }

        // set index info
        mesh->index_count = static_cast<unsigned int>(indexes.size());
        mesh->ib_size = sizeof(WORD) * mesh->index_count;

        if (!color_as_normal)
        {
            // compute normal
            for (size_t i = 0; i < indexes.size(); i += 3)
            {
                DirectX::XMFLOAT4 normal = get_noraml(vertices[indexes[i]].pos, vertices[indexes[i + 1]].pos, vertices[indexes[i + 2]].pos);
                float4_plus_equal(vertices[indexes[i]].normal, normal); v_repeat[indexes[i]]++;
                float4_plus_equal(vertices[indexes[i + 1]].normal, normal); v_repeat[indexes[i + 1]]++;
                float4_plus_equal(vertices[indexes[i + 2]].normal, normal); v_repeat[indexes[i + 2]]++;
            }
            for (size_t i = 0; i < vertices.size(); i++)
            {
                if (v_repeat[i] != 0)
                {
                    vertices[i].normal.x /= v_repeat[i];
                    vertices[i].normal.y /= v_repeat[i];
                    vertices[i].normal.z /= v_repeat[i];
                    vertices[i].normal.w /= v_repeat[i];
                    DirectX::XMStoreFloat4(&vertices[i].normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat4(&vertices[i].normal)));
                }
            }
        }


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


        // fill buffer desc
        bd.ByteWidth = mesh->ib_size;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;

        // fill subresource data
        sd.pSysMem = &indexes[0];
        sd.SysMemPitch = 0;
        sd.SysMemSlicePitch = 0;

        // create index buffer
        hr = device->CreateBuffer(&bd, &sd, mesh->ib.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return mesh;
    }

    Texture::Texture()
    {
    }

    Texture::~Texture()
    {
    }

    std::shared_ptr<Texture> Texture::CreateTexturePlaid(ID3D11Device5* device,
        unsigned int color1, unsigned int color2, unsigned int unit_pixel, unsigned int n)
    {
        unit_pixel = max(unit_pixel, 1);
        n = max(n, 1);
        std::shared_ptr<Texture> tex(new Texture());

        // generate texture
        int width = unit_pixel * 2 * n;
        int width2 = width * width;
        std::vector<unsigned int> plaid(width2);
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < width; y++)
            {
                int i = y * width + x;
                int j = x / unit_pixel + y / unit_pixel;
                plaid[i] = (j % 2 == 0 ? color1 : color2);
            }
        }

        // fill texture2d desc
        D3D11_TEXTURE2D_DESC td;
        td.Width = width;
        td.Height = width;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.SampleDesc.Quality = 0;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        td.CPUAccessFlags = 0;
        td.MiscFlags = 0;

        // fill subresource data
        D3D11_SUBRESOURCE_DATA sd;
        sd.pSysMem = &plaid[0];
        sd.SysMemPitch = width * sizeof(unsigned int);
        sd.SysMemSlicePitch = 0;

        // create texture
        HRESULT hr = device->CreateTexture2D(&td, &sd, tex->tex.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        // create srv
        hr = device->CreateShaderResourceView(tex->tex.Get(), nullptr, tex->srv.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return tex;
    }

    ComPtr<ID3D11Texture2D> Texture::CreateTextureForRTV(ID3D11Device5* device, unsigned int width, unsigned int height)
    {
        ComPtr<ID3D11Texture2D> rtv_buffer;

        // fill texture2d desc
        D3D11_TEXTURE2D_DESC td;
        td.Width = width;
        td.Height = height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        td.SampleDesc.Count = 1;
        td.SampleDesc.Quality = 0;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET;
        td.CPUAccessFlags = 0;
        td.MiscFlags = 0;

        // create rtv buffer
        HRESULT hr = device->CreateTexture2D(&td, nullptr, rtv_buffer.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return rtv_buffer;
    }

    ComPtr<ID3D11Texture2D> Texture::CreateTextureForDSV(ID3D11Device5* device, unsigned int width, unsigned int height)
    {
        ComPtr<ID3D11Texture2D> dsv_buffer;

        // fill texture2d desc
        D3D11_TEXTURE2D_DESC td;
        td.Width = width;
        td.Height = height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        td.SampleDesc.Count = 1;
        td.SampleDesc.Quality = 0;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        td.CPUAccessFlags = 0;
        td.MiscFlags = 0;

        // create dsv buffer
        HRESULT hr = device->CreateTexture2D(&td, nullptr, dsv_buffer.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        return dsv_buffer;
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

    CBuffer::CBuffer()
    {
        cb_struct = nullptr;
        struct_size = 0;
    }

    CBuffer::~CBuffer()
    {
        if (cb_struct)
            std::free(cb_struct);
    }

    bool CBuffer::ApplyToCBuffer(ID3D11DeviceContext4* context)
    {
        D3D11_MAPPED_SUBRESOURCE ms;
        HRESULT hr = context->Map(cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        if (FAILED(hr))
            return false;
        std::memcpy(ms.pData, cb_struct, struct_size);
        context->Unmap(cb.Get(), 0);
        return true;
    }

    std::shared_ptr<CBuffer> CBuffer::CreateCBuffer(ID3D11Device5* device, UINT cb_size)
    {
        std::shared_ptr<CBuffer> cb(new CBuffer());

        // fill buffer desc
        D3D11_BUFFER_DESC bd;
        /*if (cb_size % 16 == 0)
            bd.ByteWidth = cb_size;
        else
            bd.ByteWidth = (cb_size / 16 + 1) * 16; // align to 16 byte*/
        assert(cb_size % 16 == 0);
        bd.ByteWidth = cb_size;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;

        // create constant buffer
        HRESULT hr = device->CreateBuffer(&bd, nullptr, cb->cb.GetAddressOf());
        if (FAILED(hr))
            return nullptr;

        // malloc struct
        cb->struct_size = cb_size;
        cb->cb_struct = std::calloc(1, cb_size);

        return cb;
    }

    Transform::Transform() : pos(0, 0, 0), scale(1, 1, 1)
    {
        roll = pitch = yaw = 0;
    }

    Transform::~Transform()
    {
    }

    void Transform::PosAddOffset(DirectX::XMFLOAT3 offset, float times)
    {
        pos.x += offset.x * times;
        pos.y += offset.y * times;
        pos.z += offset.z * times;
    }

    DirectX::XMMATRIX Transform::GetTransformMatrix()
    {
        DirectX::XMMATRIX mat_scale = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        DirectX::XMMATRIX mat_rotate = DirectX::XMMatrixRotationZ(roll)
            * DirectX::XMMatrixRotationX(pitch) * DirectX::XMMatrixRotationY(yaw);
        //mat_rotate = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        DirectX::XMMATRIX mat_translate = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
        return mat_scale * mat_rotate * mat_translate;
    }

    DirectX::XMMATRIX Transform::GetInverseTransformMatrix()
    {
        DirectX::XMMATRIX mat_scale = DirectX::XMMatrixScaling(1 / scale.x, 1 / scale.y, 1 / scale.z);
        DirectX::XMMATRIX mat_rotate = DirectX::XMMatrixRotationY(-yaw)
            * DirectX::XMMatrixRotationX(-pitch) * DirectX::XMMatrixRotationZ(-roll);
        DirectX::XMMATRIX mat_translate = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z);
        return mat_translate * mat_rotate * mat_scale;
    }

    DirectX::XMFLOAT3 Transform::GetFront()
    {
        DirectX::XMFLOAT3 front(0, 0, 1);
        DirectX::XMVECTOR qua_rotate = DirectX::XMQuaternionRotationRollPitchYaw(roll, yaw, pitch);
        DirectX::XMStoreFloat3(&front, DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&front), qua_rotate));
        return front;
    }

    DirectX::XMFLOAT3 Transform::GetUp()
    {
        DirectX::XMFLOAT3 up(0, 1, 0);
        DirectX::XMVECTOR qua_rotate = DirectX::XMQuaternionRotationRollPitchYaw(roll, yaw, pitch);
        DirectX::XMStoreFloat3(&up, DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&up), qua_rotate));
        return up;
    }

    DirectX::XMFLOAT3 Transform::GetRight()
    {
        DirectX::XMFLOAT3 right(1, 0, 0);
        DirectX::XMVECTOR qua_rotate = DirectX::XMQuaternionRotationRollPitchYaw(roll, yaw, pitch);
        DirectX::XMStoreFloat3(&right, DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&right), qua_rotate));
        return right;
    }

    DirectX::XMFLOAT3 Transform::GetFrontXZ()
    {
        DirectX::XMFLOAT3 front = GetFront();
        float l2 = front.x * front.x + front.z * front.z;
        if (l2 < 0.0001f)
            return DirectX::XMFLOAT3(0, 0, 0);
        else
        {
            float divl = 1 / std::sqrtf(l2);
            return DirectX::XMFLOAT3(front.x * divl, 0, front.z * divl);
        }
    }

    DirectX::XMFLOAT3 Transform::GetRightXZ()
    {
        DirectX::XMFLOAT3 right = GetRight();
        float l2 = right.x * right.x + right.z * right.z;
        if (l2 < 0.0001f)
            return DirectX::XMFLOAT3(0, 0, 0);
        else
        {
            float divl = 1 / std::sqrtf(l2);
            return DirectX::XMFLOAT3(right.x * divl, 0, right.z * divl);
        }
    }

    void Transform::SetScale(float s)
    {
        scale.x = scale.y = scale.z = s;
    }

    Material::Material() : Material(white)
    {
    }

    Material::Material(DirectX::XMFLOAT4 color)
        : ambient(color), diffuse(color), specular(color), emissive(0, 0, 0, 0), power(5)
    {
    }

    Material::~Material()
    {
    }

    void Material::SetToBuffer(VSCBMaterial* vscb_struct)
    {
        vscb_struct->ambient = DirectX::XMVectorSet(ambient.x, ambient.y, ambient.z, ambient.w);
        vscb_struct->diffuse = DirectX::XMVectorSet(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
        vscb_struct->specular = DirectX::XMVectorSet(specular.x, specular.y, specular.z, specular.w);
        vscb_struct->emissive = DirectX::XMVectorSet(emissive.x, emissive.y, emissive.z, emissive.w);
        vscb_struct->power = power;
    }

    DirectX::XMFLOAT4 Material::white(DirectX::XMFLOAT4(1, 1, 1, 1));
    DirectX::XMFLOAT4 Material::black(DirectX::XMFLOAT4(0, 0, 0, 1));
    DirectX::XMFLOAT4 Material::red(DirectX::XMFLOAT4(1, 0, 0, 1));
    DirectX::XMFLOAT4 Material::green(DirectX::XMFLOAT4(0, 1, 0, 1));
    DirectX::XMFLOAT4 Material::blue(DirectX::XMFLOAT4(0, 0, 1, 1));
    DirectX::XMFLOAT4 Material::yellow(DirectX::XMFLOAT4(0.91f, 0.88f, 0.34f, 1));
    DirectX::XMFLOAT4 Material::orange(DirectX::XMFLOAT4(1, 0.5f, 0.14f, 1));

    Object::Object() : bs_blend_factor{ 0,0,0,0 }
    {
        dss_stencil_ref = 0;
    }

    Object::Object(ID3D11Device5* device, std::shared_ptr<Mesh> _mesh,
        std::shared_ptr<VertexShader> _vs, std::shared_ptr<PixelShader> _ps,
        std::shared_ptr<CBuffer> _vscb_transform, std::shared_ptr<CBuffer> _vscb_material)
        : mesh(_mesh), vs(_vs), ps(_ps), vscb_transform(_vscb_transform), vscb_material(_vscb_material)
    {
        UpdateInputLayout(device);
    }

    Object::~Object()
    {
    }

    Object::operator bool()
    {
        return il;
    }

    bool Object::UpdateInputLayout(ID3D11Device5* device)
    {
        if (!mesh)
            return false;

        HRESULT hr = device->CreateInputLayout(&mesh->vertex_desc[0], static_cast<unsigned int>(mesh->vertex_desc.size()),
            vs->blob->GetBufferPointer(), vs->blob->GetBufferSize(), il.GetAddressOf());
        if (FAILED(hr))
            return false;

        return true;
    }

    void Object::Draw(ID3D11DeviceContext4* context)
    {
        // set vb & ib
        context->IASetInputLayout(il.Get());
        if (mesh)
        {
            unsigned int zero = 0;
            context->IASetVertexBuffers(0, 1, mesh->vb.GetAddressOf(), &mesh->vertex_size, &zero);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            if (mesh->ib)
                context->IASetIndexBuffer(mesh->ib.Get(), DXGI_FORMAT_R16_UINT, 0);
        }

        // set shader
        if (vs)
            context->VSSetShader(vs->vs.Get(), nullptr, 0);
        if (ps)
            context->PSSetShader(ps->ps.Get(), nullptr, 0);

        // set transform
        if (vscb_transform)
        {
            VSCBTransform* vscb_struct = static_cast<VSCBTransform*>(vscb_transform->GetPointer());
            vscb_struct->world = DirectX::XMMatrixTranspose(transform.GetTransformMatrix());
            vscb_struct->world_view = vscb_struct->view * vscb_struct->world; // =transpose(transpose(world)*transpose(view))
            vscb_struct->world_view_proj = vscb_struct->view_proj * vscb_struct->world; // =transpose(transpose(world)*transpose(view_proj))
            assert(vscb_transform->ApplyToCBuffer(context) == true);
            context->VSSetConstantBuffers(vscb_struct->slot, 1, vscb_transform->GetBuffer().GetAddressOf());
        }

        // set material
        if (vscb_material)
        {
            VSCBMaterial* vscb_struct = static_cast<VSCBMaterial*>(vscb_material->GetPointer());
            material.SetToBuffer(vscb_struct);
            assert(vscb_material->ApplyToCBuffer(context) == true);
            context->VSSetConstantBuffers(vscb_struct->slot, 1, vscb_material->GetBuffer().GetAddressOf());
        }

        // set texture
        for (int i = 0; i < textures.size(); i++)
            context->PSSetShaderResources(i, 1, textures[i]->srv.GetAddressOf());

        // set state
        if (rs)
            context->RSSetState(rs.Get());
        else
            context->RSSetState(nullptr);
        if (bs)
            context->OMSetBlendState(bs.Get(), bs_blend_factor, 0xffffffff);
        else
            context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
        if (dss)
            context->OMSetDepthStencilState(dss.Get(), dss_stencil_ref);
        else
            context->OMSetDepthStencilState(nullptr, 0);

        // draw
        if (mesh)
        {
            if (mesh->ib)
                context->DrawIndexed(mesh->index_count, 0, 0);
            else
                context->Draw(mesh->vertex_count, 0);
        }
    }

    Projection::Projection()
    {
        fovy = aspect = znear = zfar = 0;
    }

    Projection::~Projection()
    {
    }

    DirectX::XMMATRIX Projection::GetTransformMatrix()
    {
        return DirectX::XMMatrixPerspectiveFovLH(fovy, aspect, znear, zfar);
    }

    Camera::Camera(ID3D11Device5* device, ID3D11Resource* buffer, ID3D11Resource* dsv_buffer,
        std::shared_ptr<CBuffer> _vscb_transform, float width, float height)
        : vscb_transform(_vscb_transform)
    {
        HRESULT hr = device->CreateRenderTargetView(buffer, nullptr, rtv.GetAddressOf());
        if (FAILED(hr))
            rtv = nullptr;

        hr = device->CreateDepthStencilView(dsv_buffer, nullptr, dsv.GetAddressOf());
        if (FAILED(hr))
            dsv = nullptr;

        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        transform.pos.z = -5;
        projection.fovy = DirectX::XM_PIDIV2;
        projection.aspect = width / height;
        projection.znear = 1;
        projection.zfar = 500;
    }

    Camera::Camera(ID3D11Device5* device, IDXGISwapChain* sc, ID3D11Resource* dsv_buffer,
        std::shared_ptr<CBuffer> _vscb_transform, float width, float height)
        : vscb_transform(_vscb_transform)
    {
        ComPtr<ID3D11Texture2D> sc_buffer;
        HRESULT hr = sc->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sc_buffer.GetAddressOf()));
        if (FAILED(hr))
            rtv = nullptr;
        else
        {
            hr = device->CreateRenderTargetView(sc_buffer.Get(), nullptr, rtv.GetAddressOf());
            if (FAILED(hr))
                rtv = nullptr;
        }

        hr = device->CreateDepthStencilView(dsv_buffer, nullptr, dsv.GetAddressOf());
        if (FAILED(hr))
            dsv = nullptr;

        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        transform.pos.z = -5;
        projection.fovy = DirectX::XM_PIDIV2;
        projection.aspect = width / height;
        projection.znear = 1;
        projection.zfar = 500;
    }

    Camera::~Camera()
    {
    }

    Camera::operator bool()
    {
        return rtv && dsv;
    }

    void Camera::Clear(ID3D11DeviceContext4* context, const float rgba[4])
    {
        context->ClearRenderTargetView(rtv.Get(), rgba);
        context->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    }

    void Camera::Clear(ID3D11DeviceContext4* context, float r, float g, float b, float a)
    {
        float rgba[4] = { r,g,b,a };
        Clear(context, rgba);
    }

    void Camera::ClearDepth(ID3D11DeviceContext4* context)
    {
        context->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH, 1, 0);
    }

    void Camera::SetToContext(ID3D11DeviceContext4* context, DirectX::XMMATRIX global_transform)
    {
        // set transform
        if (vscb_transform)
        {
            VSCBTransform* vscb_struct = static_cast<VSCBTransform*>(vscb_transform->GetPointer());
            vscb_struct->view = DirectX::XMMatrixTranspose(global_transform * transform.GetInverseTransformMatrix());
            vscb_struct->proj = DirectX::XMMatrixTranspose(projection.GetTransformMatrix());
            vscb_struct->view_proj = vscb_struct->proj * vscb_struct->view; // =transpose(transpose(view)*transpose(proj))
            vscb_struct->world_view_proj = vscb_struct->view_proj * vscb_struct->world; // =transpose(transpose(world)*transpose(view_proj))
            assert(vscb_transform->ApplyToCBuffer(context) == true);
            context->VSSetConstantBuffers(vscb_struct->slot, 1, vscb_transform->GetBuffer().GetAddressOf());
        }
        context->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());
        context->RSSetViewports(1, &vp);
    }

}
