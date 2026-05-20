#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <string>
#include <wrl.h>

class Shader {
public:
	// シェーダーファイルを読み込み、コンパイルしてバイナリデータを取得する関数
	void Load(const std::wstring& filePath, const std::wstring& shaderModel);

	void LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel);

	// 生成したコンパイル済みデータを取得する
	Microsoft::WRL::ComPtr<ID3DBlob> GetShaderBlob();
	Microsoft::WRL::ComPtr<IDxcBlob> GetDxcBlob();

	// コンストラクタ
	Shader();

	// デストラクタ
	~Shader();

private:
	// コンパイル済みのシェーダーのバイナリデータ
	Microsoft::WRL::ComPtr<ID3DBlob> blob_;
	Microsoft::WRL::ComPtr<IDxcBlob> dxcBlob_;
};