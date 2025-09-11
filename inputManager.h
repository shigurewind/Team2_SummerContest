#pragma once
#include "main.h"
#include <map>
#include <vector>


enum GameAction
{
	// 移動
	ACTION_MOVE_FORWARD,
	ACTION_MOVE_BACKWARD,
	ACTION_MOVE_LEFT,
	ACTION_MOVE_RIGHT,
	ACTION_JUMP,

	// 操作
	ACTION_LIGHT_SWITCH,


	// 攻撃
	ACTION_SHOOT,
	ACTION_MELEE,

	// 武器
	ACTION_WEAPON_CHANGE,
	ACTION_BULLET_CHANGE,


	// アイテム
	ACTION_USE_ITEM,
	ACTION_LAST_ITEM,
	ACTION_NEXT_ITEM,

	// UI
	ACTION_MENU,
	ACTION_CONFIRM,
	ACTION_CANCEL,

	// その他
	ACTION_RESET_POSITION,

	// アクション数
	ACTION_MAX
};


// 入力デバイス
enum InputDevice
{
	INPUTDEVICE_KEYBOARD,
	INPUTDEVICE_GAMEPAD,
	INPUTDEVICE_MOUSE,
	INPUTDEVICE_LEFT_STICK,
	INPUTDEVICE_RIGHT_STICK,
	INPUTDEVICE_DPAD        //十字キー
};


struct InputBinding
{
	InputDevice device;
	int key;        // キーコードまたはマウスボタンコード、ゲームパッドのボタンコード
	int padIndex;   // ゲームパッドのインデックス（0から始まる）

	InputBinding() : device(INPUTDEVICE_KEYBOARD), key(0), padIndex(0) {}
	InputBinding(InputDevice dev, int k, int pad = 0) : device(dev), key(k), padIndex(pad) {}
};


// InputManagerクラス
class InputManager
{
private:
	// 全てのアクションに対するバインディング
	std::vector<InputBinding> actionBindings[ACTION_MAX];

	// リセット用長押し管理
	float resetHoldTime;           // 現在の長押し時間
	const float RESET_HOLD_DURATION = 2.0f; // リセット必要時間

public:
	InputManager();
	~InputManager();

	// 初期化
	void InitializeDefaultBindings();

	// アクションの状態チェック
	bool IsActionPressed(GameAction action);
	bool IsActionTriggered(GameAction action);
	bool IsActionReleased(GameAction action);

	// キー/ボタンのバインド管理
	void BindAction(GameAction action, InputBinding binding);
	void ClearBindings(GameAction action);
	std::vector<InputBinding> GetBindings(GameAction action);

	// 状態チェック関数
	bool IsBindingPressed(const InputBinding& binding);
	bool IsBindingTriggered(const InputBinding& binding);
	bool IsBindingReleased(const InputBinding& binding);

	// アナログスティックの値取得
	float GetLeftStickXValue(int padIndex = 0);
	float GetLeftStickYValue(int padIndex = 0);
	float GetRightStickXValue(int padIndex = 0);
	float GetRightStickYValue(int padIndex = 0);


	void Update(float deltaTime);

	// リセット長押しチェック
	bool IsResetTriggered();
};


// グローバルなInputManagerインスタンス
extern InputManager* g_pInputManager;