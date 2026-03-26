#include "UI/KemWidgets.h"

#include "Actors/KemClassroomStateActor.h"
#include "Actors/KemWorldActors.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Core/KemRoomSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "WebBrowser.h"

void UKemActionButton::EnsureBinding()
{
    if (!bBound)
    {
        OnClicked.AddDynamic(this, &UKemActionButton::HandleInternalClicked);
        bBound = true;
    }
}

void UKemActionButton::HandleInternalClicked()
{
    OnKemActionClicked.Broadcast(ActionId);
}

void UKemPanelWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (!WidgetTree->RootWidget)
    {
        BuildUI();
    }
}

void UKemPanelWidgetBase::NativeDestruct()
{
    Super::NativeDestruct();
}

void UKemPanelWidgetBase::BuildUI()
{
}

UBorder* UKemPanelWidgetBase::CreatePanelBorder(const FLinearColor& Tint) const
{
    UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
    Border->SetPadding(FMargin(28.0f));
    Border->SetBrushColor(Tint);
    return Border;
}

UVerticalBox* UKemPanelWidgetBase::CreateVerticalBox() const
{
    return WidgetTree->ConstructWidget<UVerticalBox>();
}

UTextBlock* UKemPanelWidgetBase::CreateTextBlock(const FString& Text, int32 Size, const FLinearColor& Color) const
{
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>();
    TextBlock->SetText(FText::FromString(Text));
    TextBlock->SetColorAndOpacity(Color);
    TextBlock->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", Size)));
    return TextBlock;
}

UKemActionButton* UKemPanelWidgetBase::CreateActionButton(const FString& Label, const FString& ActionId) const
{
    UKemActionButton* Button = WidgetTree->ConstructWidget<UKemActionButton>();
    Button->EnsureBinding();
    Button->ActionId = ActionId;
    Button->SetBackgroundColor(FLinearColor(0.88f, 0.88f, 0.91f, 0.95f));
    Button->SetColorAndOpacity(FLinearColor::Black);

    UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
    Text->SetText(FText::FromString(Label));
    Text->SetColorAndOpacity(FLinearColor::Black);
    Text->SetJustification(ETextJustify::Center);
    Text->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 28)));
    Button->SetContent(Text);
    return Button;
}

UEditableTextBox* UKemPanelWidgetBase::CreateTextBox(const FString& Hint) const
{
    UEditableTextBox* TextBox = WidgetTree->ConstructWidget<UEditableTextBox>();
    TextBox->SetHintText(FText::FromString(Hint));
    TextBox->SetMinDesiredWidth(380.0f);
    return TextBox;
}

USlider* UKemPanelWidgetBase::CreateSlider(float InitialValue) const
{
    USlider* Slider = WidgetTree->ConstructWidget<USlider>();
    Slider->SetValue(InitialValue);
    return Slider;
}

void UKemHubMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
    {
        RoomSubsystem->OnHostOperationFinished.RemoveDynamic(this, &UKemHubMenuWidget::HandleHostResult);
        RoomSubsystem->OnJoinOperationFinished.RemoveDynamic(this, &UKemHubMenuWidget::HandleJoinResult);
        RoomSubsystem->OnRoomInfoChanged.RemoveDynamic(this, &UKemHubMenuWidget::HandleRoomInfoChanged);

        RoomSubsystem->OnHostOperationFinished.AddDynamic(this, &UKemHubMenuWidget::HandleHostResult);
        RoomSubsystem->OnJoinOperationFinished.AddDynamic(this, &UKemHubMenuWidget::HandleJoinResult);
        RoomSubsystem->OnRoomInfoChanged.AddDynamic(this, &UKemHubMenuWidget::HandleRoomInfoChanged);
    }
}

void UKemHubMenuWidget::BuildUI()
{
    UBorder* RootBorder = CreatePanelBorder(FLinearColor(0.08f, 0.09f, 0.12f, 0.82f));
    UVerticalBox* RootBox = CreateVerticalBox();
    RootBorder->SetContent(RootBox);
    WidgetTree->RootWidget = RootBorder;

    if (UVerticalBoxSlot* VBoxSlot = RootBox->AddChildToVerticalBox(CreateTextBlock(TEXT("KemSUinVR"), 34)))
    {
        VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }

    if (UVerticalBoxSlot* VBoxSlot = RootBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Create or join an auditorium"), 22, FLinearColor(0.8f, 0.82f, 0.9f, 1.0f))))
    {
        VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
    }

    StatusText = CreateTextBlock(TEXT("Ready."), 18, FLinearColor(0.72f, 0.86f, 1.0f, 1.0f));
    if (UVerticalBoxSlot* VBoxSlot = RootBox->AddChildToVerticalBox(StatusText))
    {
        VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
    }

    ContentBox = CreateVerticalBox();
    RootBox->AddChildToVerticalBox(ContentBox);

    RebuildMenuState();
}

void UKemHubMenuWidget::RebuildMenuState()
{
    if (!ContentBox)
    {
        return;
    }

    ContentBox->ClearChildren();
    JoinCodePreviewText = nullptr;

    auto AddBoxButton = [this](const FString& Label, const FString& ActionId, bool bRootAction, bool bKeyboardAction)
    {
        UKemActionButton* Button = CreateActionButton(Label, ActionId);
        if (bRootAction)
        {
            Button->OnKemActionClicked.AddDynamic(this, &UKemHubMenuWidget::HandleRootAction);
        }
        else if (bKeyboardAction)
        {
            Button->OnKemActionClicked.AddDynamic(this, &UKemHubMenuWidget::HandleKeyboardAction);
        }
        else
        {
            Button->OnKemActionClicked.AddDynamic(this, &UKemHubMenuWidget::HandleAuditoriumAction);
        }

        UVerticalBoxSlot* Slot = ContentBox->AddChildToVerticalBox(Button);
        Slot->SetPadding(FMargin(0.0f, 6.0f));
    };

    switch (MenuState)
    {
    case EKemHubMenuState::Root:
        AddBoxButton(TEXT("Create Auditorium"), TEXT("create"), true, false);
        AddBoxButton(TEXT("Join Auditorium"), TEXT("join"), true, false);
        break;

    case EKemHubMenuState::AuditoriumSelection:
    {
        if (UVerticalBoxSlot* VBoxSlot = ContentBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Choose an auditorium preset"), 22)))
        {
            VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
        }

        if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
        {
            for (const FKemAuditoriumDefinition& Auditorium : RoomSubsystem->GetAuditoriums())
            {
                AddBoxButton(Auditorium.DisplayName.ToString(), Auditorium.AuditoriumId.ToString(), false, false);
            }
        }

        AddBoxButton(TEXT("Back"), TEXT("back"), false, false);
        break;
    }

    case EKemHubMenuState::JoinCodeEntry:
    {
        JoinCodePreviewText = CreateTextBlock(PendingJoinCode.IsEmpty() ? TEXT("------") : PendingJoinCode, 30);
        if (UVerticalBoxSlot* VBoxSlot = ContentBox->AddChildToVerticalBox(JoinCodePreviewText))
        {
            VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
        }

        UUniformGridPanel* KeyboardGrid = WidgetTree->ConstructWidget<UUniformGridPanel>();
        const FString KeyboardChars = TEXT("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        for (int32 Index = 0; Index < KeyboardChars.Len(); ++Index)
        {
            const FString KeyText = KeyboardChars.Mid(Index, 1);
            UKemActionButton* KeyButton = CreateActionButton(KeyText, KeyText);
            KeyButton->OnKemActionClicked.AddDynamic(this, &UKemHubMenuWidget::HandleKeyboardAction);
            KeyboardGrid->AddChildToUniformGrid(KeyButton, Index / 6, Index % 6);
        }
        if (UVerticalBoxSlot* VBoxSlot = ContentBox->AddChildToVerticalBox(KeyboardGrid))
        {
            VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
        }

        AddBoxButton(TEXT("Backspace"), TEXT("BACK"), false, true);
        AddBoxButton(TEXT("Clear"), TEXT("CLEAR"), false, true);
        AddBoxButton(TEXT("Join Room"), TEXT("JOIN"), false, true);
        AddBoxButton(TEXT("Cancel"), TEXT("CANCEL"), false, true);
        break;
    }

    case EKemHubMenuState::RoomHosted:
        if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
        {
            AddBoxButton(FString::Printf(TEXT("Room Code: %s"), *RoomSubsystem->GetCurrentRoomCode()), TEXT("noop"), true, false);
            if (StatusText)
            {
                StatusText->SetText(FText::FromString(TEXT("Host ready. Other users can now join by code.")));
            }
        }
        break;

    case EKemHubMenuState::InRoom:
        if (UVerticalBoxSlot* VBoxSlot = ContentBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Connected to auditorium."), 24)))
        {
            VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
        }
        AddBoxButton(TEXT("Leave Room"), TEXT("leave"), true, false);
        break;
    }
}

UKemRoomSubsystem* UKemHubMenuWidget::GetRoomSubsystem() const
{
    return GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr;
}

void UKemHubMenuWidget::HandleRootAction(const FString& ActionId)
{
    if (ActionId == TEXT("create"))
    {
        MenuState = EKemHubMenuState::AuditoriumSelection;
        RebuildMenuState();
    }
    else if (ActionId == TEXT("join"))
    {
        MenuState = EKemHubMenuState::JoinCodeEntry;
        PendingJoinCode.Reset();
        RebuildMenuState();
    }
    else if (ActionId == TEXT("leave"))
    {
        if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
        {
            RoomSubsystem->LeaveCurrentRoom();
        }
    }
}

void UKemHubMenuWidget::HandleAuditoriumAction(const FString& ActionId)
{
    if (ActionId == TEXT("back"))
    {
        MenuState = EKemHubMenuState::Root;
        RebuildMenuState();
        return;
    }

    if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
    {
        if (StatusText)
        {
            StatusText->SetText(FText::FromString(TEXT("Creating auditorium...")));
        }
        RoomSubsystem->HostAuditorium(FName(*ActionId));
    }
}

void UKemHubMenuWidget::HandleKeyboardAction(const FString& ActionId)
{
    if (ActionId == TEXT("BACK"))
    {
        PendingJoinCode.LeftChopInline(1);
    }
    else if (ActionId == TEXT("CLEAR"))
    {
        PendingJoinCode.Reset();
    }
    else if (ActionId == TEXT("JOIN"))
    {
        if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
        {
            RoomSubsystem->JoinRoomByCode(PendingJoinCode);
        }
        return;
    }
    else if (ActionId == TEXT("CANCEL"))
    {
        MenuState = EKemHubMenuState::Root;
        PendingJoinCode.Reset();
        RebuildMenuState();
        return;
    }
    else if (PendingJoinCode.Len() < 8)
    {
        PendingJoinCode.Append(ActionId);
    }

    if (JoinCodePreviewText)
    {
        JoinCodePreviewText->SetText(FText::FromString(PendingJoinCode.IsEmpty() ? TEXT("------") : PendingJoinCode));
    }
}

void UKemHubMenuWidget::HandleHostResult(bool bSuccess, const FString& Message)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
    }

    if (bSuccess)
    {
        MenuState = EKemHubMenuState::RoomHosted;
        RebuildMenuState();
    }
}

void UKemHubMenuWidget::HandleJoinResult(bool bSuccess, const FString& Message)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
    }

    if (bSuccess)
    {
        MenuState = EKemHubMenuState::InRoom;
        RebuildMenuState();
    }
}

void UKemHubMenuWidget::HandleRoomInfoChanged(const FKemRoomInfo& RoomInfo)
{
    MenuState = RoomInfo.bIsInRoom ? (RoomInfo.bIsHost ? EKemHubMenuState::RoomHosted : EKemHubMenuState::InRoom) : EKemHubMenuState::Root;
    RebuildMenuState();
}

void UKemRoomCodeWidget::BuildUI()
{
    UBorder* RootBorder = CreatePanelBorder(FLinearColor(0.08f, 0.1f, 0.14f, 0.78f));
    UVerticalBox* RootBox = CreateVerticalBox();
    RootBorder->SetContent(RootBox);
    WidgetTree->RootWidget = RootBorder;

    HeaderText = CreateTextBlock(TEXT("Room Code"), 24);
    CodeText = CreateTextBlock(TEXT("------"), 32, FLinearColor(0.98f, 0.96f, 0.78f, 1.0f));
    RootBox->AddChildToVerticalBox(HeaderText);
    RootBox->AddChildToVerticalBox(CodeText);
}

void UKemRoomCodeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (UKemRoomSubsystem* RoomSubsystem = GetRoomSubsystem())
    {
        const FString CurrentCode = RoomSubsystem->GetCurrentRoomCode();
        CodeText->SetText(FText::FromString(CurrentCode.IsEmpty() ? TEXT("------") : CurrentCode));
        HeaderText->SetText(FText::FromString(RoomSubsystem->IsHost() ? TEXT("Active Auditorium Code") : TEXT("Awaiting Host")));
    }
}

UKemRoomSubsystem* UKemRoomCodeWidget::GetRoomSubsystem() const
{
    return GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr;
}

void UKemTeacherControlWidget::BuildUI()
{
    UBorder* RootBorder = CreatePanelBorder(FLinearColor(0.07f, 0.09f, 0.13f, 0.86f));
    UVerticalBox* RootBox = CreateVerticalBox();
    RootBorder->SetContent(RootBox);
    WidgetTree->RootWidget = RootBorder;

    RootBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Teacher Console"), 28));
    RootBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Spawn classroom objects and update lecture URL."), 18, FLinearColor(0.8f, 0.82f, 0.9f, 1.0f)));

    const TArray<TPair<FString, FString>> Actions = {
        {TEXT("Spawn Cube"), TEXT("spawn_cube")},
        {TEXT("Spawn Cylinder"), TEXT("spawn_cylinder")},
        {TEXT("Spawn Sphere"), TEXT("spawn_sphere")},
        {TEXT("Clear Spawned Models"), TEXT("clear_models")},
        {TEXT("Set Lecture URL: KemSU"), TEXT("url_kemsu")},
        {TEXT("Set Lecture URL: Wikipedia"), TEXT("url_wiki")},
        {TEXT("Set Lecture URL: OpenAI"), TEXT("url_openai")}
    };

    for (const TPair<FString, FString>& Action : Actions)
    {
        UKemActionButton* Button = CreateActionButton(Action.Key, Action.Value);
        Button->OnKemActionClicked.AddDynamic(this, &UKemTeacherControlWidget::HandleAction);
        RootBox->AddChildToVerticalBox(Button);
    }
}

AKemClassroomStateActor* UKemTeacherControlWidget::GetRoomState() const
{
    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        return RoomSubsystem->GetCurrentClassroomState();
    }
    return nullptr;
}

void UKemTeacherControlWidget::HandleAction(const FString& ActionId)
{
    AKemClassroomStateActor* RoomState = GetRoomState();
    if (!RoomState)
    {
        return;
    }

    if (ActionId.StartsWith(TEXT("spawn_")))
    {
        APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        const APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
        const FVector Origin = Pawn ? Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 350.0f + FVector(0.0f, 0.0f, 50.0f) : FVector(0.0f, 0.0f, 100.0f);
        const FTransform SpawnTransform(FRotator::ZeroRotator, Origin, FVector(0.75f));

        EKemPresentationPrimitive PrimitiveType = EKemPresentationPrimitive::Cube;
        if (ActionId == TEXT("spawn_cylinder"))
        {
            PrimitiveType = EKemPresentationPrimitive::Cylinder;
        }
        else if (ActionId == TEXT("spawn_sphere"))
        {
            PrimitiveType = EKemPresentationPrimitive::Sphere;
        }

        RoomState->SpawnPresentationPrimitive(PrimitiveType, SpawnTransform);
        return;
    }

    if (ActionId == TEXT("clear_models"))
    {
        RoomState->ClearSpawnedPresentationActors();
        return;
    }

    if (ActionId == TEXT("url_kemsu"))
    {
        RoomState->SetLectureUrl(TEXT("https://kemsu.ru"));
    }
    else if (ActionId == TEXT("url_wiki"))
    {
        RoomState->SetLectureUrl(TEXT("https://www.wikipedia.org"));
    }
    else if (ActionId == TEXT("url_openai"))
    {
        RoomState->SetLectureUrl(TEXT("https://openai.com"));
    }
}

void UKemBrowserDeskWidget::BuildUI()
{
    UBorder* RootBorder = CreatePanelBorder(FLinearColor(0.07f, 0.09f, 0.13f, 0.88f));
    UVerticalBox* RootBox = CreateVerticalBox();
    RootBorder->SetContent(RootBox);
    WidgetTree->RootWidget = RootBorder;

    RootBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Lecture Browser Desk"), 26));

    UrlTextBox = CreateTextBox(TEXT("https://kemsu.ru"));
    RootBox->AddChildToVerticalBox(UrlTextBox);

    UKemActionButton* OpenButton = CreateActionButton(TEXT("Open URL"), TEXT("open_typed"));
    OpenButton->OnKemActionClicked.AddDynamic(this, &UKemBrowserDeskWidget::HandleAction);
    RootBox->AddChildToVerticalBox(OpenButton);

    const TArray<TPair<FString, FString>> QuickUrls = {
        {TEXT("KemSU"), TEXT("quick_kemsu")},
        {TEXT("Wikipedia"), TEXT("quick_wiki")},
        {TEXT("OpenAI"), TEXT("quick_openai")}
    };

    for (const TPair<FString, FString>& QuickUrl : QuickUrls)
    {
        UKemActionButton* Button = CreateActionButton(QuickUrl.Key, QuickUrl.Value);
        Button->OnKemActionClicked.AddDynamic(this, &UKemBrowserDeskWidget::HandleAction);
        RootBox->AddChildToVerticalBox(Button);
    }

    CurrentUrlText = CreateTextBlock(TEXT("Current URL: not loaded"), 16, FLinearColor(0.85f, 0.88f, 0.95f, 1.0f));
    RootBox->AddChildToVerticalBox(CurrentUrlText);

    USizeBox* BrowserSizeBox = WidgetTree->ConstructWidget<USizeBox>();
    BrowserSizeBox->SetWidthOverride(900.0f);
    BrowserSizeBox->SetHeightOverride(500.0f);
    BrowserWidget = WidgetTree->ConstructWidget<UWebBrowser>();
    BrowserWidget->LoadURL(TEXT("https://kemsu.ru"));
    BrowserSizeBox->SetContent(BrowserWidget);
    RootBox->AddChildToVerticalBox(BrowserSizeBox);

    CachedUrl = TEXT("https://kemsu.ru");
    CurrentUrlText->SetText(FText::FromString(FString::Printf(TEXT("Current URL: %s"), *CachedUrl)));
}

void UKemBrowserDeskWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (AKemClassroomStateActor* RoomState = GetRoomState())
    {
        const FString LectureUrl = RoomState->GetLectureUrl();
        if (!LectureUrl.IsEmpty() && LectureUrl != CachedUrl)
        {
            LoadUrl(LectureUrl, false);
        }
    }
}

AKemClassroomStateActor* UKemBrowserDeskWidget::GetRoomState() const
{
    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        return RoomSubsystem->GetCurrentClassroomState();
    }
    return nullptr;
}

void UKemBrowserDeskWidget::LoadUrl(const FString& Url, bool bReplicateToRoomState)
{
    CachedUrl = Url;
    if (BrowserWidget)
    {
        BrowserWidget->LoadURL(Url);
    }
    if (CurrentUrlText)
    {
        CurrentUrlText->SetText(FText::FromString(FString::Printf(TEXT("Current URL: %s"), *Url)));
    }
    if (bReplicateToRoomState)
    {
        if (AKemClassroomStateActor* RoomState = GetRoomState())
        {
            RoomState->SetLectureUrl(Url);
        }
    }
}

void UKemBrowserDeskWidget::HandleAction(const FString& ActionId)
{
    if (ActionId == TEXT("open_typed"))
    {
        const FString Url = UrlTextBox ? UrlTextBox->GetText().ToString() : TEXT("https://kemsu.ru");
        LoadUrl(Url, true);
    }
    else if (ActionId == TEXT("quick_kemsu"))
    {
        LoadUrl(TEXT("https://kemsu.ru"), true);
    }
    else if (ActionId == TEXT("quick_wiki"))
    {
        LoadUrl(TEXT("https://www.wikipedia.org"), true);
    }
    else if (ActionId == TEXT("quick_openai"))
    {
        LoadUrl(TEXT("https://openai.com"), true);
    }
}

void UKemLectureScreenWidget::BuildUI()
{
    UBorder* RootBorder = CreatePanelBorder(FLinearColor(0.04f, 0.05f, 0.07f, 0.92f));
    UVerticalBox* RootBox = CreateVerticalBox();
    RootBorder->SetContent(RootBox);
    WidgetTree->RootWidget = RootBorder;

    TitleText = CreateTextBlock(TEXT("Lecture Screen"), 28);
    UrlText = CreateTextBlock(TEXT("URL: waiting for teacher"), 14, FLinearColor(0.8f, 0.82f, 0.9f, 1.0f));
    RootBox->AddChildToVerticalBox(TitleText);
    RootBox->AddChildToVerticalBox(UrlText);

    USizeBox* BrowserSizeBox = WidgetTree->ConstructWidget<USizeBox>();
    BrowserSizeBox->SetWidthOverride(1200.0f);
    BrowserSizeBox->SetHeightOverride(700.0f);
    BrowserWidget = WidgetTree->ConstructWidget<UWebBrowser>();
    BrowserSizeBox->SetContent(BrowserWidget);
    RootBox->AddChildToVerticalBox(BrowserSizeBox);

    CloneButton = CreateActionButton(TEXT("Create Personal Copy"), TEXT("clone_screen"));
    CloneButton->OnKemActionClicked.AddDynamic(this, &UKemLectureScreenWidget::HandleAction);
    CloneButton->SetVisibility(bAllowCloneButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    RootBox->AddChildToVerticalBox(CloneButton);
}

void UKemLectureScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (AKemClassroomStateActor* RoomState = GetRoomState())
    {
        const FString LectureUrl = RoomState->GetLectureUrl();
        if (!LectureUrl.IsEmpty() && LectureUrl != CachedLectureUrl)
        {
            LoadLectureUrl(LectureUrl);
        }
    }
}

void UKemLectureScreenWidget::SetAllowLocalCloneButton(bool bAllowClone)
{
    bAllowCloneButton = bAllowClone;
    if (CloneButton)
    {
        CloneButton->SetVisibility(bAllowCloneButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

AKemClassroomStateActor* UKemLectureScreenWidget::GetRoomState() const
{
    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        return RoomSubsystem->GetCurrentClassroomState();
    }
    return nullptr;
}

void UKemLectureScreenWidget::LoadLectureUrl(const FString& Url)
{
    CachedLectureUrl = Url;
    if (UrlText)
    {
        UrlText->SetText(FText::FromString(FString::Printf(TEXT("URL: %s"), *Url)));
    }
    if (BrowserWidget)
    {
        BrowserWidget->LoadURL(Url);
    }
}

void UKemLectureScreenWidget::HandleAction(const FString& ActionId)
{
    if (ActionId != TEXT("clone_screen") || !bAllowCloneButton)
    {
        return;
    }

    if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        FVector ViewLocation = FVector::ZeroVector;
        FRotator ViewRotation = FRotator::ZeroRotator;
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        if (AKemLocalLectureScreenActor* NewScreen = GetWorld()->SpawnActor<AKemLocalLectureScreenActor>(ViewLocation + ViewRotation.Vector() * 220.0f, ViewRotation, SpawnParameters))
        {
            NewScreen->SetActorScale3D(FVector(0.75f));
        }
    }
}

void UKemParticipantModeratorWidget::SetParticipantInfo(const FKemParticipantInfo& InParticipantInfo)
{
    ParticipantInfo = InParticipantInfo;
    if (NameText)
    {
        NameText->SetText(FText::FromString(ParticipantInfo.DisplayName));
    }
    if (VolumeSlider)
    {
        VolumeSlider->SetValue(ParticipantInfo.VolumeMultiplier / 2.0f);
    }
}

void UKemParticipantModeratorWidget::BuildUI()
{
    UBorder* RootBorder = CreatePanelBorder(FLinearColor(0.06f, 0.07f, 0.11f, 0.84f));
    UVerticalBox* RootBox = CreateVerticalBox();
    RootBorder->SetContent(RootBox);
    WidgetTree->RootWidget = RootBorder;

    NameText = CreateTextBlock(TEXT("Participant"), 24);
    RootBox->AddChildToVerticalBox(NameText);
    RootBox->AddChildToVerticalBox(CreateTextBlock(TEXT("Volume"), 16, FLinearColor(0.8f, 0.84f, 0.92f, 1.0f)));

    VolumeSlider = CreateSlider(0.5f);
    VolumeSlider->OnValueChanged.AddDynamic(this, &UKemParticipantModeratorWidget::HandleVolumeChanged);
    RootBox->AddChildToVerticalBox(VolumeSlider);

    const TArray<TPair<FString, FString>> Actions = {
        {TEXT("Mute / Unmute"), TEXT("toggle_mute")},
        {TEXT("Kick"), TEXT("kick")},
        {TEXT("Ban"), TEXT("ban")}
    };

    for (const TPair<FString, FString>& Action : Actions)
    {
        UKemActionButton* Button = CreateActionButton(Action.Key, Action.Value);
        Button->OnKemActionClicked.AddDynamic(this, &UKemParticipantModeratorWidget::HandleAction);
        RootBox->AddChildToVerticalBox(Button);
    }
}

AKemClassroomStateActor* UKemParticipantModeratorWidget::GetRoomState() const
{
    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        return RoomSubsystem->GetCurrentClassroomState();
    }
    return nullptr;
}

void UKemParticipantModeratorWidget::HandleAction(const FString& ActionId)
{
    AKemClassroomStateActor* RoomState = GetRoomState();
    if (!RoomState)
    {
        return;
    }

    if (ActionId == TEXT("toggle_mute"))
    {
        ParticipantInfo.bForcedMuted = !ParticipantInfo.bForcedMuted;
        RoomState->SetParticipantMuted(ParticipantInfo.ParticipantId, ParticipantInfo.bForcedMuted);
    }
    else if (ActionId == TEXT("kick"))
    {
        RoomState->KickParticipant(ParticipantInfo.ParticipantId);
    }
    else if (ActionId == TEXT("ban"))
    {
        RoomState->BanParticipant(ParticipantInfo.ParticipantId);
    }
}

void UKemParticipantModeratorWidget::HandleVolumeChanged(float NewValue)
{
    ParticipantInfo.VolumeMultiplier = FMath::Lerp(0.0f, 2.0f, NewValue);
    if (AKemClassroomStateActor* RoomState = GetRoomState())
    {
        RoomState->SetParticipantVolume(ParticipantInfo.ParticipantId, ParticipantInfo.VolumeMultiplier);
    }
}
