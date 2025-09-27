#include "D1CosmeticManagerComponent.h"

#include "D1Define.h"
#include "Actors/D1ArmorBase.h"
#include "Character/LyraCharacter.h"
#include "Data/D1CharacterData.h"
//#include "Item/Fragments/D1ItemFragment_Equipable_Armor.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(D1CosmeticManagerComponent)

UD1CosmeticManagerComponent::UD1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UD1CosmeticManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeManager();
}

void UD1CosmeticManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HeadSlot)
	{
		HeadSlot->DestroyComponent();
	}
	
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			CosmeticSlot->DestroyComponent();
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void UD1CosmeticManagerComponent::RefreshArmorMesh(EArmorType ArmorType /*, const UD1ItemFragment_Equipable_Armor* ArmorFragment*/)
{
	check(IsInGameThread());

	if (ArmorType == EArmorType::Count)
		return;
	
	InitializeManager();
	
	/*if (ArmorFragment)
	{
		if (ArmorFragment == nullptr || ArmorFragment->ArmorType != ArmorType)
			return;
		
		SetPrimaryArmorMesh(ArmorType, ArmorFragment->ArmorMesh);
		
		if (ArmorType == EArmorType::Chest)
		{
			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(ArmorFragment->bIsFullBody);
				}
			}
		}
		else if (ArmorType == EArmorType::Helmet)
		{
			if (HeadSlot)
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(HeadSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(true);
				}
			}
		}
	}
	else*/
	{
		if (ArmorType == EArmorType::Chest)
		{
			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(false);
				}
			}
		}
		else if (ArmorType == EArmorType::Helmet)
		{
			if (HeadSlot)
			{
				if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(HeadSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(false);
				}
			}
		}

		SetPrimaryArmorMesh(ArmorType, nullptr);
	}
}

void UD1CosmeticManagerComponent::SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr)
{
	check(IsInGameThread());

	if (ArmorType == EArmorType::Count)
		return;
	
	InitializeManager();
	
	if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)ArmorType])
	{
		if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
		{
			if (ArmorMeshPtr.IsNull())
			{
				CosmeticActor->SetPrimaryArmorMesh(nullptr);
			}
			else
			{
				USkeletalMesh* ArmorMesh = ULyraAssetManager::GetAsset<USkeletalMesh>(ArmorMeshPtr);
				CosmeticActor->SetPrimaryArmorMesh(ArmorMesh);
			}
		}
	}
}

void UD1CosmeticManagerComponent::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (HeadSlot)
	{
		if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(HeadSlot->GetChildActor()))
		{
			OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
		}
	}
	
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			if (AD1ArmorBase* CosmeticActor = Cast<AD1ArmorBase>(CosmeticSlot->GetChildActor()))
			{
				OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
			}
		}
	}
}

void UD1CosmeticManagerComponent::InitializeManager()
{
	check(IsInGameThread());

	if (bInitialized)
		return;

	bInitialized = true;

	const int32 ArmorTypeCount = (int32)EArmorType::Count;
	CosmeticSlots.SetNumZeroed(ArmorTypeCount);
	
	check(CosmeticSlotClass);
	check(CharacterSkinType != ECharacterSkinType::Count);
	
	//코스메틱 슬롯 초기화 한 뒤에 서버클라분기
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (Character->IsNetMode(NM_DedicatedServer) == false)
		{
			const auto& CharacterData = UD1CharacterData::Get();
			const FD1DefaultArmorMeshSet& DefaultArmorMeshSet = CharacterData.GetDefaultArmorMeshSet(CharacterSkinType);
			
			HeadSlot = SpawnCosmeticSlotActor(DefaultArmorMeshSet.HeadDefaultMesh, DefaultArmorMeshSet.HeadSecondaryMesh, NAME_None, nullptr);
			
			for (int32 i = 0; i < (int32)EArmorType::Count; i++)
			{
				EArmorType ArmorType = (EArmorType)i;
				FName SkinMaterialSlotName;
				TSoftObjectPtr<UMaterialInterface> SkinMaterial;
				
				if (ArmorType == EArmorType::Helmet || ArmorType == EArmorType::Chest || ArmorType == EArmorType::Hands)
				{
					SkinMaterialSlotName = FName("UpperBody");
					SkinMaterial = DefaultArmorMeshSet.UpperBodySkinMaterial;
				}
				else if (ArmorType == EArmorType::Legs || ArmorType == EArmorType::Foot)
				{
					SkinMaterialSlotName = FName("LowerBody");
					SkinMaterial = DefaultArmorMeshSet.LowerBodySkinMaterial;
				}
				
				CosmeticSlots[i] = SpawnCosmeticSlotActor(DefaultArmorMeshSet.DefaultMeshEntries[i], DefaultArmorMeshSet.SecondaryMeshEntries[i], SkinMaterialSlotName, SkinMaterial);
			}
		}
	}
}

UChildActorComponent* UD1CosmeticManagerComponent::SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial)
{
	check(IsInGameThread());

	UChildActorComponent* CosmeticComponent = nullptr;
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		USceneComponent* ComponentToAttachTo = Character->GetMesh();
		CosmeticComponent = NewObject<UChildActorComponent>(Character);
		CosmeticComponent->SetChildActorClass(CosmeticSlotClass);
		CosmeticComponent->SetupAttachment(ComponentToAttachTo);
		CosmeticComponent->RegisterComponent();//월드에 등록 및 액터 스폰

		if (AD1ArmorBase* SpawnedActor = Cast<AD1ArmorBase>(CosmeticComponent->GetChildActor()))
		{
			if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
			{
				SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
				//자식이 먼저 틱하는거 방지
				//부모와 자식이 모두 월드에 등록한 직후에 걸어준다.
			}
			
			SpawnedActor->InitializeActor(InDefaultMesh, InSecondaryMesh, InSkinMaterialSlotName, InSkinMaterial);
		}
	}
	
	return CosmeticComponent;
}
