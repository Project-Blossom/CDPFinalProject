#include "AI/MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Monsters/MonsterBase.h"

AMonsterAIController::AMonsterAIController()
{
    // Blackboard Component 생성
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    
    // BehaviorTree Component는 AAIController에 기본 포함되어 있음
    
    UE_LOG(LogTemp, Log, TEXT("MonsterAIController created"));
}

void AMonsterAIController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("MonsterAIController BeginPlay"));
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!InPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterAIController: InPawn is NULL!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("MonsterAIController possessed: %s"), *InPawn->GetName());
    
    // CRITICAL: Blackboard & BehaviorTree status check
    UE_LOG(LogTemp, Warning, TEXT("  BlackboardAsset: %s"), BlackboardAsset ? *BlackboardAsset->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  BehaviorTree: %s"), BehaviorTree ? *BehaviorTree->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  BlackboardComp: %s"), BlackboardComp ? TEXT("Valid") : TEXT("NULL"));

    // Blackboard initialization (independent from BehaviorTree)
    if (BlackboardAsset)
    {
        bool bInitSuccess = BlackboardComp->InitializeBlackboard(*BlackboardAsset);
        UE_LOG(LogTemp, Warning, TEXT("  InitializeBlackboard result: %s"), bInitSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
        
        if (bInitSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Blackboard initialized for %s"), *InPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FAILED: Could not initialize Blackboard for %s"), *InPawn->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: BlackboardAsset is NULL for %s"), *InPawn->GetName());
    }

    // Behavior Tree execution (after Blackboard initialization)
    if (BehaviorTree)
    {
        bool bRunSuccess = RunBehaviorTree(BehaviorTree);
        UE_LOG(LogTemp, Warning, TEXT("  RunBehaviorTree result: %s"), bRunSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
        
        if (bRunSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Behavior Tree started for %s"), *InPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FAILED: Could not start Behavior Tree for %s"), *InPawn->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("INFO: BehaviorTree is NULL for %s (will add later)"), *InPawn->GetName());
    }
}

void AMonsterAIController::OnUnPossess()
{
    Super::OnUnPossess();
    
    UE_LOG(LogTemp, Log, TEXT("MonsterAIController unpossessed"));
}

void AMonsterAIController::StartBehaviorTree()
{
    if (BehaviorTree && BlackboardAsset)
    {
        // Blackboard가 초기화되지 않았으면 초기화
        if (!BlackboardComp->GetBlackboardAsset())
        {
            BlackboardComp->InitializeBlackboard(*BlackboardAsset);
        }
        
        RunBehaviorTree(BehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("Behavior Tree manually started"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start Behavior Tree: Missing assets"));
    }
}

void AMonsterAIController::StopBehaviorTree()
{
    UBrainComponent* BrainComp = GetBrainComponent();
    if (BrainComp)
    {
        BrainComp->StopLogic(TEXT("Manually stopped"));
        UE_LOG(LogTemp, Log, TEXT("Behavior Tree stopped"));
    }
}
