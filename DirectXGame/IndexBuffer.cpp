#include "IndexBuffer.h"
#include "KamataEngine.h"
#include <cassert>

using namespace Microsoft::WRL;
using namespace KamataEngine;

void IndexBuffer::Create(const UINT size, const UINT stride) {
	// strideの値によってインデックスのフォーマットを決める
	assert(stride == 2 || stride == 4);
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	// クラス内でdxCommonを使用するため、インスタンスを取得しておく
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 頂点リソース用のヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // アップロード可能なヒープ

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc = {};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // リソースの次元
	vertexResourceDesc.Width = size;                                // 頂点データ全体のサイズ

	// バッファの場合は1にする
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作成する
	ID3D12Resource* indexResource = nullptr;

	hr =
	    dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexResource));
#ifdef DEBUG
	assert(SUCCEEDED(hr));
#endif // DEBUG

	indexBuffer_ = indexResource;

	/// IndexBufferViewの作成
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};

	// リソースの先頭アドレスを指定
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

	// 使用するインデックスリソースのサイズを指定
	indexBufferView.SizeInBytes = size;

	// インデックス1つ分のサイズを指定
	indexBufferView.Format = format;

	indexBufferView_ = indexBufferView;
}

ID3D12Resource* IndexBuffer::GetResource() { return indexBuffer_.Get(); }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return &indexBufferView_; }

IndexBuffer::~IndexBuffer() { indexBuffer_ = nullptr; }