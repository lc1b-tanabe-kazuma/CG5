#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// KamataEngineの初期化
	KamataEngine::Initialize(L"タイトルバー");

	// DirectXCommonのインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画前処理
		dxCommon->PreDraw();

		// ここに描画処理を記述

		// 描画後処理
		dxCommon->PostDraw();
	}

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
