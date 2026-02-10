# KaspiKassaAPIv3 (NativeAPI, 1C 8.3.22 x64)

## Build (Windows x64, MSVC only)
```powershell
mkdir build
cd build
cmake -A x64 ..
cmake --build . --config Release
```

Expected output DLL:
- `build/bin/Release/KaspiKassaAPIv3_x64.dll`

The build also copies DLL into:
- `bpo_driver/KaspiKassaAPIv3_x64.dll`

## Package ZIP for BPO installation
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\pack.ps1 -CheckExports
```

Generated package:
- `KaspiKassaAPIv3_BPO.zip`

ZIP root contains exactly:
- `INFO.XML`
- `MANIFEST.XML`
- `TableParameters.xml`
- `TableActions.xml`
- `KaspiKassaAPIv3_x64.dll`

## AddIn.None diagnostic checklist
1. Verify 1C client is x64 and DLL is x64.
2. Verify exports via `dumpbin /EXPORTS KaspiKassaAPIv3_x64.dll`:
   - `GetClassObject`
   - `DestroyObject`
   - `GetClassNames`
   - `SetGlobalMemoryManager`
   - `SetMemManager`
3. Verify `GetClassNames` signature is exactly:
   - `long __stdcall GetClassNames(WCHAR_T** names, long* count)`
   - returns class `KaspiKassaAPIv3`, `count = 1`.
4. Verify `INFO.XML` and `MANIFEST.XML` are in bundle format with namespace:
   - `http://v8.1c.ru/8.2/addin/bundle`
5. Verify XML declares native component with:
   - `type="native"`, `os="Windows"`, `arch="x86_64"`.
6. Verify DLL file name matches XML exactly:
   - `KaspiKassaAPIv3_x64.dll`
7. Verify DLL is in ZIP root (not inside subfolders).
8. Verify `WCHAR_T=uint16_t`, `VTYPE_LPWSTR=31`, `sizeof(tVariant)=24`.
9. Verify all returned strings are allocated only via IMemoryManager.
