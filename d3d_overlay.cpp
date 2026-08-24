#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "d3d_overlay.h"

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <algorithm>

#include "optical_state.h"

namespace {
struct ShaderParams {
    float sphere;
    float cylinder;
    float axisRad;
    float distanceCm;
    float deconv;
    float contrast;
    float marginRatio;
    float reserved;
};

ShaderParams g_shaderParams = {};

void SafeRelease(IUnknown* ptr) {
    if (ptr) ptr->Release();
}
}

bool ProbeD3D11Overlay(std::wstring* statusMessage) {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    D3D_FEATURE_LEVEL requested[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL actual = D3D_FEATURE_LEVEL_10_0;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        requested,
        ARRAYSIZE(requested),
        D3D11_SDK_VERSION,
        &device,
        &actual,
        &context
    );

    if (FAILED(hr)) {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            requested,
            ARRAYSIZE(requested),
            D3D11_SDK_VERSION,
            &device,
            &actual,
            &context
        );
    }

    SafeRelease(context);
    SafeRelease(device);

    if (FAILED(hr)) {
        if (statusMessage) *statusMessage = L"D3D11 no inicializo; usando GDI+ fallback.";
        return false;
    }

    if (statusMessage) {
        *statusMessage = (actual >= D3D_FEATURE_LEVEL_11_0)
            ? L"D3D11 disponible para overlay GPU."
            : L"D3D11 disponible con feature level bajo; probar rendimiento.";
    }
    return true;
}

void UpdateD3DOverlayParameters(const OpticalState& state) {
    g_shaderParams.sphere = state.sphere;
    g_shaderParams.cylinder = state.cylinder;
    g_shaderParams.axisRad = state.axis * 3.1415926535f / 180.0f;
    g_shaderParams.distanceCm = (float)std::max(30, state.distance);
    g_shaderParams.deconv = state.deconv;
    g_shaderParams.contrast = state.contrast;
    g_shaderParams.marginRatio = state.marginRatio;
    g_shaderParams.reserved = 0.0f;
}
