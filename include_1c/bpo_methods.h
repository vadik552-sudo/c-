#pragma once

enum class MethodId : long {
    GetVersion = 0,
    GetDescription,
    GetParameters,
    SetParameter,
    Open,
    Close,
    DeviceTest,
    GetAdditionalActions,
    DoAdditionalAction,
    GetLastError,
    Count
};

enum class ActionId : long {
    TestConnection = 0,
    ShowLastLog,
    Count
};
