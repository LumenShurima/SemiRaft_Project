// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

UCLASS()
class SEMIRAFT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION()
	void OnMySphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UInputMappingContext> IMC_Player;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<class USphereComponent> SphereComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="MyVar")
	TSubclassOf<class UHookAimUI> HookAimUIFactory;
	
	UPROPERTY()
	TObjectPtr<UHookAimUI> HookAimUI;
	
	// FItem 데이터를 입력하기 전 테스트용
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyVar")
	TSubclassOf<class AHook> HookClass;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyVar")
	TObjectPtr<class AItemBase> CurrentItem;
	
	// 라인트레이스를 Tick으로 계속 쏘고 마지막으로 부딪힌 것 저장하기
	UPROPERTY()
	TObjectPtr<class AItemBase> CurrentTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UInputAction> IA_MyInteract;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UInputAction> IA_LeftClick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UInputAction> IA_RightClick;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UCameraComponent> Cam;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UInventoryComponent> InventoryComp;
	
	void UpdatedChargingUI();
	
	// E키로 아이템 획득
	void OnPressedEKey();
	void OnLeftClickStarted();
	void OnLeftClickTriggered();
	void OnLeftClickCompleted();
	void OnRightClickStarted();
	
	void InteractionCheck();
};
