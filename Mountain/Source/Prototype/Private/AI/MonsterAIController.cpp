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

    // Blackboard와 BehaviorTree 시작
    if (BlackboardAsset && BehaviorTree)
    {
        // Blackboard 초기화
        if (BlackboardComp->InitializeBlackboard(*BlackboardAsset))
        {
            UE_LOG(LogTemp, Log, TEXT("Blackboard initialized for %s"), *InPawn->GetName());
            
            // Behavior Tree 실행
            RunBehaviorTree(BehaviorTree);
            UE_LOG(LogTemp, Warning, TEXT("Behavior Tree started for %s"), *InPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize Blackboard for %s"), *InPawn->GetName());
        }
    }
    else
    {
        if (!BlackboardAsset)
            UE_LOG(LogTemp, Error, TEXT("MonsterAIController: BlackboardAsset is NULL for %s"), *InPawn->GetName());
        if (!BehaviorTree)
            UE_LOG(LogTemp, Error, TEXT("MonsterAIController: BehaviorTree is NULL for %s"), *InPawn->GetName());
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
