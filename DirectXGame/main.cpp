#include "KamataEngine.h"
#include <Windows.h>
#include <cassert>
#include <d3dcompiler.h>

using namespace KamataEngine;
using namespace Microsoft::WRL;

// シェーダーコンパイル関数
ComPtr<ID3DBlob> CompileShader(const std::wstring& filePath, const std::string& target) {
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	// シェーダーファイルを読み込んでコンパイルする
	HRESULT hr = D3DCompileFromFile(filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", target.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shaderBlob, &errorBlob);

	// コンパイルに失敗した場合はエラーメッセージを出力してアサート
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}

	// コンパイルに成功した場合はシェーダーブロブを返す
	return shaderBlob;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// KamataEngineの初期化
	KamataEngine::Initialize(L"タイトルバー");

	// DirectXCommonのインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXのウィンドウの幅と高さを取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf("ウィンドウの幅: %d, 高さ: %d\n", w, h);

	// DirectXCommonのコマンドリストを取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	/// RootSignatureの作成
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // 入力レイアウトを使用するフラグ

	Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob = nullptr; // ルートシグネチャのバイナリデータ
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlog = nullptr;         // エラーメッセージのバイナリデータ
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlog);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元にルートシグネチャを作成する
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayoutの作成
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";                        // 頂点シェーダー側のセマンティクス
	inputElementDescs[0].SemanticIndex = 0;                                // セマンティクスインデックス
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;             // データの形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;    // 入力要素の配列
	inputLayoutDesc.NumElements = _countof(inputElementDescs); // 入力要素の数

	// BlendStateの作成
	D3D12_BLEND_DESC blendDesc = {};

	// 全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerStateの作成
	D3D12_RASTERIZER_DESC rasterizerDesc = {};

	//  裏面をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

	// 塗りつぶしモードをソリッドにする
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// コンパイル済みのshader,エラー情報を格納するための変数
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileShader(L"Resources/shaders/TestVS.hlsl", "vs_5_0");    // 頂点シェーダーのバイナリデータ
	assert(vsBlob != nullptr);

	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompileShader(L"Resources/shaders/TestPS.hlsl", "ps_5_0");    // ピクセルシェーダーのバイナリデータ
	assert(psBlob != nullptr);

	// PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();                       // ルートシグネチャ
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                              // 入力レイアウト
	graphicsPipelineStateDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()}; // 頂点シェーダー
	graphicsPipelineStateDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()}; // ピクセルシェーダー
	graphicsPipelineStateDesc.BlendState = blendDesc;                                     // ブレンドステート
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                           // ラスタライザーステート

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                       // 書き込むRTVの数
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // RTVのフォーマット

	// 利用するトロポジーの情報
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // トポロジーのタイプ

	// どのように画面に色を表示するかの情報
	graphicsPipelineStateDesc.SampleDesc.Count = 1;                   // サンプリング数
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準のサンプルマスク

	// PSOの作成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	/// VertexResourceの作成
	// 頂点リソース用のヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // アップロード可能なヒープ

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc = {};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // リソースの次元
	vertexResourceDesc.Width = sizeof(Vector4) * 3;                 // 頂点データ全体のサイズ

	// バッファの場合は1にする
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作成する
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	/// VertexBufferViewの作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	// リソースの先頭アドレスを指定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用する頂点リソースのサイズを指定
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;

	// 頂点1つ分のサイズを指定
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// 頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f}; // 頂点1の位置
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};   // 頂点2の位置
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};  // 頂点3の位置
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画前処理
		dxCommon->PreDraw();

		/// ここに描画処理を記述

		// PSOの設定
		commandList->SetPipelineState(graphicsPipelineState.Get());

		// ルートシグネチャの設定
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		// 頂点バッファビューの設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

		// プリミティブトポロジーの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画コマンド
		commandList->DrawInstanced(3, 1, 0, 0);

		// 描画後処理
		dxCommon->PostDraw();
	}

	// 解放処理

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}