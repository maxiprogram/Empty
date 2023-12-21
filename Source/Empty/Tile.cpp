// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"
#include "Grid.h"

// Sets default values
ATile::ATile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if(RootComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("C++ ATile::ATile() SetMobility(Movable)"));
		RootComponent->SetMobility(EComponentMobility::Movable);
	}

	StateTile = TypeStateTile::Normal;
	SpeedFalling = 10.0f;
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("C++ BEGIN TILE"));

	SetActorTickEnabled(false);

	RefGrid = Cast<AGrid>(GetOwner());
	
	OnClicked.AddUniqueDynamic(this, &ATile::OnClickedMy);
}

// Called every frame
void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    UE_LOG(LogTemp, Warning, TEXT("C++ TICK TILE"));

	if(StateTile == TypeStateTile::Fall)
	{
		FVector location = GetActorLocation();
		location.Z += -SpeedFalling * DeltaTime;
		if(location.Z < FinishFallingLocation.Z)
		{
			location.Z = FinishFallingLocation.Z;

			StateTile = TypeStateTile::Normal;
			SetActorTickEnabled(false);

			RefGrid->OnTileFinishedFalling(this, NewLogicIndex);
			UE_LOG(LogTemp, Warning, TEXT("C++ TICK END"));
		}
		SetActorLocation(location);
	}
}

void ATile::SetSprite(UPaperSprite* sprite)
{
	bool result = false;
	if(sprite)
	{
		result = GetRenderComponent()->SetSprite(sprite);
	}
	UE_LOG(LogTemp, Warning, TEXT("C++ ATile::SetSprite() result=%d"), result);
}

void ATile::OnClickedMy(AActor* Target, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT("C++ ATile::OnClicked()"));

	//int current_col = LogicIndex % 5;
	//int current_row = (int)LogicIndex / 5;
	//UE_LOG(LogTemp, Warning, TEXT("C++ ATile::OnClicked() col=%d row=%d"), current_col, current_row);

	if(RefGrid)
	{
		if(RefGrid->SelectedTile)
		{
			UE_LOG(LogTemp, Warning, TEXT("C++ ATile::OnClicked() RefGrid->SelectedTile == true"));
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("C++ ATile::OnClicked() RefGrid->SelectedTile == false"));
		}
		RefGrid->OnSelectTile(this);
	}
}

void ATile::OnSwapMove_Implementation(ATile* tile, bool isSwap)
{
	RefGrid->OnSwapDisplayFinished(this);
}

void ATile::StartFalling()
{
	int current_col = LogicIndex % RefGrid->CountColum;
	int current_row = (int)LogicIndex / RefGrid->CountColum;

	int count_offset_down = 0;
	int count_falling = 0;

	for(int i = 1; i <= current_row; i++)
	{
		int current_bottom_row = current_row - i;
		ATile* tile_bottom = RefGrid->GameTiles[(RefGrid->CountColum * current_bottom_row) + current_col];
		if(tile_bottom)
		{
			UE_LOG(LogTemp, Warning, TEXT("C++ ATile::StartFalling() iteration LogicIndex=%d current_col=%d current_bottom_row=%d [current_col*current_bottom_row]=%d tile_bottom->StateTile=%d"), LogicIndex, current_col, current_bottom_row, ((RefGrid->CountColum * current_bottom_row) + current_col), tile_bottom->StateTile);
			if(tile_bottom->StateTile == TypeStateTile::Fall)
			{
				count_falling++;
			}
			if(tile_bottom->StateTile == TypeStateTile::Normal)
			{
				break;
			}
		}
		count_offset_down++;
	}

	//Отнимаем кол-во падающих снизу
	count_offset_down -= count_falling;

	//Задаем новое положение в GameTiles
	NewLogicIndex = (RefGrid->CountColum * (current_row - count_offset_down)) + current_col;

	FVector location = GetActorLocation();
	FinishFallingLocation.X = location.X;
	FinishFallingLocation.Y = location.Y;
	FinishFallingLocation.Z = location.Z - RefGrid->TileHeight * count_offset_down;	

	UE_LOG(LogTemp, Warning, TEXT("C++ ATile::StartFalling() LogicIndex=%d NewLogicIndex=%d count_offset_down=%d old_location=%s new_location=%s"), LogicIndex, NewLogicIndex, count_offset_down, *location.ToString(), *FinishFallingLocation.ToString());

	SetActorTickEnabled(true);
}
