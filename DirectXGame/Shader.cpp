#include "Shader.h"
#include <d3dcompiler.h>
#include <cassert>

using namespace Microsoft::WRL;

// コンストラクタ
Shader::Shader() {}

// デストラクタ
Shader::~Shader() { }

// 生成したコンパイル済みデータを取得する
void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) {
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	// シェーダーファイルを読み込んでコンパイルする
	HRESULT hr = D3DCompileFromFile(filePath.c_str(), 
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, 
		"main", shaderModel.c_str(),
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &shaderBlob, &errorBlob);

	// コンパイルに失敗した場合はエラーメッセージを出力してアサート
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}

	// コンパイルに成功した場合はシェーダーブロブを返す
	blob_= shaderBlob;
}

ComPtr<ID3DBlob> Shader::GetShaderBlob() { return blob_; }