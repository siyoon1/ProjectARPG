// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseCharacter.h"
#include "ProjectARPG/Interface/C_Interactable.h"
#include "C_NPCCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_NPCCharacter : public AC_BaseCharacter, public IC_Interactable
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class UWidgetComponent* m_InteractWidgetComp;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	TArray<FText> m_DialogueLine;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	bool m_bHasShop = true;

protected:
	void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	virtual void interact_Implementation(AActor* Interator) override;

	UWidgetComponent* getInteractWidgetComp() const;

	UFUNCTION(BlueprintCallable)
	const TArray<FText>& getDialogueLine() const;
	
	UFUNCTION(BlueprintCallable)
	bool hasShop() const;

	
};
