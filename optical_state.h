#pragma once

#include <windows.h>

enum AppPhase {
    PHASE_CALIBRATION_WIZARD = 0,
    PHASE_MAGNIFIER_90 = 1,
    PHASE_FULLSCREEN_MULTI = 2
};

enum CalibPhase {
    CALIB_SPHERE = 0,
    CALIB_CYL_POWER,
    CALIB_AXIS_FINE
};

enum MonitorTarget {
    MONITOR_PRIMARY = 0,
    MONITOR_SECONDARY = 1,
    MONITOR_ALL_VIRTUAL = 2
};

struct OpticalState {
    float sphere = -4.50f;
    float cylinder = -1.25f;
    int axis = 95;
    int distance = 60;
    float deconv = 1.2f;
    float contrast = 1.05f;
    float marginRatio = 0.94f;

    AppPhase appPhase = PHASE_CALIBRATION_WIZARD;
    CalibPhase calibPhase = CALIB_SPHERE;
    MonitorTarget monitorTarget = MONITOR_PRIMARY;

    bool isFullScreen = false;
    RECT prevNormalRect = {0, 0, 1300, 800};

    int autoCurrentOption = 1;
    DWORD lastToggleTick = 0;

    float sphereCenter = -4.50f;
    float sphereStep = 1.00f;

    float cylCenter = -1.25f;
    float cylStep = 0.75f;

    float axisCenter = 95.0f;
    float axisStep = 10.0f;

    float opt1Sphere = -4.00f, opt2Sphere = -5.00f;
    float opt1Cyl = -0.75f, opt2Cyl = -1.75f;
    float opt1Axis = 90.0f, opt2Axis = 100.0f;

    int iterationCount = 0;
    int sourceType = 0;
};
