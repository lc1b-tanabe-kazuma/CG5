#include "WorldTransformEx.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void WorldTransformEx::UpdateMatrix() {
	// MakeAffineMatrixを呼び出して、matWorld_に代入する
	matWorld_ = MakeAffineMatrix();

	// 定数バッファに転送する
	TransferMatrix();
}

KamataEngine::Matrix4x4 WorldTransformEx::MakeAffineMatrix() {
	// スケーリング行列の作成
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale_);

	// 回転行列の作成
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);

	// 回転行列の合成（Z軸回転→X軸回転→Y軸回転の順）
	Matrix4x4 rotationMatrix = matRotZ * matRotX * matRotY;

	// 平行移動行列の作成
	Matrix4x4 translationMatrix = MakeTranslateMatrix(translation_);

	// アフィン行列の合成
	Matrix4x4 affineMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	return affineMatrix;
}