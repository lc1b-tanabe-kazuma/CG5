#include "RootSignature.h"
#include "MiscUtility.h"
#include <KamataEngine.h>
#include <cassert>

using namespace Microsoft::WRL;
using namespace KamataEngine;

RootSignature::RootSignature() {}

RootSignature::~RootSignature() {}

void RootSignature::Create() {

	// 既にルートシグネチャが作成されている場合は作成しない
	if (rootSignature_) {
		rootSignature_ = nullptr;
	}

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // 入力レイアウトを使用するフラグ

	// デスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};

	// レジスタを利用可能にする
	descriptorRange[0].BaseShaderRegister = 0;                                                   // レジスタ番号
	descriptorRange[0].NumDescriptors = 1;                                                       // デスクリプタの数
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;                              // SRVを利用する
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // デスクリプタテーブルの先頭からのオフセット値

	// RootParameterの設定
	D3D12_ROOT_PARAMETER rootParameter[1]{};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;      // デスクリプタテーブルを利用する
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                // ピクセルシェーダーで利用する
	rootParameter[0].DescriptorTable.pDescriptorRanges = descriptorRange;             // デスクリプタレンジの配列].
	rootParameter[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // デスクリプタレンジの数
	descriptionRootSignature.pParameters = rootParameter;                             // ルートパラメータの配列
	descriptionRootSignature.NumParameters = _countof(rootParameter);                 // ルートパラメータの数

	// samplerの設定

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

	rootSignature_ = rootSignature;
}

ID3D12RootSignature* RootSignature::GetRootSignature() const { return rootSignature_.Get(); }