// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityLandShake.h"

UGravityLandShake::UGravityLandShake()
{
	bSingleInstance = true;

	OscillationDuration = 0.4f;
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.4f;

	RotOscillation.Pitch.Amplitude = 0.5f;
	RotOscillation.Pitch.Frequency = 12.f;

	RotOscillation.Yaw.Amplitude = 0.f;
	RotOscillation.Yaw.Frequency = 5.f;

	RotOscillation.Roll.Amplitude = 5.f;
	RotOscillation.Roll.Frequency = 50.f;

	LocOscillation.Z.Amplitude = 1.f;
	LocOscillation.Z.Frequency = 5.f;

	FOVOscillation.Amplitude = 10.f;
	FOVOscillation.Frequency = 2.0f;

	AnimBlendOutTime = 0.f;
}
