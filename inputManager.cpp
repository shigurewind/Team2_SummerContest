#include "inputManager.h"
#include "input.h"


InputManager* g_pInputManager = nullptr;

InputManager::InputManager()
{
    InitializeDefaultBindings();
}

InputManager::~InputManager()
{
}

//デフォルトのキー設定
void InputManager::InitializeDefaultBindings()
{
    for (int i = 0; i < ACTION_MAX; i++)
    {
        actionBindings[i].clear();
    }

	// Default
    // 移動---------------------------------------------------------------
    BindAction(ACTION_MOVE_FORWARD, InputBinding(INPUTDEVICE_KEYBOARD, DIK_W));
    BindAction(ACTION_MOVE_FORWARD, InputBinding(INPUTDEVICE_LEFT_STICK, 1));

    BindAction(ACTION_MOVE_BACKWARD, InputBinding(INPUTDEVICE_KEYBOARD, DIK_S));
    BindAction(ACTION_MOVE_BACKWARD, InputBinding(INPUTDEVICE_LEFT_STICK, -1));

    BindAction(ACTION_MOVE_LEFT, InputBinding(INPUTDEVICE_KEYBOARD, DIK_A));
    BindAction(ACTION_MOVE_LEFT, InputBinding(INPUTDEVICE_LEFT_STICK, 2));

    BindAction(ACTION_MOVE_RIGHT, InputBinding(INPUTDEVICE_KEYBOARD, DIK_D));
    BindAction(ACTION_MOVE_RIGHT, InputBinding(INPUTDEVICE_LEFT_STICK, 3));

    //ジャンプ
    BindAction(ACTION_JUMP, InputBinding(INPUTDEVICE_KEYBOARD, DIK_SPACE));
    BindAction(ACTION_JUMP, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_A, 0));

    //操作---------------------------------------------------------------
    BindAction(ACTION_LIGHT_SWITCH, InputBinding(INPUTDEVICE_KEYBOARD, DIK_T));
    BindAction(ACTION_LIGHT_SWITCH, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_A, 0));

	// 攻撃---------------------------------------------------------------
    //射撃
    BindAction(ACTION_SHOOT, InputBinding(INPUTDEVICE_MOUSE, 0, 0));
    BindAction(ACTION_SHOOT, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_R2, 0));
	//近接攻撃
    BindAction(ACTION_MELEE, InputBinding(INPUTDEVICE_MOUSE, 1, 0));
    BindAction(ACTION_MELEE, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_X, 0));


    // 武器---------------------------------------------------------------
	// 武器変更
    BindAction(ACTION_WEAPON_CHANGE, InputBinding(INPUTDEVICE_KEYBOARD, DIK_1));
    BindAction(ACTION_WEAPON_CHANGE, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_L, 0));
	// 弾薬変更
    BindAction(ACTION_BULLET_CHANGE, InputBinding(INPUTDEVICE_KEYBOARD, DIK_2));
    BindAction(ACTION_BULLET_CHANGE, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_R, 0));

    // アイテム---------------------------------------------------------------
	// アイテム使用
    BindAction(ACTION_USE_ITEM, InputBinding(INPUTDEVICE_KEYBOARD, DIK_F));
    BindAction(ACTION_USE_ITEM, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_Y, 0));
	// 先のアイテム
    BindAction(ACTION_LAST_ITEM, InputBinding(INPUTDEVICE_KEYBOARD, DIK_Q));
    BindAction(ACTION_LAST_ITEM, InputBinding(INPUTDEVICE_DPAD, BUTTON_LEFT, 0));
	// 次のアイテム
    BindAction(ACTION_NEXT_ITEM, InputBinding(INPUTDEVICE_KEYBOARD, DIK_E));
    BindAction(ACTION_NEXT_ITEM, InputBinding(INPUTDEVICE_DPAD, BUTTON_RIGHT, 0));

    // UI---------------------------------------------------------------
	// メニュー
    BindAction(ACTION_MENU, InputBinding(INPUTDEVICE_KEYBOARD, DIK_ESCAPE));
    BindAction(ACTION_MENU, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_SELECT, 0));
	// 決定
    BindAction(ACTION_CONFIRM, InputBinding(INPUTDEVICE_KEYBOARD, DIK_RETURN));
    BindAction(ACTION_CONFIRM, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_A, 0));
	// キャンセル
    BindAction(ACTION_CANCEL, InputBinding(INPUTDEVICE_KEYBOARD, DIK_ESCAPE));
    BindAction(ACTION_CANCEL, InputBinding(INPUTDEVICE_GAMEPAD, BUTTON_B, 0));
    
    

    
}


// 状態チェック関数
bool InputManager::IsActionPressed(GameAction action)
{
    if (action >= ACTION_MAX) return false;

    for (const auto& binding : actionBindings[action])
    {
        if (IsBindingPressed(binding))
        {
            return true;
        }
    }
    return false;
}

bool InputManager::IsActionTriggered(GameAction action)
{
    if (action >= ACTION_MAX) return false;

    for (const auto& binding : actionBindings[action])
    {
        if (IsBindingTriggered(binding))
        {
            return true;
        }
    }
    return false;
}

bool InputManager::IsActionReleased(GameAction action)
{
    if (action >= ACTION_MAX) return false;

    for (const auto& binding : actionBindings[action])
    {
        if (IsBindingReleased(binding))
        {
            return true;
        }
    }
    return false;
}


// バインディング管理関数
void InputManager::BindAction(GameAction action, InputBinding binding)
{
    if (action >= ACTION_MAX) return;

    actionBindings[action].push_back(binding);
}

void InputManager::ClearBindings(GameAction action)
{
    if (action >= ACTION_MAX) return;

    actionBindings[action].clear();
}

std::vector<InputBinding> InputManager::GetBindings(GameAction action)
{
    if (action >= ACTION_MAX) return std::vector<InputBinding>();

    return actionBindings[action];
}


// キーバインディング状態チェック関数
bool InputManager::IsBindingPressed(const InputBinding& binding)
{
    switch (binding.device)
    {
    case INPUTDEVICE_KEYBOARD:
        return GetKeyboardPress(binding.key) ? true : false;

    case INPUTDEVICE_GAMEPAD:
        return IsButtonPressed(binding.padIndex, binding.key) ? true : false;

    case INPUTDEVICE_MOUSE:
        switch (binding.key)
        {
        case 0: return IsMouseLeftPressed() ? true : false;
        case 1: return IsMouseRightPressed() ? true : false;
        case 2: return IsMouseCenterPressed() ? true : false;
        default: return false;
        }

    case INPUTDEVICE_LEFT_STICK:
    {
        float x = GetLeftStickX(binding.padIndex);
        float y = GetLeftStickY(binding.padIndex);
		float threshold = 0.3f; // deadzone threshold

        switch (binding.key)
        {
        case 1: return y > threshold;   // y+
        case -1: return y < -threshold; // y-
        case 2: return x < -threshold;  // x-
        case 3: return x > threshold;   // x+
        default: return false;
        }
    }

    case INPUTDEVICE_RIGHT_STICK:
    {
        float x = GetRightStickX(binding.padIndex);
        float y = GetRightStickY(binding.padIndex);
        float threshold = 0.3f;

        switch (binding.key)
        {
        case 1: return y > threshold;
        case -1: return y < -threshold;
        case 2: return x < -threshold;
        case 3: return x > threshold;
        default: return false;
        }
    }

    case INPUTDEVICE_DPAD:
		// 十字キー
        switch (binding.key)
        {
        case BUTTON_UP: return IsDPadUpTriggered(binding.padIndex);
        case BUTTON_DOWN: return IsDPadDownTriggered(binding.padIndex);
        case BUTTON_LEFT: return IsDPadLeftTriggered(binding.padIndex);
        case BUTTON_RIGHT: return IsDPadRightTriggered(binding.padIndex);
        default: return false;
        }

    default:
        return false;
    }
}


bool InputManager::IsBindingTriggered(const InputBinding& binding)
{
    switch (binding.device)
    {
    case INPUTDEVICE_KEYBOARD:
        return GetKeyboardTrigger(binding.key) ? true : false;

    case INPUTDEVICE_GAMEPAD:
        return IsButtonTriggered(binding.padIndex, binding.key) ? true : false;

    case INPUTDEVICE_MOUSE:
        switch (binding.key)
        {
        case 0: return IsMouseLeftTriggered() ? true : false;
        case 1: return IsMouseRightTriggered() ? true : false;
        case 2: return IsMouseCenterTriggered() ? true : false;
        default: return false;
        }

    case INPUTDEVICE_LEFT_STICK:
    case INPUTDEVICE_RIGHT_STICK:
        return IsBindingPressed(binding);

    case INPUTDEVICE_DPAD:
        switch (binding.key)
        {
        case BUTTON_UP: return IsDPadUpTriggered(binding.padIndex);
        case BUTTON_DOWN: return IsDPadDownTriggered(binding.padIndex);
        case BUTTON_LEFT: return IsDPadLeftTriggered(binding.padIndex);
        case BUTTON_RIGHT: return IsDPadRightTriggered(binding.padIndex);
        default: return false;
        }

    default:
        return false;
    }
}

bool InputManager::IsBindingReleased(const InputBinding& binding)
{
    switch (binding.device)
    {
    case INPUTDEVICE_KEYBOARD:
        return GetKeyboardRelease(binding.key) ? true : false;

    case INPUTDEVICE_GAMEPAD:
    case INPUTDEVICE_MOUSE:
    case INPUTDEVICE_LEFT_STICK:
    case INPUTDEVICE_RIGHT_STICK:
    case INPUTDEVICE_DPAD:
		// このデバイスではリリース状態のチェックはサポートしていません
        return false;

    default:
        return false;
    }
}


// スティック値取得関数
float InputManager::GetLeftStickXValue(int padIndex)
{
    return GetLeftStickX(padIndex);
}

float InputManager::GetLeftStickYValue(int padIndex)
{
    return GetLeftStickY(padIndex);
}

float InputManager::GetRightStickXValue(int padIndex)
{
    return GetRightStickX(padIndex);
}

float InputManager::GetRightStickYValue(int padIndex)
{
    return GetRightStickY(padIndex);
}



