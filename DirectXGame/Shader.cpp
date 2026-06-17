#include "Shader.h"
#include "MiscUtility.h"
#include <cassert>

#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;

// コンストラクタ
Shader::Shader() {}

// デストラクタ
Shader::~Shader() {}

// 生成したコンパイル済みデータを取得する
void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	std::string mbShaderModel = WStringToString(shaderModel);

	// シェーダーファイルを読み込んでコンパイルする
	HRESULT hr =
	    D3DCompileFromFile(filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", mbShaderModel.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shaderBlob, &errorBlob);

	// コンパイルに失敗した場合はエラーメッセージを出力してアサート
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}

	// コンパイルに成功した場合はシェーダーブロブを返す
	blob_ = shaderBlob;
}

// 外部コンパイラーを使用してシェーダーをコンパイルする関数
void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel) {

	// DXCを初期化
	static ComPtr<IDxcUtils> dxcUtils;
	static ComPtr<IDxcCompiler3> dxcCompiler;
	static ComPtr<IDxcIncludeHandler> dxcIncludeHandler;

	HRESULT hr;

	/// うまくいかなかったら起動できないようにする
	if (dxcUtils == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		assert(SUCCEEDED(hr));
	}

	if (dxcCompiler == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		assert(SUCCEEDED(hr));
	}

	if (dxcIncludeHandler == nullptr) {
		hr = dxcUtils->CreateDefaultIncludeHandler(&dxcIncludeHandler);
		assert(SUCCEEDED(hr));
	}

	// hlslファイルを読み込む
	ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容をDxcBufferに変換する
	DxcBuffer shaderSourceBuffer = {};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF-8エンコードされたテキスト

	// シェーダーをコンパイルする
	LPCWSTR arguments[] = {
	    filePath.c_str(), // コンパイルするファイルのパス
	    L"-E",
	    L"main", // エントリーポイントを指定
	    L"-T",
	    shaderModel.c_str(), // シェーダーモデルを指定
	    L"-Zi",              // デバッグ情報を生成する
	    L"-Qembed_debug",    // デバッグ情報をシェーダーブロブに埋め込む
	    L"-Od",              // 最適化を無効にする
	    L"-Zpr",             // プリプロセッサの出力を有効にする

	};

	// コンパイルに成功した場合はシェーダーブロブを返す
	ComPtr<IDxcResult> compileResult = nullptr;
	hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments), dxcIncludeHandler.Get(), IID_PPV_ARGS(&compileResult));

	// コンパイルに失敗した場合はエラーメッセージを出力してアサート
	assert(SUCCEEDED(hr));

	// 警告がある場合はエラーメッセージを出力する
	ComPtr<IDxcBlobUtf8> shaderError;
	ComPtr<IDxcBlobWide> nameBlob;

	compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), &nameBlob);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		OutputDebugStringA(shaderError->GetStringPointer());
		assert(false);
	}

	// コンパイル結果を受け取る
	ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &nameBlob);
	assert(SUCCEEDED(hr));

	// リソース解放

	// 実行用のバイナリを取っておく
	dxcBlob_ = shaderBlob;
}

ComPtr<ID3DBlob> Shader::GetShaderBlob() { return blob_; }
ComPtr<IDxcBlob> Shader::GetDxcBlob() { return dxcBlob_; }