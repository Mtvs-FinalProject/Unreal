// Fill out your copyright notice in the Description page of Project Settings.


#include "PSH/PSH_UI/PSH_ObjectWidget.h"
#include "PSH/PSH_Actor/PSH_BlockActor.h"
#include "PSH/PSH_DataTable/PSH_ObjectDataTable.h"
#include "Engine/DataTable.h"
#include "PSH/PSH_UI/PSH_BlockWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "PSH/PSH_Player/PSH_Player.h"
#include "PSH/PSH_Player/PSH_PlayerController.h"
#include "PSH/PSH_Actor/PSH_GarbageBot.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ScrollBoxSlot.h"
#include "Styling/SlateColor.h"
#include "PSH/PSH_Actor/PSH_SpawnBot.h"
#include "../FinalProject.h"

void UPSH_ObjectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btr_Back->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnClickedBack);
	Btr_Save->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnClickedSave);
	Btr_Load->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnClickedLoad);

	// Nomal UI 버튼
	Btr_NormalRight->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnNormalScrollRightClicked);
	Btr_NormalLeft->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnNormalScrollLeftClicked);

	Btr_FunctionRight->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnFunctionScrollRightClicked);
	Btr_FunctionLeft->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnFunctionScrollLeftClicked);

	Btr_MapLeft->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnMapScrollLeftClicked);
	Btr_MapRight->OnClicked.AddDynamic(this, &UPSH_ObjectWidget::OnMapScrollRightClicked);

	AddchildBlock();
}
void UPSH_ObjectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsNomalScrolling)
	{
		float CurrentOffset = Scroll_NomarlBlcok->GetScrollOffset();
		float NewOffset = FMath::FInterpTo(CurrentOffset, nomalScrollOffset, InDeltaTime, 10.0f);  // 보간 속도 10.0f 조절 가능
		Scroll_NomarlBlcok->SetScrollOffset(NewOffset);

		// 목표 오프셋에 거의 도달하면 스크롤 완료
		if (FMath::IsNearlyEqual(NewOffset, nomalScrollOffset, 0.5f))
		{
			bIsNomalScrolling = false;
			Scroll_NomarlBlcok->SetScrollOffset(nomalScrollOffset);  // 정확히 맞춰 줌
		}
	}

	if (bIsFunctionScrolling)
	{
		float CurrentOffset = Scroll_FunctionBlcok->GetScrollOffset();
		float NewOffset = FMath::FInterpTo(CurrentOffset, functionScrollOffset, InDeltaTime, 10.0f);  // 보간 속도 10.0f 조절 가능
		Scroll_FunctionBlcok->SetScrollOffset(NewOffset);

		// 목표 오프셋에 거의 도달하면 스크롤 완료
		if (FMath::IsNearlyEqual(NewOffset, functionScrollOffset, 0.5f))
		{
			bIsFunctionScrolling = false;
			Scroll_FunctionBlcok->SetScrollOffset(functionScrollOffset);  // 정확히 맞춰 줌
		}
	}

	if (bIsmapScrolling)
	{
		float CurrentOffset = Scroll_MapBlcok->GetScrollOffset();
		float NewOffset = FMath::FInterpTo(CurrentOffset, mapScrollOffset, InDeltaTime, 10.0f);  // 보간 속도 10.0f 조절 가능
		Scroll_MapBlcok->SetScrollOffset(NewOffset);

		// 목표 오프셋에 거의 도달하면 스크롤 완료
		if (FMath::IsNearlyEqual(NewOffset, mapScrollOffset, 0.5f))
		{
			bIsmapScrolling = false;
			Scroll_MapBlcok->SetScrollOffset(mapScrollOffset);  // 정확히 맞춰 줌
		}
	}
}
void UPSH_ObjectWidget::AddchildBlock() // 데이터 파싱해서 블럭 조정
{
	AddNomalBlock();
	AddFunctionBlock();
	AddMapBlock();
}
void UPSH_ObjectWidget::AddNomalBlock()
{
	TArray<FPSH_UiBlockData*> dataAraay;
	nomalObjectData->GetAllRows<FPSH_UiBlockData>(TEXT("non"), dataAraay);

	ScrollBoxHeight = Scroll_NomarlBlcok->GetDesiredSize().Y;

	for (int i = 0; i < dataAraay.Num(); i++)
	{
		auto* blockWidget = CreateWidget<UPSH_BlockWidget>(GetWorld(), blockWidgetFac);
	
		blockWidget->SetDesiredSizeInViewport(FVector2D(ScrollBoxHeight, ScrollBoxHeight));

		FButtonStyle btrStyle;
		FSlateBrush normal , hover , pressed;

		normal.SetResourceObject(dataAraay[i]->icon);
		normal.ImageSize = FVector2D(300, 300);
		normal.TintColor = FSlateColor(FLinearColor::White);

		hover.SetResourceObject(dataAraay[i]->icon);
		hover.ImageSize = FVector2D(300, 300);
		hover.TintColor = FSlateColor(FLinearColor(0.8f,0.8f,0.8f,1.0f));
		hover.DrawAs = ESlateBrushDrawType::RoundedBox;
		hover.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		hover.OutlineSettings.Width = 5;
		hover.OutlineSettings.Color = FSlateColor(FLinearColor::Red);

		pressed.SetResourceObject(dataAraay[i]->icon);
		pressed.ImageSize = FVector2D(300, 300);
		pressed.TintColor = FSlateColor(FLinearColor(0.4f,0.4f,0.4f,1.0f));

		btrStyle.SetNormal(normal);
		btrStyle.SetHovered(hover);
		btrStyle.SetPressed(pressed);

		blockWidget->Btn_Icon->SetStyle(btrStyle);
		blockWidget->spawnActor = dataAraay[i]->actor;
		/*blockWidget->spawnMesh = dataAraay[i]->mesh;*/
		
		Scroll_NomarlBlcok->AddChild(blockWidget);

		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(blockWidget->Slot))
		{
			ScrollSlot->SetPadding(FMargin(5, 0, 5, 0));
		}
	}
}
void UPSH_ObjectWidget::AddFunctionBlock()
{
	TArray<FPSH_UiBlockData*> dataAraay;
	functionObjectData->GetAllRows<FPSH_UiBlockData>(TEXT("non"), dataAraay);

	ScrollBoxHeight = Scroll_NomarlBlcok->GetDesiredSize().Y;

	for (int i = 0; i < dataAraay.Num(); i++)
	{
		auto* blockWidget = CreateWidget<UPSH_BlockWidget>(GetWorld(), blockWidgetFac);

		blockWidget->SetDesiredSizeInViewport(FVector2D(ScrollBoxHeight, ScrollBoxHeight));

		FButtonStyle btrStyle;
		FSlateBrush normal, hover, pressed;

		normal.SetResourceObject(dataAraay[i]->icon);
		normal.ImageSize = FVector2D(300, 300);
		normal.TintColor = FSlateColor(FLinearColor::White);
		

		hover.SetResourceObject(dataAraay[i]->icon);
		hover.ImageSize = FVector2D(300, 300);
		hover.TintColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
		hover.DrawAs = ESlateBrushDrawType::RoundedBox;
		hover.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		hover.OutlineSettings.Width = 5;
		hover.OutlineSettings.Color = FSlateColor(FLinearColor::Red);

		pressed.SetResourceObject(dataAraay[i]->icon);
		pressed.ImageSize = FVector2D(300, 300);
		pressed.TintColor = FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));

		btrStyle.SetNormal(normal);
		btrStyle.SetHovered(hover);
		btrStyle.SetPressed(pressed);

		blockWidget->Btn_Icon->SetStyle(btrStyle);
		blockWidget->spawnActor = dataAraay[i]->actor;
		/*blockWidget->spawnMesh = dataAraay[i]->mesh;*/

		Scroll_FunctionBlcok->AddChild(blockWidget);

		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(blockWidget->Slot))
		{
			ScrollSlot->SetPadding(FMargin(5, 0, 5, 0));
		}

	}
}
void UPSH_ObjectWidget::AddMapBlock()
{
	TArray<FPSH_UiBlockData*> dataAraay;
	mapObjectData->GetAllRows<FPSH_UiBlockData>(TEXT("non"), dataAraay);

	ScrollBoxHeight = Scroll_NomarlBlcok->GetDesiredSize().Y;

	for (int i = 0; i < dataAraay.Num(); i++)
	{
		auto* blockWidget = CreateWidget<UPSH_BlockWidget>(GetWorld(), blockWidgetFac);

		blockWidget->SetDesiredSizeInViewport(FVector2D(ScrollBoxHeight, ScrollBoxHeight));

		FButtonStyle btrStyle;
		FSlateBrush normal, hover, pressed;

		normal.SetResourceObject(dataAraay[i]->icon);
		normal.ImageSize = FVector2D(300, 300);
		normal.TintColor = FSlateColor(FLinearColor::White);

		hover.SetResourceObject(dataAraay[i]->icon);
		hover.ImageSize = FVector2D(300, 300);
		hover.TintColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
		hover.DrawAs = ESlateBrushDrawType::RoundedBox;
		hover.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		hover.OutlineSettings.Width = 5;
		hover.OutlineSettings.Color = FSlateColor(FLinearColor::Red);

		pressed.SetResourceObject(dataAraay[i]->icon);
		pressed.ImageSize = FVector2D(300, 300);
		pressed.TintColor = FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));

		btrStyle.SetNormal(normal);
		btrStyle.SetHovered(hover);
		btrStyle.SetPressed(pressed);

		blockWidget->Btn_Icon->SetStyle(btrStyle);
		blockWidget->spawnActor = dataAraay[i]->actor;
		/*blockWidget->spawnMesh = dataAraay[i]->mesh;*/

		Scroll_MapBlcok->AddChild(blockWidget);

		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(blockWidget->Slot))
		{
			ScrollSlot->SetPadding(FMargin(5, 0, 5, 0));
		}
	}
}
void UPSH_ObjectWidget::OnClickedBack()
{
	APSH_Player* player = Cast<APSH_Player>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (player == nullptr ) return;
	player->ToggleARmLength();
	player->SRPC_SpawnbotIdel();
	SetVisibility(ESlateVisibility::Hidden);
}
void UPSH_ObjectWidget::OnClickedSave()
{
	APSH_Player* player = Cast<APSH_Player>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (player == nullptr || player->pc == nullptr) return;

	player->pc->ObjectSave();
	
}
void UPSH_ObjectWidget::OnClickedLoad()
{
	APSH_Player* player = Cast<APSH_Player>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (player == nullptr) return;

	player->pc->ObjectLoad();

}

void UPSH_ObjectWidget::OnNormalScrollRightClicked()
{
	const int32 NumChildren = Scroll_NomarlBlcok->GetChildrenCount();
	if (NumChildren == 0) return;

	float WidgetHeight = ScrollBoxHeight > 0 ? ScrollBoxHeight : Scroll_NomarlBlcok->GetDesiredSize().Y + 5;
	CurrentIndex = (CurrentIndex + 1) % NumChildren;
	nomalScrollOffset = CurrentIndex * WidgetHeight;
	bIsNomalScrolling = true;  // 스크롤 시작

}
void UPSH_ObjectWidget::OnNormalScrollLeftClicked()
{
	const int32 NumChildren = Scroll_NomarlBlcok->GetChildrenCount();
	if (NumChildren == 0) return;

	float WidgetHeight = ScrollBoxHeight > 0 ? ScrollBoxHeight : Scroll_NomarlBlcok->GetDesiredSize().Y + 5;
	CurrentIndex = (CurrentIndex - 1 + NumChildren) % NumChildren;
	nomalScrollOffset = CurrentIndex * WidgetHeight;
	bIsNomalScrolling = true;  // 스크롤 시작
}
void UPSH_ObjectWidget::OnFunctionScrollLeftClicked()
{
	const int32 NumChildren = Scroll_FunctionBlcok->GetChildrenCount();
	if (NumChildren == 0) return;

	UE_LOG(LogTemp,Warning,TEXT("Left"));

	float WidgetHeight = ScrollBoxHeight > 0 ? ScrollBoxHeight : Scroll_FunctionBlcok->GetDesiredSize().Y;
	CurrentIndex = (CurrentIndex - 1 + NumChildren) % NumChildren;
	functionScrollOffset = CurrentIndex * WidgetHeight;
	bIsFunctionScrolling = true;  // 스크롤 시작
}
void UPSH_ObjectWidget::OnFunctionScrollRightClicked()
{
	const int32 NumChildren = Scroll_FunctionBlcok->GetChildrenCount();
	if (NumChildren == 0) return;

	UE_LOG(LogTemp, Warning, TEXT("Right"));

	float WidgetHeight = ScrollBoxHeight > 0 ? ScrollBoxHeight : Scroll_FunctionBlcok->GetDesiredSize().Y;
	CurrentIndex = (CurrentIndex + 1) % NumChildren;
	functionScrollOffset = CurrentIndex * WidgetHeight;
	bIsFunctionScrolling = true;  // 스크롤 시작
}

void UPSH_ObjectWidget::OnMapScrollRightClicked()
{
	const int32 NumChildren = Scroll_MapBlcok->GetChildrenCount();
	if (NumChildren == 0) return;

	UE_LOG(LogTemp, Warning, TEXT("Left"));

	float WidgetHeight = ScrollBoxHeight > 0 ? ScrollBoxHeight : Scroll_MapBlcok->GetDesiredSize().Y;
	CurrentIndex = (CurrentIndex - 1 + NumChildren) % NumChildren;
	mapScrollOffset = CurrentIndex * WidgetHeight;
	bIsmapScrolling = true;  // 스크롤 시작
}

void UPSH_ObjectWidget::OnMapScrollLeftClicked()
{
	const int32 NumChildren = Scroll_MapBlcok->GetChildrenCount();
	if (NumChildren == 0) return;

	UE_LOG(LogTemp, Warning, TEXT("Right"));

	float WidgetHeight = ScrollBoxHeight > 0 ? ScrollBoxHeight : Scroll_MapBlcok->GetDesiredSize().Y;
	CurrentIndex = (CurrentIndex + 1) % NumChildren;
	mapScrollOffset = CurrentIndex * WidgetHeight;
	bIsmapScrolling = true;  // 스크롤 시작
}