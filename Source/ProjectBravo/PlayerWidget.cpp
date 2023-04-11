// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"

UPlayerWidget::UPlayerWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UPlayerWidget::UpdateHealthAndArmor(float HealthPercent, float ArmorPercent)
{
	HealthBar->SetPercent(HealthPercent);
	HealthBar->SetFillColorAndOpacity(FLinearColor::LerpUsingHSV(BadColor, GoodColor, HealthPercent));

	HealthTextBlock->SetText(FText::Format(HealthText, HealthPercent * 100));

	ArmorBar->SetPercent(ArmorPercent);
	ArmorBar->SetFillColorAndOpacity(FLinearColor::LerpUsingHSV(BadColor, GoodColor, ArmorPercent));

	ArmorTextBlock->SetText(FText::Format(ArmorText, ArmorPercent * 100));
}

void UPlayerWidget::UpdateDash(int32 DashCharges)
{
	if (DashCharges > 0)
	{
		DashBar->SetFillColorAndOpacity(GoodColor);
		DashTextBlock->SetText(FText::Format(DashReadyText, DashCharges));

		return;
	}

	DashBar->SetFillColorAndOpacity(BadColor);
	DashTextBlock->SetText(DashRechargingText);
}

void UPlayerWidget::UpdateAmmo(bool IsReloading, int32 CurrentClipAmmo, int32 MaxClipAmmo, int32 PocketAmmo)
{
	if (IsReloading)
	{
		AmmoTextBlock->SetText(FText::Format(ReloadingText, PocketAmmo));

		return;
	}

	AmmoTextBlock->SetText(FText::Format(AmmoText, CurrentClipAmmo, MaxClipAmmo, PocketAmmo));
}

void UPlayerWidget::UpdateWeaponNameAndColor(FText Name, FLinearColor Color)
{
	WeaponName->SetText(Name);
	WeaponColor->SetColorAndOpacity(Color);
}

//void UPlayerWidget::UpdateRevolver(bool InRevolverMode, int32 RevolverCharges)
//{
//	if (InRevolverMode)
//	{
//		RevolverTextBlock->SetText(RevolverActiveText);
//		return;
//	}
//
//	RevolverTextBlock->SetText(FText::Format(RevolverChargesText, RevolverCharges));
//}
