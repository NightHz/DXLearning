#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Rehenz/window.h"
#include <memory>
#include <vector>
#include <string>
#include "Rehenz/mesh.h"

namespace Dx11
{
    using Microsoft::WRL::ComPtr;

    struct Infrastructure
    {
        ComPtr<ID3D11Device5> device;
        ComPtr<ID3D11DeviceContext4> context;
        D3D_FEATURE_LEVEL feature_level;
        ComPtr<IDXGISwapChain> sc;
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
        unsigned int ib_size;
        unsigned int index_count;

        Mesh();

    public:
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        ~Mesh();

        static std::shared_ptr<Mesh> CreateTriangleXYZ(ID3D11Device5* device);
        static std::shared_ptr<Mesh> CreateCubeColor(ID3D11Device5* device);
        static std::shared_ptr<Mesh> CreateFromRehenzMesh(ID3D11Device5* device, std::shared_ptr<Rehenz::Mesh> _mesh, bool color_as_normal = false);
    };

    class Texture
    {
        friend class Object;
    private:
        ComPtr<ID3D11Texture2D> tex;
        ComPtr<ID3D11ShaderResourceView> srv;

        Texture();

    public:
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        ~Texture();

        static std::shared_ptr<Texture> CreateTexturePlaid(ID3D11Device5* device,
            unsigned int color1 = 0xffffffff, unsigned int color2 = 0xfff0c154, unsigned int unit_pixel = 16, unsigned int n = 4);
        static ComPtr<ID3D11Texture2D> CreateTextureForRTV(ID3D11Device5* device, unsigned int width, unsigned int height);
        static ComPtr<ID3D11Texture2D> CreateTextureForDSV(ID3D11Device5* device, unsigned int width, unsigned int height);
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

    struct VSCBTransform
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX world_view;
        DirectX::XMMATRIX view_proj;
        DirectX::XMMATRIX world_view_proj;
        static const unsigned int slot = 0;
    };

    struct VSCBMaterial
    {
        DirectX::XMVECTOR ambient;
        DirectX::XMVECTOR diffuse;
        DirectX::XMVECTOR specular;
        DirectX::XMVECTOR emissive;
        float power;
        static const unsigned int slot = 1;
    };

    struct VSCBLight
    {
        alignas(4) bool dl_enable;
        alignas(4) bool dl_specular_enable;
        DirectX::XMVECTOR dl_dir;
        DirectX::XMVECTOR dl_ambient;
        DirectX::XMVECTOR dl_diffuse;
        DirectX::XMVECTOR dl_specular;
        alignas(4) bool pl_enable;
        alignas(4) bool pl_specular_enable;
        float pl_range;
        DirectX::XMVECTOR pl_pos;
        DirectX::XMVECTOR pl_ambient;
        DirectX::XMVECTOR pl_diffuse;
        DirectX::XMVECTOR pl_specular;
        static const unsigned int slot = 2;
    };

    class CBuffer
    {
    private:
        ComPtr<ID3D11Buffer> cb;
        void* cb_struct;
        unsigned int struct_size;

        CBuffer();

    public:
        CBuffer(const CBuffer&) = delete;
        CBuffer& operator=(const CBuffer&) = delete;
        ~CBuffer();

        ComPtr<ID3D11Buffer> GetBuffer() { return cb; }
        // use column-first storage(hlsl) instead of row-first storage(dx)
        void* GetPointer() { return cb_struct; }

        bool ApplyToCBuffer(ID3D11DeviceContext4* context);

        static std::shared_ptr<CBuffer> CreateCBuffer(ID3D11Device5* device, UINT cb_size);
    };

    class Transform
    {
    public:
        DirectX::XMFLOAT3 pos;
        float roll, pitch, yaw;
        DirectX::XMFLOAT3 scale;

        Transform();
        ~Transform();

        void PosAddOffset(DirectX::XMFLOAT3 offset, float times = 1);

        DirectX::XMMATRIX GetTransformMatrix();
        DirectX::XMMATRIX GetInverseTransformMatrix();

        DirectX::XMFLOAT3 GetFront();
        DirectX::XMFLOAT3 GetUp();
        DirectX::XMFLOAT3 GetRight();
        DirectX::XMFLOAT3 GetFrontXZ();
        DirectX::XMFLOAT3 GetRightXZ();

        void SetScale(float s);
    };

    class Material
    {
    public:
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;
        DirectX::XMFLOAT4 emissive;
        float power;

        Material();
        Material(DirectX::XMFLOAT4 color);
        ~Material();

        void SetToBuffer(VSCBMaterial* vscb_struct);
        
        static DirectX::XMFLOAT4 white, black, red, green, blue, yellow, orange;
    };

    class Object
    {
    private:
        ComPtr<ID3D11InputLayout> il;

    public:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<VertexShader> vs;
        std::shared_ptr<PixelShader> ps;
        std::shared_ptr<CBuffer> vscb_transform;
        std::shared_ptr<CBuffer> vscb_material;

        std::vector<std::shared_ptr<Texture>> textures;

        ComPtr<ID3D11RasterizerState> rs;
        ComPtr<ID3D11BlendState> bs;
        float bs_blend_factor[4];
        ComPtr<ID3D11DepthStencilState> dss;
        unsigned int dss_stencil_ref;

        Transform transform;
        Material material;

        Object();
        Object(ID3D11Device5* device, std::shared_ptr<Mesh> _mesh,
            std::shared_ptr<VertexShader> _vs, std::shared_ptr<PixelShader> _ps,
            std::shared_ptr<CBuffer> _vscb_transform, std::shared_ptr<CBuffer> _vscb_material);
        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        ~Object();

        operator bool();

        bool UpdateInputLayout(ID3D11Device5* device);

        void Draw(ID3D11DeviceContext4* context);
    };

    class Projection
    {
    public:
        float fovy, aspect, znear, zfar;

        Projection();
        ~Projection();

        DirectX::XMMATRIX GetTransformMatrix();
    };

    class Camera
    {
    private:
        ComPtr<ID3D11RenderTargetView> rtv;
        ComPtr<ID3D11DepthStencilView> dsv;

    public:
        D3D11_VIEWPORT vp;
        std::shared_ptr<CBuffer> vscb_transform;

        Transform transform;
        Projection projection;

        Camera(ID3D11Device5* device, ID3D11Resource* rtv_buffer, ID3D11Resource* dsv_buffer,
            std::shared_ptr<CBuffer> _vscb_transform, float width, float height);
        Camera(ID3D11Device5* device, IDXGISwapChain* sc, ID3D11Resource* dsv_buffer,
            std::shared_ptr<CBuffer> _vscb_transform, float width, float height);
        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;
        ~Camera();

        operator bool();

        void Clear(ID3D11DeviceContext4* context, const float rgba[4]);
        void Clear(ID3D11DeviceContext4* context, float r, float g, float b, float a);
        void ClearDepth(ID3D11DeviceContext4* context);
        void SetToContext(ID3D11DeviceContext4* context, DirectX::XMMATRIX global_transform = DirectX::XMMatrixIdentity());
    };

}
