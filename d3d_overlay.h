#pragma once

#include <string>

struct OpticalState;

bool ProbeD3D11Overlay(std::wstring* statusMessage);
void UpdateD3DOverlayParameters(const OpticalState& state);
