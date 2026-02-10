#pragma once

enum class MethodId : long {
    GetVersion = 0,
    GetDescription = 1,
    GetParameters = 2,
    SetParameter = 3,
    Open = 4,
    Close = 5,
    DeviceTest = 6,
    GetAdditionalActions = 7,
    DoAdditionalAction = 8,
    GetLastError = 9,
    Count = 10
};

enum class ParamId : long {
    BaseURL = 0,
    Token = 1,
    PointId = 2,
    TimeoutMs = 3,
    PollIntervalMs = 4,
    VerboseLog = 5
};
