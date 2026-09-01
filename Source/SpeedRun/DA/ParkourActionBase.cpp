// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourActionBase.h"
#include "SpeedRunCharacter.h"
#include "ChooserFunctionLibrary.h"
#include "Animation/AnimMontage.h"

float UParkourActionBase::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	return 0.0f;
}

void UParkourActionBase::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
}

// 파생 액션들이 공통으로 쓰는 몽타주 선택 + 재생.
// EnvData 를 Chooser 파라미터로 넘겨 높이 / 깊이 / 속도 조건으로 몽타주를 고른다.
void UParkourActionBase::PlaySelectedMontage(ASpeedRunCharacter* Player, const FEnvData& EnvData) const
{
	if (!ActionChooser || !Player) return;

	FChooserEvaluationContext Context;

	FEnvData TempEnvData = EnvData;
	Context.AddStructParam(TempEnvData);

	FInstancedStruct ChooserStruct = UChooserFunctionLibrary::MakeEvaluateChooser(ActionChooser);

	UObject* ResultObj = UChooserFunctionLibrary::EvaluateObjectChooserBase(
		Context,
		ChooserStruct,
		UAnimMontage::StaticClass()
	);

	if (UAnimMontage* SelectedMontage = Cast<UAnimMontage>(ResultObj))
	{
		Player->PlayAnimMontage(SelectedMontage);
	}
}
