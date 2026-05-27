#include "VertexBuffer.h"
#include "KamataEngine.h"
#include <cassert>
#include <d3d12.h>

using namespace Microsoft::WRL;
using namespace KamataEngine;

void VertexBuffer::Create(const UINT size, const UINT stride) {
	//
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
	HRESULT hr =
	    dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource_));
	assert(SUCCEEDED(hr));

	/// VertexBufferViewの作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	// リソースの先頭アドレスを指定
	vertexBufferView.BufferLocation = vertexResource_->GetGPUVirtualAddress();

	// 使用する頂点リソースのサイズを指定
	vertexBufferView.SizeInBytes = size;

	// 頂点1つ分のサイズを指定
	vertexBufferView.StrideInBytes = stride;

	vertexBufferView_ = vertexBufferView;
}

ID3D12Resource* VertexBuffer::GetResource() { return vertexResource_.Get(); }

D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::GetView() { return &vertexBufferView_; }

VertexBuffer::~VertexBuffer() { vertexResource_ = nullptr; }