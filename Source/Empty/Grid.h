// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSprite.h"
#include "Tile.h"
#include "Grid.generated.h"

USTRUCT(BlueprintType)
struct FTypeTile
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UPaperSprite* sprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ATile> tile_class;
};

UCLASS()
class EMPTY_API AGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrid();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void InitGrid();

	void OnSelectTile(ATile* tile);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSelectedTile(ATile* tile);

	void OnSwapDisplayFinished(ATile* tile);

	//Событие тайл закончил падение
	void OnTileFinishedFalling(ATile* tile, int newLogicIndex);

	ATile* CreateTile(int colum, int row, int logic_index);
	//Генерация новых тайлов
	void RespawnTiles();
	//Выполнить удаление совпавших тайлов, вычисление падающих, генерацию новых
	void ExecuteMatch();

	TArray<ATile*> FindMatch(ATile* tile);
	TArray<ATile*> FindFall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CountColum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CountRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TileWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TileHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTypeTile> TileLibrary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ATile* SelectedTile;

	//Игровая сетка тайлов
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ATile*> GameTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ATile*> MatchTiles;

	//Список падающих тайлов
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ATile*> FallTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int CountSwapFinished;
	
};
