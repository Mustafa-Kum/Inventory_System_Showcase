// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/WeaponDataAsset.h"

FPrimaryAssetId UWeaponDataAsset::GetPrimaryAssetId() const
{
	// This tells the Unreal Asset Manager how to categorize and find this asset. "Weapon" is the category type.
	return FPrimaryAssetId(FName("Weapon"), GetFName());
}
