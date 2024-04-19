class Actor;


UCLASS()
class APizduk : AActor
{


	UFUNCTION(BlueprintCallable)
	FVector GetVelocity();
};

void foo()
{
	Actor* actor;
	FVector velocity = actor->GetVelocity();
	float speed = velocity.Length();
}