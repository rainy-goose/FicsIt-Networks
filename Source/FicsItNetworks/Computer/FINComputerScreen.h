﻿#pragma once

#include "Computer/FINComputerModule.h"
#include "FicsItNetworks/Graphics/FINScreen.h"
#include "FINComputerScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenWidgetUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenGPUUpdate);

UCLASS()
class AFINComputerScreen : public AFINComputerModule, public IFINScreen {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame)
	UObject* GPU = nullptr;
	
public:
	TSharedPtr<SWidget> Widget;

	/**
	 * This event gets triggered when a new widget got set by the GPU
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FScreenWidgetUpdate OnWidgetUpdate;

	/**
	 * This event gets triggered when a new GPU got bound
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
    FScreenGPUUpdate OnGPUUpdate;

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
    // Begin IFINScreen
    virtual void BindGPU(UObject* gpu) override;
	virtual UObject* GetGPU() const override;
	virtual void SetWidget(TSharedPtr<SWidget> widget) override;
	virtual TSharedPtr<SWidget> GetWidget() const override;
	// End IFINScreen
};