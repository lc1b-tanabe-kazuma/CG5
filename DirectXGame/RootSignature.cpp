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