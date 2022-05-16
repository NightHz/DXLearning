#include "dx12.h"

namespace Dx12
{
	DeviceDx12::DeviceDx12()
	{
	}

	DeviceDx12::~DeviceDx12()
	{
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

}
