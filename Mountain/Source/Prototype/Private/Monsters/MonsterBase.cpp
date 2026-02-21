#include "Monsters/MonsterBase.h"
#include "DownfallCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

DEFINE_LOG_CATEGORY(LogMonster);

AMonsterBase::AMonsterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // AI Perception Setup
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    
    // Hearing Config (기본값 - 모든 몬스터가 사용)
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1000.0f;
    HearingConfig->SetMaxAge(0.2f);  // 즉시 반응
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(HearingConfig->GetSenseImplementation());
    
    // Sight Config (Flying 몬스터용 - 자식 클래스에서 선택적으로 추가 가능)
    // WallCrawler는 Hearing만 사용하므로 여기서는 설정 안 함
    
    UE_LOG(LogMonster, Warning, TEXT("MonsterBase constructor - Perception configured (Hearing)"));
}

void AMonsterBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    // Perception Delegate Binding
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterBase::OnPerceptionUpdated);
        
        // CRITICAL: Perception 활성화
        PerceptionComponent->Activate(true);
        
        UE_LOG(LogMonster, Warning, TEXT("%s Perception activated! SightRadius: %.1f"), *GetName(), SightRadius);
    }
    else
    {
        UE_LOG(LogMonster, Error, TEXT("%s PerceptionComponent is NULL!"), *GetName());
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
    UE_LOG(LogMonster, Warning, TEXT("%s OnPerceptionUpdated! Actor: %s, Successfully Sensed: %s"), 
        *GetName(), 
        Actor ? *Actor->GetName() : TEXT("NULL"),
        Stimulus.WasSuccessfullySensed() ? TEXT("YES") : TEXT("NO"));

    if (!Actor) return;

    ADownfallCharacter* Player = Cast<ADownfallCharacter>(Actor);
    if (!Player)
    {
        UE_LOG(LogMonster, Warning, TEXT("%s Actor is not DownfallCharacter: %s"), *GetName(), *Actor->GetClass()->GetName());
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // 플레이어 인식
        TargetPlayer = Player;
        UE_LOG(LogMonster, Warning, TEXT("%s DETECTED player at distance: %.1f"), 
            *GetName(), FVector::Dist(GetActorLocation(), Player->GetActorLocation()));
    }
    else
    {
        // 플레이어 놓침
        TargetPlayer = nullptr;
        UE_LOG(LogMonster, Warning, TEXT("%s LOST player"), *GetName());
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