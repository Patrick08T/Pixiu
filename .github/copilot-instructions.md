# Pixiu AI Coding Guidelines

## Project Overview
**Pixiu** is an Unreal Engine 5.4 action game project using GAS (Gameplay Ability System) for character abilities and the Enhanced Input System for player controls.

**Key Dependencies:** UE5.4, GameplayAbilities plugin, EnhancedInput, UMG (UI), Slate

---

## Architecture Patterns

### 1. Character System (Server/Client Initialization)
Characters use **split initialization** between server and client:
- **`ServerSideInit()`** (in [Source/Pixiu/Private/Character/CCharacter.h](Source/Pixiu/Private/Character/CCharacter.h#L14)): Called on server during possession, initializes GAS and applies initial effects via `UCAbilitySystemComponent::ApplyInitialEffects()`
- **`ClientSideInit()`**: Called on client to initialize GAS actor info
- **Timing:** Server init happens in `ACCharacter::PossessedBy()` override; client init in `ACPlayerController::AcknowledgePossession()`

**Key File:** [Source/Pixiu/Private/Character/CCharacter.cpp](Source/Pixiu/Private/Character/CCharacter.cpp) shows the pattern.

### 2. GAS Integration
- All characters inherit from `ACCharacter` which implements `IAbilitySystemInterface`
- **Attribute System:** [Source/Pixiu/Private/GAS/CAttributeSet.h](Source/Pixiu/Private/GAS/CAttributeSet.h) defines replicated attributes (Health, MaxHealth, Mana, MaxMana) with `OnRep_*` callbacks
- **Ability System Component:** [Source/Pixiu/Private/GAS/CAbilitySystemComponent.h](Source/Pixiu/Private/GAS/CAbilitySystemComponent.h) wraps `UAbilitySystemComponent`, calls `ApplyInitialEffects()` on server init
- Initial gameplay effects are configured via `TArray<TSubclassOf<UGameplayEffect>> InitialEffects` in blueprint

### 3. Player Character Input (Enhanced Input System)
- [CPlayerCharacter.h](Source/Pixiu/Private/Player/CPlayerCharacter.h) uses Enhanced Input with three input actions: Move, Look, Jump
- **Input Setup:** Done in `PawnClientRestart()` by adding `GamePlayerCharacterMappingContext` to Enhanced Input subsystem
- **Camera:** Spring arm with controller rotation; camera moves in viewport space (uses `Camera->GetRightVector()` and `Camera->GetForwardVector()` for movement directions)
- **Movement:** Bindings trigger `HandleMoveInput()` and `HandleLookInput()` which apply movement/rotation

### 4. Animation State Management
- [CAnimInstance.h](Source/Pixiu/Private/Animations/CAnimInstance.h) uses three native update overrides:
  - `NativeInitializeAnimation()` → setup
  - `NativeUpdateAnimation()` → gather data (not thread-safe)
  - `NativeThreadSafeUpdateAnimation()` → heavy lifting (thread-safe, worker thread)
- Exposed blueprint-callable getters for Speed, YawSpeed, IsJumping, LookRotOffset (expose animation state to blueprints)

### 5. UI Widget System
- **Gameplay Widget:** [CPlayerController.cpp](Source/Pixiu/Private/Player/CPlayerController.cpp) spawns `UGameplayWidget` on client via `AcknowledgePossession()` (local player check)
- **Widget Binding:** Uses `meta = (BindWidget)` properties (HealthBar, ManaBar) auto-bound in UMG
- **OverHead Status Display:** [OverHeadStatusGauge.h](Source/Pixiu/Private/Widgets/OverHeadStatusGauge.h) shows enemy health; hidden for player-controlled characters
- **Visibility Update:** Background timer checks player camera distance to toggle gauge visibility

---

## Critical Conventions

### Naming
- **Classes:** C-prefix (CCharacter, CPlayerCharacter, CAbilitySystemComponent) — inherited convention from the project
- **Assets in Content/:** Blueprint suffix with _BP (e.g., CPlayerCharacter_BP, CGameMode_BP)

### Replication
- Attributes in `CAttributeSet` marked `Replicated` with `OnRep_*` callbacks for property changes
- Network transmission: Server applies effects → attribute changes → OnRep callbacks update clients

### Blueprint Configuration
- Input actions defined in Content/Input/ (IA_Move, IA_Look, IA_Jump, IM_Gameplay mapping context)
- Character blueprints store asset references (skeleton mesh) and assign input actions in details

### Content Organization
- [Content/Characters/](Content/Characters/) → character skeletal meshes
- [Content/Framework/](Content/Framework/) → game mode, player controller blueprints
- [Content/Player/](Content/Player/) → player-specific blueprints
- [Content/Widgets/Gameplay/](Content/Widgets/Gameplay/) → UI layouts and classes
- [Content/ParagonCrunch/](Content/ParagonCrunch/) → external character assets

---

## Important Development Notes

### Build Configuration
- **Default Game Mode:** `/Game/Framework/CGameMode_BP` (set in DefaultEngine.ini)
- **Project Dependencies:** [Pixiu.Build.cs](Source/Pixiu/Pixiu.Build.cs) requires GameplayAbilities, UMG; EnhancedInput modules
- **Plugin Setup:** GameplayAbilities enabled; ModelingToolsEditorMode for editor only

### Common Workflows
1. **Add new ability:** Create `UGameplayAbility` subclass, add to `InitialEffects` array in blueprint
2. **Add UI element:** Create `UUserWidget` subclass with `meta = (BindWidget)` properties; spawn in controller
3. **Modify movement:** Update [CPlayerCharacter.cpp](Source/Pixiu/Private/Player/CPlayerCharacter.cpp) direction vectors (GetLookRightDir, GetMoveFwdDir)
4. **Debug network issues:** Check server/client init order in CCharacter's PossessedBy + AcknowledgePossession

### Known Patterns NOT to Break
- Do **not** call ClientSideInit before server-side GAS setup completes (sequencing matters)
- OverHead widgets must respect `IsLocallyControlledByPlayer()` check to avoid showing self
- Enhanced Input mappings must be removed before re-adding (see PawnClientRestart pattern)

---

## Key References
- [Source/Pixiu/Private/Character/CCharacter.h](Source/Pixiu/Private/Character/CCharacter.h) — Base character with GAS
- [Source/Pixiu/Private/Player/CPlayerCharacter.h](Source/Pixiu/Private/Player/CPlayerCharacter.h) — Player controller + input
- [Source/Pixiu/Private/GAS/CAttributeSet.h](Source/Pixiu/Private/GAS/CAttributeSet.h) — Attribute definitions
- [Source/Pixiu/Pixiu.Build.cs](Source/Pixiu/Pixiu.Build.cs) — Module dependencies
- [Config/DefaultEngine.ini](Config/DefaultEngine.ini) — Game mode, graphics settings
- [Content/Input/](Content/Input/) — Input action assets
