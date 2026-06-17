#include "IndexBuffer.h"
#include "KamataEngine.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;
using namespace Microsoft::WRL;

//
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	// inputElementDescsの作成
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";                        // 頂点シェーダー側のセマンティクス
	inputElementDescs[0].SemanticIndex = 0;                                // セマンティクスインデックス
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;          // データの形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値
	inputElementDescs[1].SemanticName = "TEXCOORD";                        // 頂点シェーダー側のセマンティクス
	inputElementDescs[1].SemanticIndex = 0;                                // セマンティクスインデックス
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;                // データの形式
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};                 // 入力レイアウトの説明構造体
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

	// PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rs.GetRootSignature();                                       // ルートシグネチャ
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // 入力レイアウト
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // 頂点シェーダー
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // ピクセルシェーダー
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // ブレンドステート
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                       // 書き込むRTVの数
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // RTVのフォーマット

	// 利用するトロポジーの情報
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // トポロジーのタイプ

	// どのように画面に色を表示するかの情報
	graphicsPipelineStateDesc.SampleDesc.Count = 1;                   // サンプリング数
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準のサンプルマスク

	// PSOの作成
	pipelineState.Create(graphicsPipelineStateDesc);
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// KamataEngineの初期化
	KamataEngine::Initialize(L"CG5_LE3D_16_タナベ");

	// DirectXCommonのインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXのウィンドウの幅と高さを取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf("ウィンドウの幅: %d, 高さ: %d\n", w, h);

	// DirectXCommonのコマンドリストを取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	/// RootSignatureの作成
	RootSignature rs;
	rs.Create();

	Shader vsShader;
	vsShader.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vsShader.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader psShader;
	psShader.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(psShader.GetDxcBlob() != nullptr);

	// パイプラインステートの作成
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vsShader, psShader);

	struct VertexData {
		Vector4 position; // 頂点の位置
	};

	// 頂点データの準備
	VertexData vertices[] = {
	    {-1.0f, -1.0f, 0.0f, 1.0f}, // 0 左下
	    {-1.0f, 1.0f,  0.0f, 1.0f}, // 1 左上
	    {1.0f,  -1.0f, 0.0f, 1.0f}, // 2 右下
	    {1.0f,  1.0f,  0.0f, 1.0f}, // 3 右上
	};

	/// VertexResourceの作成
	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	/// 頂点リソースにデータを書き込む
	Vector4* pGpuVertices = nullptr;
	vb.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	// 頂点リソースをマップして、CPUから書き込めるようにする
	for (int i = 0; i < _countof(vertices); i++) {
		pGpuVertices[i] = vertices[i].position;
	}

	// 頂点データの準備
	uint16_t indices[] = {0, 1, 2, 2, 1, 3}; // 頂点のインデックス

	// indexBuffer
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点データを書き込む
	uint16_t* pGpuIndices = nullptr;
	ib.GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	// 頂点リソースをマップして、CPUから書き込めるようにする
	for (int i = 0; i < _countof(indices); i++) {
		pGpuIndices[i] = indices[i];
	}

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
		commandList->SetPipelineState(pipelineState.Get());

		// ルートシグネチャの設定
		commandList->SetGraphicsRootSignature(rs.GetRootSignature());

		// 頂点バッファビューの設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView());

		// IBVを設定
		commandList->IASetIndexBuffer(ib.GetView());

		// プリミティブトポロジーの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画コマンド
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画後処理
		dxCommon->PostDraw();
	}

	// 解放処理

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}