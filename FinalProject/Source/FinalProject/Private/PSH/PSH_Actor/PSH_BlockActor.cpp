// Fill out your copyright notice in the Description page of Project Settings.


#include "PSH/PSH_Actor/PSH_BlockActor.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetArrayLibrary.h"

// Sets default values
APSH_BlockActor::APSH_BlockActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(meshComp);
}

// Called when the game starts or when spawned
void APSH_BlockActor::BeginPlay()
{
	Super::BeginPlay();
	
	meshComp->OnComponentSleep.AddDynamic(this, &APSH_BlockActor::OnComponentSleep);
}

// Called every frame
void APSH_BlockActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

// 	if (bGrab) // true �ϋ�
// 	{
// 		FVector worldLoc; // ���콺�� ���� ������
// 		FVector worldDir; // ī�޶� �����ǰ� ���콺 Ŭ�� ��Ұ��� ����
// 
// 		APlayerController* pc = Cast<APlayerController>(GetOwner());
// 		UE_LOG(LogTemp, Warning, TEXT("%s"), *GetOwner()->GetName());
// 
// 		if (pc == nullptr) return;
// 		pc->DeprojectMousePositionToWorld(worldLoc, worldDir);
// 
// 		/*UE_LOG(LogTemp, Warning, TEXT("X : %f Y : %f Z : %f"), worldLoc.X, worldLoc.Y, worldLoc.Z);*/
// 		FVector TargetLocation = worldLoc + (worldDir * 200.0f); // �ణ �������� �̵�
// 
// 		playerHandle->SetTargetLocation(TargetLocation);
// 	}
}
void APSH_BlockActor::PickUp(class UPhysicsHandleComponent* handle)
{
	if (handle == nullptr) return;
	
	// �θ���� ���� ����
	Remove();

	// ���� ���
	handle->GrabComponentAtLocationWithRotation(meshComp, NAME_None, GetActorLocation(), GetActorRotation());

	pickedUp = true;

	meshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	meshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);

	for (auto* actor : childsActors)
	{
		Cast<APSH_BlockActor>(actor)->ChildCollisionUpdate(ECollisionEnabled::QueryOnly);
	}
}

void APSH_BlockActor::Drop(class UPhysicsHandleComponent* physicshandle)
{
	if (physicshandle != nullptr)
	{
		physicshandle->ReleaseComponent();
	}
	
	pickedUp = false;

	meshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	meshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	meshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	for (auto* actor : childsActors)
	{
		Cast<APSH_BlockActor>(actor)->ChildCollisionUpdate(ECollisionEnabled::QueryAndPhysics);
	}
}

void APSH_BlockActor::Place(class APSH_BlockActor* attachActor, FTransform worldTransform)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("Pickup Dev")));

	attachActor->AddChild(this);
	FAttachmentTransformRules rule = FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	this->AttachToActor(attachActor, rule);

	meshComp->SetSimulatePhysics(false);
	parent = attachActor;

	SetActorRelativeLocation(worldTransform.GetLocation());
	SetActorRotation(worldTransform.GetRotation());
	attachActor->TransferChildren(childsActors);

	meshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	meshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	meshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	for (auto* actor : childsActors)
	{
		Cast<APSH_BlockActor>(actor)->ChildCollisionUpdate(ECollisionEnabled::QueryAndPhysics);
	}
}

void APSH_BlockActor::Remove()
{

	if(parent == nullptr) return;
	
	// �θ𿡼� �и�
	FDetachmentTransformRules rule = FDetachmentTransformRules::KeepWorldTransform;
	DetachFromActor(rule);

	// �ù����̼� Ȱ��ȭ
	meshComp->SetSimulatePhysics(true);

	// �θ𿡼� �ڽ� ����
	parent->RemoveChild(this);
	parent->RemoveChildren(childsActors);
	
	parent = nullptr;
}

void APSH_BlockActor::ChildCollisionUpdate(ECollisionEnabled::Type NewType) // �ڽ� �ݸ��� ������Ʈ
{
	meshComp->SetCollisionEnabled(NewType);

	ECollisionResponse newResponse = ECR_Ignore;

	switch (NewType)
	{
	case ECollisionEnabled::NoCollision:
		newResponse = ECR_Overlap;
		pickedUp = true;
		break;
	case ECollisionEnabled::QueryOnly:
		newResponse = ECR_Overlap;
		pickedUp = true;
		break;
	case ECollisionEnabled::PhysicsOnly:
		newResponse = ECR_Ignore;
		pickedUp = false;
		break;
	case ECollisionEnabled::QueryAndPhysics:
		newResponse = ECR_Block;
		pickedUp = false;
		break;
	case ECollisionEnabled::ProbeOnly:
		newResponse = ECR_Ignore;
		pickedUp = false;
		break;
	case ECollisionEnabled::QueryAndProbe:
		newResponse = ECR_Ignore;
		pickedUp = false;
		break;
	}

	meshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, newResponse);
	meshComp->SetCollisionResponseToChannel(ECC_WorldStatic, newResponse);


	for (auto* actor : childsActors)
	{
		Cast<APSH_BlockActor>(actor)->ChildCollisionUpdate(NewType);
	}
}

TArray<FVector> APSH_BlockActor::GetSnapPoints()
{
	return snapPoints;
}
TArray<FRotator> APSH_BlockActor::GetSnapDirections()
{
	return snapDirections;
}
TArray<int32> APSH_BlockActor::GetsnapPritority()
{
	return snapPritority;
}
void APSH_BlockActor::AddSnapPoint(FVector location, FRotator rotation, int32 priority)
{
	snapPoints.Add(location);
	snapDirections.Add(rotation);
	snapPritority.Add(priority);

	SnapApplyPriority();
}

void APSH_BlockActor::SnapApplyPriority()
{
	// Loop Through All Snap Points

	// �ּ� 2���� �켱������ �ִ��� Ȯ��
	if (snapPritority.Num() < 2) return;

	// �ӽ� �迭 ũ�⸦ �̸� ����
	TArray<FVector> tempSnapPoints;
	tempSnapPoints.SetNum(snapPoints.Num());

	TArray<FRotator> tempSnapDirections;
	tempSnapDirections.SetNum(snapDirections.Num());

	TArray<int32> tempSnapPritority;
	tempSnapPritority.SetNum(snapPritority.Num());

	for (int i = 0; i < snapPritority.Num(); i++)
	{
		int32 minIndex = 0;
		int32 pilYoeObSeum = 0;

		// �ּ� �켱���� ã��
		UKismetMathLibrary::MinOfIntArray(snapPritority, minIndex, pilYoeObSeum);

		// �迭 ũ�⸦ �������� ���� (Size to Fit ��� ����)
		if (tempSnapPoints.Num() <= i)
		{
			tempSnapPoints.SetNum(i + 1);  // �迭 ũ�� ���߱�
		}
		if (tempSnapDirections.Num() <= i)
		{
			tempSnapDirections.SetNum(i + 1);  // �迭 ũ�� ���߱�
		}
		if (tempSnapPritority.Num() <= i)
		{
			tempSnapPritority.SetNum(i + 1);  // �迭 ũ�� ���߱�
		}

		// �ּҰ��� �ش��ϴ� ���� ����Ʈ �� ������ �ӽ� �迭�� ����
		if (snapPoints.IsValidIndex(minIndex))
		{
			tempSnapPoints[i] = snapPoints[minIndex];
		}
		if (snapDirections.IsValidIndex(minIndex))
		{
			tempSnapDirections[i] = snapDirections[minIndex];
		}
		if (snapPritority.IsValidIndex(minIndex))
		{
			tempSnapPritority[i] = snapPritority[minIndex];
		}

		// �ּҰ��� �ִ밪���� �����Ͽ� ���� �ݺ����� ����
		if (snapPritority.IsValidIndex(minIndex))
		{
			snapPritority[minIndex] = 2147483647;  // int32 �ִ밪
		}
	}

	// �迭 ���Ҵ�
	snapPoints = tempSnapPoints;
	snapDirections = tempSnapDirections;
	snapPritority = tempSnapPritority;
}

bool APSH_BlockActor::OvelapChek()
{
	// ��ġ�� ���͵��� ������ �迭 ����
	TArray<AActor*> OutOverlappingActors;

	// ���͸��� Ŭ������ Ÿ���� ���� (���⼭�� �ƹ��͵� �������� ����)
	TSubclassOf<AActor> ClassFilter = APSH_BlockActor::StaticClass();

	// ���� �޽� ������Ʈ�� ��ġ�� ��� ���͸� ã��
	meshComp->GetOverlappingActors(OutOverlappingActors, ClassFilter);

	// ��ġ�� ���Ͱ� ������ ��ȿ�� ��ġ�� ����
	bool validPlacement = OutOverlappingActors.Num() == 0;

	// �ڽ� ���͵鿡 ���� ��������� ��ħ �˻�
	for (auto* actor : childsActors)
	{
		
	}


	return true; // ���� ��� ��ȯ
}


void APSH_BlockActor::AddChild(class APSH_BlockActor* childActor)
{
	//�ڽſ��� �ڽ��� �߰�
	childsActors.Add(childActor);

	// �ڽĿ��� �θ� �Ҵ�
	childActor->parent = this;
}


void APSH_BlockActor::TransferChildren(TArray<AActor*> childActor)
{
	for (auto* actor : childActor)
	{
		if (Cast<APSH_BlockActor>(actor))
		{
			this->AddChild(Cast<APSH_BlockActor>(actor));
		}
	}
}

void APSH_BlockActor::RemoveChild(class APSH_BlockActor* actor)
{
	
	//�ڽ� ��Ͽ��� ����
	childsActors.Remove(actor);
	
	// �θ� ���� ����
	actor->parent = nullptr;
}

void APSH_BlockActor::RemoveChildren(TArray<AActor*> childActor)
{
	for (auto* actor : childActor)
	{
		if (Cast<APSH_BlockActor>(actor))
		{
			this->RemoveChild(Cast<APSH_BlockActor>(actor));
		}
// 		childActor.Remove(Cast<APSH_BlockActor>(actor));
// 		Cast<APSH_BlockActor>(actor)->parent = nullptr;
	}
}
void APSH_BlockActor::OnComponentSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
		// ���� ������Ʈ�� ����� (Wake Rigid Body)
		SleepingComponent->WakeRigidBody(BoneName);

}