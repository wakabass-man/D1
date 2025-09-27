#include "D1ArmorBase.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(D1ArmorBase)//컴파일 속도 향상

AD1ArmorBase::AD1ArmorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

    ArmorMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("ArmorMeshComponent");
	ArmorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ArmorMeshComponent);
}

void AD1ArmorBase::InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryArmorMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial)
{
	DefaultArmorMesh = InDefaultArmorMesh;
	SecondaryArmorMesh = (InSecondaryArmorMesh.IsNull() == false ? InSecondaryArmorMesh : InDefaultArmorMesh);
	
	if (InSkinMaterialSlotName.IsNone() == false && InSkinMaterial.IsNull() == false)
	{
		SkinMaterialSlotName = InSkinMaterialSlotName;
		SkinMaterial = InSkinMaterial;
	}

	SetArmorMesh(DefaultArmorMesh);
}

void AD1ArmorBase::SetPrimaryArmorMesh(USkeletalMesh* InPrimaryArmorMesh)
{
	PrimaryArmorMesh = InPrimaryArmorMesh;
	RefreshArmorMesh();
}

void AD1ArmorBase::ShouldUseSecondaryMesh(bool bInShouldUseSecondary)
{
	bShouldUseSecondaryArmor = bInShouldUseSecondary;
	RefreshArmorMesh();
}

void AD1ArmorBase::RefreshArmorMesh()//상태가 바뀔때의 진입점
{
	TSoftObjectPtr<USkeletalMesh> ArmorMesh;
	if (bShouldUseSecondaryArmor)
	{
		ArmorMesh = SecondaryArmorMesh;
	}
	else
	{
		ArmorMesh = PrimaryArmorMesh ? PrimaryArmorMesh : DefaultArmorMesh;
	}

	SetArmorMesh(ArmorMesh);
}

void AD1ArmorBase::SetArmorMesh(TSoftObjectPtr<USkeletalMesh> InArmorMesh)
{
	check(IsInGameThread());

	USkeletalMesh* LoadedArmorMesh = nullptr;
	if (InArmorMesh.IsNull() == false)
	{
		LoadedArmorMesh = ULyraAssetManager::GetAsset<USkeletalMesh>(InArmorMesh);
	}
	
	ArmorMeshComponent->SetSkeletalMesh(LoadedArmorMesh);
	ArmorMeshComponent->EmptyOverrideMaterials();
	//오버라이드된 머터리얼 배열 비움

	if (SkinMaterialSlotName.IsNone() == false && SkinMaterial.IsNull() == false)
	{
		UMaterialInterface* LoadedMaterial = ULyraAssetManager::GetAsset<UMaterialInterface>(SkinMaterial);
		ArmorMeshComponent->SetMaterialByName(SkinMaterialSlotName, LoadedMaterial);
	}
	//오버라이드 할 머터리얼 있으면 배열 오버라이드
}
//메시와 머터리얼 교체의 원자성 확보