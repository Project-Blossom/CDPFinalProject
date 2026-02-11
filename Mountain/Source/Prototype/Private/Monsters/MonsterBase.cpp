#include "Monsters/MonsterBase.h"
#include "DownfallCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

DEFINE_LOG_CATEGORY(LogMonster);

AMonsterBase::AMonsterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // AI Perception Setup
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    
    // Sight Config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = SightAngle;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AMonsterBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    // Perception Delegate Binding
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterBase::OnPerceptionUpdated);
    }

    UE_LOG(LogMonster, Log, TEXT("%s spawned with %.1f health"), *GetName(), CurrentHealth);
}

void AMonsterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Debug: 플레이어 감지 상태 표시
#if !UE_BUILD_SHIPPING
    if (TargetPlayer)
    {
        DrawDebugLine(
            GetWorld(),
            GetActorLocation(),
            TargetPlayer->GetActorLocation(),
            FColor::Red,
            false,
            0.1f,
            0,
            2.0f
        );
    }
#endif
}

void AMonsterBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    ADownfallCharacter* Player = Cast<ADownfallCharacter>(Actor);
    if (!Player) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // 플레이어 인식
        TargetPlayer = Player;
        UE_LOG(LogMonster, Log, TEXT("%s detected player at distance: %.1f"), 
            *GetName(), FVector::Dist(GetActorLocation(), Player->GetActorLocation()));
    }
    else
    {
        // 플레이어 놓침
        TargetPlayer = nullptr;
        UE_LOG(LogMonster, Log, TEXT("%s lost player"), *GetName());
    }
}

void AMonsterBase::TakeDamageCustom(float Damage)
{
    CurrentHealth -= Damage;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth);
    
    UE_LOG(LogMonster, Log, TEXT("%s took %.1f damage, remaining HP: %.1f"), 
        *GetName(), Damage, CurrentHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

void AMonsterBase::Die()
{
    UE_LOG(LogMonster, Warning, TEXT("%s died"), *GetName());
    
    // TODO: Death animation, effects, rewards
    
    // 일단 파괴
    Destroy();
}