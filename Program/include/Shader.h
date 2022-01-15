#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <d3d12.h>
#include <dxgi1_6.h>

#include "../common/include/d3dx12.h"
#include <wrl.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Shader
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	Shader() = default;
	enum Stage
	{
		Vertex, Geometry, Pixel,
		Domain, Hull,
		Compute,
	};

	struct DefineMacro {
		std::wstring Name;
		std::wstring Value;
	};

	void load(const std::wstring& fileName, Stage stage,
		const std::wstring& entryPoint,
		const std::vector<std::wstring>& flags,
		const std::vector<DefineMacro>& defines);

	const Microsoft::WRL::ComPtr<ID3DBlob>& getCode() const { return mCode; }

private:
	Microsoft::WRL::ComPtr<ID3DBlob> mCode;
};