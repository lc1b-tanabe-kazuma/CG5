#pragma once
#include <d3d12.h>
#include <string>
#include <wrl.h>

class Shader {
public:
	// シェーダーファイルを読み込み、コンパイルしてバイナリデータを取得する関数
	void Load(const std::wstring& filePath, const std::string& shaderModel);

	// 生成したコンパイル済みデータを取得する
	Microsoft::WRL::ComPtr<ID3DBlob> GetShaderBlob();

	// コンストラクタ
	Shader();

	// デストラクタ
	~Shader();

private:
	// コンパイル済みのシェーダーのバイナリデータ
	Microsoft::WRL::ComPtr<ID3DBlob> blob_;
};