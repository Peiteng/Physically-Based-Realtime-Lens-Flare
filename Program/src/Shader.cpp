#include "../include/Shader.h"
#include <exception>
#include <fstream>
#if _MSC_VER > 1922 && !defined(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#if _MSC_VER > 1922 && !defined(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>
using namespace std::experimental::filesystem;

// Shader Compile
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;

void Shader::load(const std::wstring& fileName, Stage stage,
	const std::wstring& entryPoint,
	const std::vector<std::wstring>& flags,
	const std::vector<DefineMacro>& defines)
{
	path filePath(fileName);
	std::ifstream infile(filePath, std::ios::binary);
	std::vector<char> sourceCode;
	if (!infile)
		throw std::runtime_error("shader not found");
	sourceCode.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
	infile.seekg(0, infile.beg).read(sourceCode.data(), sourceCode.size());

	ComPtr<IDxcLibrary> library;
	ComPtr<IDxcCompiler> compiler;
	ComPtr<IDxcBlobEncoding> source;
	ComPtr<IDxcOperationResult> dxcResult;

	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	library->CreateBlobWithEncodingFromPinned(sourceCode.data(), UINT(sourceCode.size()), CP_UTF8, &source);
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	wstring profile;
	switch (stage)
	{
	default:
	case Shader::Vertex:
		profile = L"vs_6_0";
		break;
	case Shader::Geometry:
		profile = L"gs_6_0";
		break;
	case Shader::Pixel:
		profile = L"ps_6_0";
		break;
	case Shader::Domain:
		profile = L"ds_6_0";
		break;
	case Shader::Hull:
		profile = L"hs_6_0";
		break;
	case Shader::Compute:
		profile = L"cs_6_0";
		break;
	}
	vector<LPCWSTR> compilerFlags;
	for (auto& v : flags)
	{
		compilerFlags.push_back(v.c_str());
	}
#if _DEBUG
	compilerFlags.push_back(L"/Zi");
	compilerFlags.push_back(L"/O0");
#else
	compilerFlags.push_back(L"/O2");
#endif
	vector<DxcDefine> defineMacros;
	for (const auto& v : defines)
	{
		defineMacros.push_back(
			DxcDefine{
			  v.Name.c_str(),
			  v.Value.c_str()
			});
	}

	IDxcIncludeHandler* handler;
	library->CreateIncludeHandler(&handler);

	compiler->Compile(
		source.Get(), filePath.wstring().c_str(),
		entryPoint.c_str(),
		profile.c_str(),
		compilerFlags.data(), UINT(compilerFlags.size()),
		defineMacros.data(), UINT(defineMacros.size()),
		handler,
		&dxcResult
	);

	ComPtr<IDxcBlobEncoding> errBlob;
	HRESULT hr;

	hr = dxcResult->GetErrorBuffer(&errBlob);
	if (SUCCEEDED(hr))
	{
		ComPtr<IDxcBlobEncoding> msgBlob;
		library->GetBlobAsUtf16(errBlob.Get(), &msgBlob);

		std::wstring msg((const wchar_t*)msgBlob->GetBufferPointer());
		OutputDebugStringW(msg.c_str());
	}

	dxcResult->GetStatus(&hr);
	if (SUCCEEDED(hr)) {
		hr = dxcResult->GetResult(
			reinterpret_cast<IDxcBlob**>(mCode.GetAddressOf())
		);
	}
	if (FAILED(hr))
	{
		throw runtime_error("shader compile failed.");
	}
}