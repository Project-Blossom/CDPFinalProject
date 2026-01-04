// Fill out your copyright notice in the Description page of Project Settings.


#include "LensDistortionComponent.h"

// Sets default values
ALensDistortionComponent::ALensDistortionComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALensDistortionComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALensDistortionComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

