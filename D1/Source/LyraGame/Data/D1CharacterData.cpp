// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/D1CharacterData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(D1CharacterData)

const UD1CharacterData& UD1CharacterData::Get()//접근용
{
	return ULyraAssetManager::Get().GetCharacterData();
}

#if WITH_EDITOR
void UD1CharacterData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	DefaultArmorMeshMap.KeySort([](const ECharacterSkinType A, const ECharacterSkinType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

const FD1DefaultArmorMeshSet& UD1CharacterData::GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const
{
	if (DefaultArmorMeshMap.Contains(CharacterSkinType) == false)
	{
		static const FD1DefaultArmorMeshSet EmptyEntry;//깡통 반환
		return EmptyEntry;
	}

	return DefaultArmorMeshMap.FindChecked(CharacterSkinType);
}
