// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "PlayerWidget.generated.h"

/**
 *
 */
UCLASS()
class PROJECTBRAVO_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPlayerWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UProgressBar* ArmorBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UProgressBar* HealthBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UProgressBar* DashBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UTextBlock* ArmorTextBlock;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UTextBlock* HealthTextBlock;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UTextBlock* DashTextBlock;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UImage* WeaponColor;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UTextBlock* WeaponName;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UTextBlock* AmmoTextBlock;

	/*UPROPERTY(VisibleAnywhere, meta = (BindWidget))
		UTextBlock* RevolverTextBlock;*/

	UPROPERTY(EditDefaultsOnly)
		FText ArmorText;

	UPROPERTY(EditDefaultsOnly)
		FText HealthText;

	UPROPERTY(EditDefaultsOnly)
		FText DashReadyText;

	UPROPERTY(EditDefaultsOnly)
		FText DashRechargingText;

	UPROPERTY(EditDefaultsOnly)
		FText AmmoText;

	UPROPERTY(EditDefaultsOnly)
		FText ReloadingText;

	/*UPROPERTY(EditDefaultsOnly)
		FText RevolverChargesText;

	UPROPERTY(EditDefaultsOnly)
		FText RevolverActiveText;*/

	UPROPERTY(EditDefaultsOnly)
		FLinearColor GoodColor;

	UPROPERTY(EditDefaultsOnly)
		FLinearColor BadColor;

public:
	void UpdateHealthAndArmor(float HealthPercent, float ArmorPercent);

	void UpdateDash(int32 DashCharges);

	void UpdateAmmo(bool IsReloading, int32 CurrentClipAmmo, int32 MaxClipAmmo, int32 PocketAmmo);

	void UpdateWeaponNameAndColor(FText Name, FLinearColor Color);

	/*void UpdateRevolver(bool InRevolverMode, int32 RevolverCharges);*/
};
