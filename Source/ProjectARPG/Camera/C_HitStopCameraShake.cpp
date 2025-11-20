// Fill out your copyright notice in the Description page of Project Settings.


#include "C_HitStopCameraShake.h"

UC_HitStopCameraShake::UC_HitStopCameraShake()
{
	OscillationDuration = 0.15f; // 카메라 흔들림 전체 지속 시간
	OscillationBlendInTime = 0.02f; // 흔들림이 시작될 때 점점 강해지는 시간
	OscillationBlendOutTime = 0.05f; // 흔들림이 끝날 때 점점 약해지는 시간


	// 카메라 상하 회전 흔들림
	RotOscillation.Pitch.Amplitude = 2.f;   
	RotOscillation.Pitch.Frequency = 25.f;


	// 카메라 좌우 회전 흔들림
	RotOscillation.Yaw.Amplitude = 2.f;
	RotOscillation.Yaw.Frequency = 25.f;

	// 카메라 회전(기울기) 흔들림
	RotOscillation.Roll.Amplitude = 1.5f;
	RotOscillation.Roll.Frequency = 22.f;

	// 카메라 위아래 이동 흔들림
	LocOscillation.Z.Amplitude = 3.f;
	LocOscillation.Z.Frequency = 30.f;
}
