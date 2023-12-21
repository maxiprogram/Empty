// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid.h"

// Sets default values
AGrid::AGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AGrid::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrid::InitGrid()
{
	GameTiles.Init(nullptr, CountColum * CountRow);
	for (int colum = 0; colum < CountColum; colum++)
	{
		for (int row = 0; row < CountRow; row++)
		{
			int index = (CountColum * row) + colum;
			ATile* new_tile = CreateTile(colum, row, index);
			GameTiles[new_tile->LogicIndex] = new_tile;
		}
	}
}

ATile* AGrid::CreateTile(int colum, int row, int logic_index)
{
	//FVector spawn_location = GetActorLocation();
	//spawn_location.Y += TileWidth * row;
	//spawn_location.Z += TileHeight * colum;
	//TODO сделать человеческий logic_index

	FVector center = GetActorLocation();
	FVector spawn_location = FVector(0.0f, -(CountColum * 0.5f) * TileWidth + (TileWidth * 0.5f), -(CountRow * 0.5f) * TileHeight + (TileHeight * 0.5f));
	spawn_location.Y += TileWidth * colum;
	spawn_location.Z += TileHeight * row;
	spawn_location += center;
	
	int rand_number = FMath::RandRange(0, TileLibrary.Num()-1);
	UE_LOG(LogTemp, Warning, TEXT("C++ RANDOM NUMBER=%d"), rand_number);

	FRotator spawn_rotation = GetActorRotation();
	FActorSpawnParameters spawn_params;
	spawn_params.Owner = this;
	spawn_params.Instigator = GetInstigator();
	spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATile* new_tile = GetWorld()->SpawnActor<ATile>(TileLibrary[rand_number].tile_class, spawn_location, spawn_rotation, spawn_params);
	new_tile->LogicIndex = logic_index;
	new_tile->TypeTile = rand_number;
	new_tile->SetSprite(TileLibrary[rand_number].sprite);
	return new_tile;
}

void AGrid::OnSelectTile(ATile* tile)
{
	UE_LOG(LogTemp, Warning, TEXT("C++ OnSelectTile"));
	if(SelectedTile)
	{
		UE_LOG(LogTemp, Warning, TEXT("C++ OnSelectTile SelectedTile != null"));
		int count_tile_between = FMath::Abs(FMath::Max(SelectedTile->LogicIndex, tile->LogicIndex) - FMath::Min(SelectedTile->LogicIndex, tile->LogicIndex));
		if((count_tile_between == 1 || count_tile_between == CountColum) && tile->TypeTile != SelectedTile->TypeTile)
		{
			UE_LOG(LogTemp, Warning, TEXT("C++ OnSelectTile Yes Move oldSelect=%d newSelect=%d oldType=%d newType=%d"), SelectedTile->LogicIndex, tile->LogicIndex, SelectedTile->TypeTile, tile->TypeTile);
			int tmp_index_A = tile->LogicIndex;
			int tmp_index_B = SelectedTile->LogicIndex;

			tile->LogicIndex = tmp_index_B;
			SelectedTile->LogicIndex = tmp_index_A;
			GameTiles[tile->LogicIndex] = tile;
			GameTiles[SelectedTile->LogicIndex] = SelectedTile;

			MatchTiles.Empty();
			MatchTiles = FindMatch(tile);
			MatchTiles.Append(FindMatch(SelectedTile));

			if(MatchTiles.Num() > 0)
			{
				FVector tmp_location_a = tile->GetActorLocation();
				FVector tmp_location_b = SelectedTile->GetActorLocation();
				tile->SetActorLocation(tmp_location_b);
				SelectedTile->SetActorLocation(tmp_location_a);

				//TODO Animation Swap
				CountSwapFinished = 0;
				tile->OnSwapMove(SelectedTile, true);
				SelectedTile->OnSwapMove(tile, true);
			} else
			{
				tile->LogicIndex = tmp_index_A;
				SelectedTile->LogicIndex = tmp_index_B;
				GameTiles[tile->LogicIndex] = tile;
				GameTiles[SelectedTile->LogicIndex] = SelectedTile;
			}

			
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("C++ OnSelectTile No Move oldSelect=%d newSelect=%d"), SelectedTile->LogicIndex, tile->LogicIndex);


		}
		SelectedTile = nullptr;

	} else {
		UE_LOG(LogTemp, Warning, TEXT("C++ OnSelectTile SelectedTile == null"));
		SelectedTile = tile;
	}
}

void AGrid::OnSwapDisplayFinished(ATile* tile)
{
	CountSwapFinished++;
	UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::OnSwapDisplayFinished() index=%d CountSwapFinished=%d"), tile->LogicIndex, CountSwapFinished);
	if(CountSwapFinished == 2)
	{
		CountSwapFinished = 0;
		ExecuteMatch();
	}
}

void AGrid::ExecuteMatch()
{
	if(MatchTiles.Num() > 0)
	{
		for(ATile* item : MatchTiles)
		{
			item->StateTile = TypeStateTile::Delete;
			UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::OnSwapDisplayFinished() change StateTile=Delete LogicIndex=%d"), item->LogicIndex);
		}

		FallTiles.Empty();
		FallTiles = FindFall();

		//Уничтожить совпавшие
		for(ATile* item : MatchTiles)
		{
			GameTiles[item->LogicIndex] = nullptr;
			item->Destroy();
		}

		if (FallTiles.Num() == 0)
		{
			RespawnTiles();
		} else
		{
			//Запустить анимацию падения
			for(ATile* item : FallTiles)
			{
				item->StartFalling();
			}
		}
	}
}

void AGrid::OnTileFinishedFalling(ATile* tile, int newLogicIndex)
{
	if(GameTiles[tile->LogicIndex] == tile)
	{
		GameTiles[tile->LogicIndex] = nullptr;
	}

	tile->LogicIndex = newLogicIndex;
	GameTiles[tile->NewLogicIndex] = tile;

	FallTiles.RemoveSingleSwap(tile);
	if (FallTiles.Num() == 0)
	{
		RespawnTiles();
	}

}

void AGrid::RespawnTiles()
{
	UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::RespawnTiles()"));
	TArray<ATile*> NewTiles;
	for(int currentColum = 0; currentColum < CountColum; currentColum++)
	{
		//Количество пустых в столбце
		int count_null = 0;
		for(int currentRow = (CountRow-1); currentRow >= 0; currentRow--)
		{
			//UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::RespawnTiles() iteration currentColum=%d currentRow=%d"), currentColum, currentRow);
			int index = (CountColum * currentRow) + currentColum;
			if(GameTiles[index] == nullptr)
			{
				count_null++;
				ATile* new_tile = CreateTile(currentColum, currentRow, index);
				GameTiles[new_tile->LogicIndex] = new_tile;
				NewTiles.Add(new_tile);
				UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::RespawnTiles() Create ATile index=%d"), index);
			} else
			{
				break;
			}
		}
	}

	//Проверяем совпадения
	MatchTiles.Empty();
	for(ATile* tile : NewTiles)
	{
		TArray<ATile*> tmp_match_tiles = FindMatch(tile);
		for(ATile* tmp_tile: tmp_match_tiles)
		{
			MatchTiles.AddUnique(tmp_tile);
		}
	}

	//ExecuteMatch();
}

TArray<ATile*> AGrid::FindFall()
{
	UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::FindFall()"));
	
	TArray<ATile*> result;

	for(ATile* item : MatchTiles)
	{
		int current_col = item->LogicIndex % CountColum;
		int current_row = (int)item->LogicIndex / CountColum;
		int max_up = FMath::Clamp((CountRow -1) - current_row, 0, CountRow - 1);

		for(int i=1; i<=max_up; i++)
		{
			int index = item->LogicIndex + (i * CountRow);
			ATile* test = GameTiles[index];
			if(test->StateTile != TypeStateTile::Delete && test->StateTile != TypeStateTile::Fall)
			{
				test->StateTile = TypeStateTile::Fall;
				result.Add(test);
				//test->StartFalling();
			}
		}
	}

	return result;
}

TArray<ATile*> AGrid::FindMatch(ATile* tile)
{
	UE_LOG(LogTemp, Warning, TEXT("C++ AGrid::FindMatch()"));

	TArray<ATile*> result;
	TArray<ATile*> tmp_result;

	int current_col = tile->LogicIndex % CountColum;
	int current_row = (int)tile->LogicIndex / CountColum;
	int max_up = FMath::Clamp((CountRow -1) - current_row, 0, CountRow - 1);
	int max_down = FMath::Clamp((CountRow -1) - max_up, 0, CountRow - 1);
	int max_right = FMath::Clamp((CountColum-1) - current_col, 0, CountColum - 1);
	int max_left = FMath::Clamp((CountColum-1) - max_right, 0, CountColum - 1);
	UE_LOG(LogTemp, Warning, TEXT("C++ FindMatch() current_row=%d current_col=%d max_up=%d max_down=%d max_left=%d max_right=%d"), current_row, current_col, max_up, max_down, max_left, max_right);

	//Down
	for(int i=1; i<=max_down; i++)
	{
		//int index = tile->LogicIndex - (i * CountRow);
		int index = tile->LogicIndex - ((current_row - i) * CountRow);
		ATile* test = GameTiles[index];
		if(test->TypeTile == tile->TypeTile)
		{
			tmp_result.Add(test);
		} else {
			break;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num before down=%d"), tile->TypeTile, tmp_result.Num());
	/*if((tmp_result.Num()+1) >= 3)
	{
		result.Append(tmp_result);
	}
	tmp_result.Empty();*/
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num last down=%d"), tile->TypeTile, tmp_result.Num());
	UE_LOG(LogTemp, Warning, TEXT("C++ Num resultMatch=%d"), result.Num());

	//Up
	for(int i=1; i<=max_up; i++)
	{
		//int index = tile->LogicIndex + (i * CountRow);
		int index = tile->LogicIndex + (i * CountColum);
		ATile* test = GameTiles[index];
		if(test->TypeTile == tile->TypeTile)
		{
			tmp_result.Add(test);
		} else {
			break;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num before up=%d"), tile->TypeTile, tmp_result.Num());
	if((tmp_result.Num()+1) >= 3)
	{
		result.Append(tmp_result);
	}
	tmp_result.Empty();
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num last up=%d"), tile->TypeTile, tmp_result.Num());
	UE_LOG(LogTemp, Warning, TEXT("C++ Num resultMatch=%d"), result.Num());

	//Left
	for(int i=1; i<=max_left; i++)
	{
		int index = tile->LogicIndex - i;
		ATile* test = GameTiles[index];
		if(test->TypeTile == tile->TypeTile)
		{
			tmp_result.Add(test);
		} else {
			break;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num before left=%d"), tile->TypeTile, tmp_result.Num());
	/*if((tmp_result.Num()+1) >= 3)
	{
		result.Append(tmp_result);
	}
	tmp_result.Empty();*/
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num last left=%d"), tile->TypeTile, tmp_result.Num());
	UE_LOG(LogTemp, Warning, TEXT("C++ Num resultMatch=%d"), result.Num());

	//Right
	for(int i=1; i<=max_right; i++)
	{
		int index = tile->LogicIndex + i;
		ATile* test = GameTiles[index];
		if(test->TypeTile == tile->TypeTile)
		{
			tmp_result.Add(test);
		} else {
			break;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num before right=%d"), tile->TypeTile, tmp_result.Num());
	if((tmp_result.Num()+1) >= 3)
	{
		result.Append(tmp_result);
	}
	tmp_result.Empty();
	UE_LOG(LogTemp, Warning, TEXT("C++ typeTile=%d Num last right=%d"), tile->TypeTile, tmp_result.Num());
	UE_LOG(LogTemp, Warning, TEXT("C++ Num resultMatch=%d"), result.Num());

	if(result.Num() > 0)
	{
		result.Add(tile);
	}
	UE_LOG(LogTemp, Warning, TEXT("C++ Num final resultMatch=%d"), result.Num());

	return result;
}
