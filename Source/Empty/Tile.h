// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Tile.generated.h"

//UENUM()
enum TypeStateTile
{
	Normal, //Обычное состояние
	Fall, //Падает
	Delete //Помечена на удаление, после совпадения
};

/**
 * 
 */
UCLASS()
class EMPTY_API ATile : public APaperSpriteActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ATile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnClickedMy(AActor* Target, FKey ButtonPressed);

	UFUNCTION(BlueprintNativeEvent)
	void OnSwapMove(ATile* tile, bool isSwap);
	virtual void OnSwapMove_Implementation(ATile* tile, bool isSwap);

	void StartFalling();

	void SetSprite(UPaperSprite* sprite);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPROPERTY(BlueprintReadOnly)
	class AGrid* RefGrid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int LogicIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int TypeTile;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TypeStateTile StateTile;

	//Конечная позиция падения
	FVector FinishFallingLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NewLogicIndex;

	//Скорость падения
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpeedFalling;
};
