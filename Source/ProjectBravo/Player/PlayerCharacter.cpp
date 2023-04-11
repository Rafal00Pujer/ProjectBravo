// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../UE_5.0/Engine/Plugins/Experimental/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h"
#include "../../../UE_5.0/Engine/Plugins/Experimental/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);

    ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
    ArmsMesh->SetupAttachment(Camera);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(ArmsMesh, TEXT("GripPoint"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Dash setup.
    CurrentDashCharges = MaxDashCharges;
    DashSpeed = DashDistance / DashTime;
    MinDashSpeed = DashSpeed * MinDashSpeedPercent;

    // Health and armor setup.
    CurrentHealth = MaxHealth;
    CurrentArmor = MaxArmor;

    float SumOfPercentages = DamageToHealth + DamageToArmor;
    checkf(SumOfPercentages != 0, TEXT("DamageToHealth + DamageToArmor = 0!!!"));

    if (SumOfPercentages != 1)
    {
        float PercentageScale = 1 / SumOfPercentages;
        DamageToHealth *= PercentageScale;
        DamageToArmor *= PercentageScale;

        UE_LOG(LogTemp, Warning, TEXT("Scaling DamageToHealth: %f and DamageToArmor: %f"), DamageToHealth, DamageToArmor);
    }
    // Health and armor setup end.

    CreateWidgetAndUpdateUI();

    // Weapons setup.
    WeaponMaterial = WeaponMesh->CreateAndSetMaterialInstanceDynamic(0);
    UpdateWeaponColor();
    SetVisibilityOnMeshes(false);

    for (TSubclassOf<UPlayerWeapon> WeaponClass : WeaponsClass)
    {
        PlayerWeapons.Add(NewObject<UPlayerWeapon>(this, WeaponClass));
    }

    for (int32 WeaponIndex : StartingWeapons)
    {
        check(PlayerWeapons.IsValidIndex(WeaponIndex));

        UPlayerWeapon* Weapon = PlayerWeapons[WeaponIndex];

        Weapon->SetIsHidden(false);
        Weapon->AddAmmo(Weapon->GetMaxPocketAmmo());

        if (SelectedWeapon == nullptr)
        {
            SelectedWeapon = Weapon;
            SelectedWeapon->SetIsSelected(true);

            SetVisibilityOnMeshes(true);
            UpdateWeaponColor();
            UpdateAmmoUI();
            UpdateWeaponNameAndColorUI();
            BindWeaponInputs();
        }
    }

    // Weapons setup end.
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickDash();
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    check(EnhancedComponent);

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    check(PlayerController);

    UEnhancedInputLocalPlayerSubsystem* EnhancedSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
    check(EnhancedSubsystem);

    EnhancedSubsystem->ClearAllMappings();
    EnhancedSubsystem->AddMappingContext(PlayerCharacterGameplayMapping, 0);

    LookInputHandle = EnhancedComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &APlayerCharacter::Look).GetHandle();

    JumpInputHandle = EnhancedComponent->BindAction(JumpInput, ETriggerEvent::Started, this, &APlayerCharacter::Jump).GetHandle();

    StopJumpInputHandle = EnhancedComponent->BindAction(JumpInput, ETriggerEvent::Completed, this, &APlayerCharacter::StopJumping).GetHandle();

    BindMoveAndDashInputs();
    BindWeaponChangeInputs();
}

void APlayerCharacter::SetVisibilityOnMeshes(bool NewVisibility)
{
    WeaponMesh->SetVisibility(NewVisibility);
    ArmsMesh->SetVisibility(NewVisibility);
}

UCameraComponent* APlayerCharacter::GetCameraComponent()
{
    return Camera;
}

#pragma region Inputs
void APlayerCharacter::RemoveWeaponChangeInputs()
{
    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(InputComponent);
    check(EnhancedComponent);
    EnhancedComponent->RemoveBindingByHandle(WeaponChangeInputHandle);
}

void APlayerCharacter::BindWeaponChangeInputs()
{
    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(InputComponent);
    check(EnhancedComponent);
    WeaponChangeInputHandle = EnhancedComponent->BindAction(WeaponChangeInput, ETriggerEvent::Started, this, &APlayerCharacter::WeaponChangeInputFunc).GetHandle();
}

void APlayerCharacter::RemoveMoveAndDashInputs()
{
    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(InputComponent);
    check(EnhancedComponent);

    EnhancedComponent->RemoveBindingByHandle(DashInputHandle);
    EnhancedComponent->RemoveBindingByHandle(MoveInputHandle);
}

void APlayerCharacter::BindMoveAndDashInputs()
{
    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(InputComponent);
    check(EnhancedComponent);

    MoveInputHandle = EnhancedComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &APlayerCharacter::Move).GetHandle();

    DashInputHandle = EnhancedComponent->BindAction(DashInput, ETriggerEvent::Started, this, &APlayerCharacter::StartDash).GetHandle();
}

void APlayerCharacter::RemoveWeaponInputs()
{
    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(InputComponent);
    check(EnhancedComponent);

    EnhancedComponent->RemoveBindingByHandle(FireInputHandle);
    EnhancedComponent->RemoveBindingByHandle(ReloadInputHandle);
    /*EnhancedComponent->RemoveBindingByHandle(RevolverInputHandle);*/
}

void APlayerCharacter::BindWeaponInputs()
{
    UEnhancedInputComponent* EnhancedComponent = Cast<UEnhancedInputComponent>(InputComponent);
    check(EnhancedComponent);

    FireInputHandle = EnhancedComponent->BindAction(FireInput, ETriggerEvent::Triggered, this, &APlayerCharacter::Fire).GetHandle();

    ReloadInputHandle = EnhancedComponent->BindAction(ReloadInput, ETriggerEvent::Started, this, &APlayerCharacter::Reload).GetHandle();

    /*RevolverInputHandle = EnhancedComponent->BindAction(RevolverInput, ETriggerEvent::Started, this, &APlayerCharacter::ActivateRevolver).GetHandle();*/
}
#pragma endregion

#pragma region Movement
void APlayerCharacter::Immobilize(float ImmobilizeTime)
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    FTimerManager& TimerManager = GetWorldTimerManager();

    if (TimerManager.IsTimerActive(DashTimer))
    {
        return;
    }

    if (TimerManager.IsTimerActive(ImmobilizeTimer) && TimerManager.GetTimerRemaining(ImmobilizeTimer) >= ImmobilizeTime)
    {
        return;
    }

    TimerManager.SetTimer(ImmobilizeTimer, this, &APlayerCharacter::ImmobilizeEnd, ImmobilizeTime);

    RemoveMoveAndDashInputs();
}

void APlayerCharacter::ImmobilizeEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    BindMoveAndDashInputs();
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    AddMovementInput(GetActorForwardVector() * Value.Get<FInputActionValue::Axis2D>().X);
    AddMovementInput(GetActorRightVector() * Value.Get<FInputActionValue::Axis2D>().Y);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    AddControllerYawInput(Value.Get<FInputActionValue::Axis2D>().X);
    AddControllerPitchInput(Value.Get<FInputActionValue::Axis2D>().Y);
}
#pragma endregion

#pragma region Dash
void APlayerCharacter::StartDash()
{
    //UE_LOG(LogTemp, Warning, TEXT("Start Dash"));

    FTimerManager& TimerManager = GetWorldTimerManager();

    if (CurrentDashCharges <= 0)
    {
        return;
    }

    CurrentDashCharges--;
    UpdateDashUI();

    if (!TimerManager.IsTimerActive(AddDashChargeTimer))
    {
        TimerManager.SetTimer(AddDashChargeTimer, this, &APlayerCharacter::AddDashCharge, DashChargeCooldown, true);
    }

    RemoveMoveAndDashInputs();

    BrakingFrictionFactorSave = GetCharacterMovement()->BrakingFrictionFactor;
    GetCharacterMovement()->BrakingFrictionFactor = 0;

    BrakingDecelerationWalkingSave = GetCharacterMovement()->BrakingDecelerationWalking;
    GetCharacterMovement()->BrakingDecelerationWalking = 0;

    FVector DashDirection = GetLastMovementInputVector().GetSafeNormal();

    // Dash forward if last input vector is zero.
    if (DashDirection.IsNearlyZero())
    {
        DashDirection = GetActorForwardVector();
    }

    LaunchCharacter(DashDirection * DashSpeed, true, false);

    TimerManager.SetTimer(DashTimer, this, &APlayerCharacter::EndDash, DashTime);
}

void APlayerCharacter::TickDash()
{
    if (GetWorldTimerManager().IsTimerActive(DashTimer) && GetVelocity().Length() <= MinDashSpeed)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Tick Dash True"));

        GetWorldTimerManager().ClearTimer(DashTimer);
        EndDash();
    }
}

void APlayerCharacter::EndDash()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    LaunchCharacter(FVector::ZeroVector, true, false);

    GetCharacterMovement()->BrakingFrictionFactor = BrakingFrictionFactorSave;
    GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalkingSave;

    BindMoveAndDashInputs();
}

void APlayerCharacter::AddDashCharge()
{
    //UE_LOG(LogTemp, Warning, TEXT("Add Dash Charge"));

    CurrentDashCharges++;
    UpdateDashUI();

    if (CurrentDashCharges == MaxDashCharges)
    {
        GetWorldTimerManager().ClearTimer(AddDashChargeTimer);
    }
}
#pragma endregion

#pragma region Health
void APlayerCharacter::AddPhysicalDamage(float DamageAmount, FVector HitDirectionInWorld)
{
    if (DamageAmount == 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Skipping damage. Damage amount = %f."), DamageAmount);
        return;
    }

    FRotator HitDirection = HitDirectionInWorld.Rotation();
    HitDirection = GetActorTransform().InverseTransformRotation(HitDirection.Quaternion()).Rotator();

    if (HitDirection.Yaw < -90.0f || HitDirection.Yaw > 90.0f)
    {
        DamageAmount *= (1 - DamageReductionFromBack);

        UE_LOG(LogTemp, Warning, TEXT("Damage from Back!!!"));
    }
    else if (SelectedWeapon != nullptr && SelectedWeapon->IsShieldActive())
    {
        DamageAmount = SelectedWeapon->AbsorbDamage(DamageAmount);

        UE_LOG(LogTemp, Warning, TEXT("Damage absorbed by the weapon shield!!!"));
    }

    /*float DamageToHealthInternal;*/

    if (CurrentArmor > 0)
    {
        DamageAmount *= (1 - DamageReductionWithArmor);
        CurrentArmor -= DamageAmount * DamageToArmor;
        //DamageToHealthInternal = DamageAmount * DamageToHealth;
        DamageAmount *= DamageToHealth;

        if (CurrentArmor < 0)
        {
            CurrentArmor = 0;
        }
    }
    /*else
    {
        DamageToHealthInternal = DamageAmount;
    }*/

    if (HasCriticalHealth())
    {
        //CurrentHealth -= DamageToHealthInternal * (1 - CriticalHealthDamageReduction);
        CurrentHealth -= DamageAmount * (1 - CriticalHealthDamageReduction);
    }
    else
    {
        //CurrentHealth -= DamageToHealthInternal;
        CurrentHealth -= DamageAmount;

        if (HasCriticalHealth())
        {
            CurrentHealth = MaxHealth * CriticalHealthActivation;
        }
    }

    if (CurrentHealth <= 0)
    {
        CurrentHealth = 0;

        UE_LOG(LogTemp, Warning, TEXT("Player died!!!"));

        // Insert Game Over logic.___________________________________________________
    }

    UE_LOG(LogTemp, Warning, TEXT("Damage received! Health: %f, Armor: %f."), CurrentHealth, CurrentArmor);

    UpdateHealtAndArmorUI();
}

void APlayerCharacter::Heal(float HealthAmount, float ArmorAmount)
{
    if (HasCriticalHealth())
    {
        HealthAmount *= (1 + HealingIncreaseOnCriticalHealth);
    }

    CurrentHealth = FMath::Clamp(CurrentHealth + HealthAmount, 0, MaxHealth);

    CurrentArmor = FMath::Clamp(CurrentArmor + ArmorAmount, 0, MaxArmor);

    UE_LOG(LogTemp, Warning, TEXT("Healing received! Health: %f, Armor: %f."), CurrentHealth, CurrentArmor);

    UpdateHealtAndArmorUI();
}

bool APlayerCharacter::HasCriticalHealth()
{
    return CurrentHealth / MaxHealth <= CriticalHealthActivation;
}

float APlayerCharacter::GetCurrentHealth()
{
    return CurrentHealth;
}

float APlayerCharacter::GetMaxHealth()
{
    return MaxHealth;
}

float APlayerCharacter::GetCurrentArmor()
{
    return CurrentArmor;
}

float APlayerCharacter::GetMaxArmor()
{
    return MaxArmor;
}

#pragma endregion

#pragma region Weapons
void APlayerCharacter::AddAmmo(FGameplayTag WeaponTag, int32 AmmoAmount)
{
    for (int32 i = 0; i < PlayerWeapons.Num(); i++)
    {
        if (WeaponTag == PlayerWeapons[i]->GetWeaponTag() && !PlayerWeapons[i]->GetIsHidden())
        {
            UPlayerWeapon* Weapon = PlayerWeapons[i];

            if (Weapon->GetCurrentPocketAmmo() >= Weapon->GetMaxPocketAmmo())
            {
                return;
            }

            Weapon->AddAmmo(AmmoAmount);

            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Weapon Tag not found."));
}

void APlayerCharacter::AddWeapon(FGameplayTag WeaponTag)
{
    for (int32 i = 0; i < PlayerWeapons.Num(); i++)
    {
        if (WeaponTag == PlayerWeapons[i]->GetWeaponTag())
        {
            UPlayerWeapon* Weapon = PlayerWeapons[i];

            if (Weapon->GetIsHidden())
            {
                Weapon->SetIsHidden(false);

                if (bAutoWeaponPickUp && !GetWorldTimerManager().IsTimerActive(WeaponChangeTimer))
                {
                    WeaponChange(i);
                }
            }
            else
            {
                Weapon->AddAmmo(Weapon->GetMaxClipAmmo());
            }

            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Weapon Tag not found."));
}

void APlayerCharacter::PlayArmsMontage(UAnimMontage* Montage, float Speed)
{
    ArmsMesh->GetAnimInstance()->Montage_Play(Montage, Speed);
}

void APlayerCharacter::UpdateWeaponColor()
{
    check(WeaponMaterial);

    if (!SelectedWeapon)
    {
        WeaponMaterial->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor::White);
    }
    else
    {
        WeaponMaterial->SetVectorParameterValue(TEXT("BodyColor"), SelectedWeapon->GetWeaponColor());
    }
}

void APlayerCharacter::Fire()
{
    //UE_LOG(LogTemp, Error, TEXT("%s"), *FString(__FUNCTION__));

    check(SelectedWeapon);

    if (SelectedWeapon->CanFire())
    {
        SelectedWeapon->Fire();
    }
}

void APlayerCharacter::Reload()
{
    //UE_LOG(LogTemp, Error, TEXT("%s"), *FString(__FUNCTION__));

    check(SelectedWeapon);

    if (SelectedWeapon->CanBeReloaded())
    {
        SelectedWeapon->Reload();
    }
}

void APlayerCharacter::WeaponChangeInputFunc(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("%s, value = %f"), *FString(__FUNCTION__), Value.Get<FInputActionValue::Axis1D>());

    // Subtract one, because first weapon is at index 0.
    WeaponChange(Value.Get<FInputActionValue::Axis1D>() - 1.0f);
}

void APlayerCharacter::WeaponChange(int32 WeaponIndex)
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    NextWeaponIndex = WeaponIndex;
    bool NextWeaponIsNone = NextWeaponIndex == -1;
    bool HasWeaponInHand = SelectedWeapon != nullptr;

    // non weapon -> non weapon
    if (!HasWeaponInHand && NextWeaponIsNone)
    {
        return;
    }

    bool NextWeaponIndexIsValid = PlayerWeapons.IsValidIndex(NextWeaponIndex);

    // current weapon == next weapon
    if (HasWeaponInHand && NextWeaponIndexIsValid && SelectedWeapon == PlayerWeapons[NextWeaponIndex])
    {
        return;
    }

    bool CurrentWeaponCanBeChanged = HasWeaponInHand && SelectedWeapon->CanBeChangedOut();
    bool NextWeaponCanBeChanged = NextWeaponIndexIsValid
        && PlayerWeapons[NextWeaponIndex]->CanBeChangedIn();

    // current weapon -> next weapon || current weapon -> non weapon
    if (CurrentWeaponCanBeChanged && (NextWeaponCanBeChanged || NextWeaponIsNone))
    {
        StartChangeOutWeapon();
    }
    // non weapon -> next weapon
    else if (!HasWeaponInHand && NextWeaponCanBeChanged)
    {
        StartChangeInWeapon();
    }
}

void APlayerCharacter::StartChangeOutWeapon()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    RemoveWeaponChangeInputs();
    RemoveWeaponInputs();

    TTuple<float, UAnimMontage*, float> ChangeOutInfo = SelectedWeapon->ChangeOut();

    PlayArmsMontage(ChangeOutInfo.Get<1>(), ChangeOutInfo.Get<2>());

    GetWorldTimerManager().SetTimer(WeaponChangeTimer, this, &APlayerCharacter::EndChangeOutWeapon, ChangeOutInfo.Get<0>());
}

void APlayerCharacter::EndChangeOutWeapon()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    // current weapon -> non weapon
    if (NextWeaponIndex == -1)
    {
        SelectedWeapon = nullptr;

        BindWeaponChangeInputs();

        SetVisibilityOnMeshes(false);

        UpdateWeaponColor();
        UpdateAmmoUI();
        UpdateWeaponNameAndColorUI();

        return;
    }
    // current weapon -> next weapon
    else
    {
        StartChangeInWeapon();
    }
}

void APlayerCharacter::StartChangeInWeapon()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    RemoveWeaponChangeInputs();

    SelectedWeapon = PlayerWeapons[NextWeaponIndex];
    TTuple<float, UAnimMontage*, float> ChangeInInfo = SelectedWeapon->ChangeIn();

    SetVisibilityOnMeshes(true);
    PlayArmsMontage(ChangeInInfo.Get<1>(), ChangeInInfo.Get<2>());

    UpdateWeaponColor();
    UpdateAmmoUI();
    UpdateWeaponNameAndColorUI();

    GetWorldTimerManager().SetTimer(WeaponChangeTimer, this, &APlayerCharacter::EndChangeInWeapon, ChangeInInfo.Get<0>());
}

void APlayerCharacter::EndChangeInWeapon()
{
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));

    BindWeaponChangeInputs();
    BindWeaponInputs();

    if (SelectedWeapon->GetCurrentClipAmmo() <= 0 && SelectedWeapon->CanBeReloaded())
    {
        SelectedWeapon->Reload();
    }
}
#pragma endregion

#pragma region UI
void APlayerCharacter::CreateWidgetAndUpdateUI()
{
    check(PlayerWidgetClass);

    PlayerWidget = CreateWidget<UPlayerWidget>(GetWorld(), PlayerWidgetClass);
    PlayerWidget->AddToViewport();

    UpdateHealtAndArmorUI();
    UpdateDashUI();
    UpdateWeaponNameAndColorUI();
    UpdateAmmoUI();
}

void APlayerCharacter::UpdateHealtAndArmorUI()
{
    check(PlayerWidget);

    PlayerWidget->UpdateHealthAndArmor(CurrentHealth / MaxHealth, CurrentArmor / MaxArmor);
}

void APlayerCharacter::UpdateDashUI()
{
    check(PlayerWidget);

    PlayerWidget->UpdateDash(CurrentDashCharges);
}

void APlayerCharacter::UpdateAmmoUI()
{
    check(PlayerWidget);

    if (!SelectedWeapon)
    {
        PlayerWidget->UpdateAmmo(false, 0, 0, 0);
    }
    else
    {
        PlayerWidget->UpdateAmmo(SelectedWeapon->GetIsReloading(), SelectedWeapon->GetCurrentClipAmmo(), SelectedWeapon->GetMaxClipAmmo(), SelectedWeapon->GetCurrentPocketAmmo());
    }
}

void APlayerCharacter::UpdateWeaponNameAndColorUI()
{
    check(PlayerWidget);

    if (!SelectedWeapon)
    {
        PlayerWidget->UpdateWeaponNameAndColor(FText(), FLinearColor::White);
    }
    else
    {
        PlayerWidget->UpdateWeaponNameAndColor(SelectedWeapon->GetWeaponName(), SelectedWeapon->GetWeaponColor());
    }
}
#pragma endregion

#pragma region Console Comands
void APlayerCharacter::AddPhysicalDamageConsole(float DamageAmount)
{
    AddPhysicalDamage(DamageAmount, FVector());
}

void APlayerCharacter::HealConsole(float HealthAmount, float ArmorAmount)
{
    Heal(HealthAmount, ArmorAmount);
}

void APlayerCharacter::AddAmmoConsole(FGameplayTag WeaponTag, int32 AmmoAmount)
{
    AddAmmo(WeaponTag, AmmoAmount);
}

void APlayerCharacter::AddWeaponConsole(FGameplayTag WeaponTag)
{
    AddWeapon(WeaponTag);
}

void APlayerCharacter::AddAllWeaponsConsole()
{
    for (int32 i = 0; i < PlayerWeapons.Num(); i++)
    {
        /*if (!PlayerWeapons[i]->GetIsHidden())
        {
            continue;
        }

        PlayerWeapons[i]->SetIsHidden(false);
        PlayerWeapons[i]->SetPocketAmmo(10000);*/

        UPlayerWeapon* Weapon = PlayerWeapons[i];

        if (Weapon->GetIsHidden())
        {
            Weapon->SetIsHidden(false);

            if (bAutoWeaponPickUp && !GetWorldTimerManager().IsTimerActive(WeaponChangeTimer))
            {
                WeaponChange(i);
            }
        }
        else
        {
            Weapon->AddAmmo(Weapon->GetMaxClipAmmo());
        }
    }
}

void APlayerCharacter::RemoveWeaponConsole(FGameplayTag WeaponTag)
{
    for (int32 i = 0; i < PlayerWeapons.Num(); i++)
    {
        if (WeaponTag == PlayerWeapons[i]->GetWeaponTag())
        {
            if (PlayerWeapons[i]->GetIsHidden())
            {
                UE_LOG(LogTemp, Warning, TEXT("Weapon already hidden."));
                return;
            }

            if (SelectedWeapon == PlayerWeapons[i])
            {
                UE_LOG(LogTemp, Warning, TEXT("Can't hide currently selected weapon. Change to a different weapon."));
                return;
            }

            PlayerWeapons[i]->SetIsHidden(true);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Weapon FName not found."));
}

void APlayerCharacter::RemoveAllWeaponsConsole()
{
    for (int32 i = 0; i < PlayerWeapons.Num(); i++)
    {
        if (PlayerWeapons[i]->GetIsHidden())
        {
            continue;
        }

        if (SelectedWeapon == PlayerWeapons[i])
        {
            UE_LOG(LogTemp, Warning, TEXT("Can't hide currently selected weapon. Change to a different weapon."));
            continue;
        }

        PlayerWeapons[i]->SetIsHidden(true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Weapon FName not found."));
}

void APlayerCharacter::HideCurrentWeaponConsole()
{
    WeaponChange(-1);
}
#pragma endregion