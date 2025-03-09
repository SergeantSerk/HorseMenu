#include "game/rdr/Natives.hpp"
#include "game/backend/Self.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/commands/PlayerCommand.hpp"

namespace YimMenu::Features
{
	enum AnimFlag
	{
		ONE_SHOT,
		REPEAT
	};

    static void PlayAnim(std::string anim_dict, std::string anim_name, AnimFlag anim_flag)
	{
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim_dict.c_str()))
		{
			STREAMING::REQUEST_ANIM_DICT(anim_dict.c_str());
			ScriptMgr::Yield();
		}

		TASK::TASK_PLAY_ANIM(Self::GetPed().GetHandle(), anim_dict.c_str(), anim_name.c_str(), 1.0f, 1.0f, -1, anim_flag, 0.0f, false, 0, false, "", false);
	}

	static void ClearPedTasks(int ped)
	{
		PED::SET_PED_SHOULD_PLAY_IMMEDIATE_SCENARIO_EXIT(ped);
		TASK::CLEAR_PED_TASKS_IMMEDIATELY(ped, true, true);
		ScriptMgr::Yield(50ms);
	}

	class Chicken : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@chicken@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Chicken1 : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@chicken@male@unarmed@full", "fullbody_alt1", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Chicken2 : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@chicken@male@unarmed@full", "fullbody_alt2", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Cuckoo : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@cuckoo@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Facepalm : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@facepalm@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Fiddlehead : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@fiddlehead@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class FingerSlinger : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@finger_slinger@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class FlyingKiss : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@flying_kiss@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class GorillaChest : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@gorilla_chest@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Howl : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@howl@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class HushYourMouth : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@hush_your_mouth@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class PointLaugh : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@pointlaugh@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Scheme : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@scheme@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Shrug : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@shrug@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Shuffle : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@shuffle@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Spooky : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@spooky@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Tada : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@tada@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

    class WagFinger : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@wagfinger@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	class Warcry : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim("script_mp@emotes@war_cry@male@unarmed@full", "fullbody", AnimFlag::ONE_SHOT);
			}
		}
	};

	static Cuckoo _Cuckoo{"cuckoo", "Cuckoo", "Cuckoo cuckoo"};
	static Facepalm _Facepalm{"facepalm", "Facepalm", "Facepalm in disappointment"};
	static Fiddlehead _Fiddlehead{"fiddlehead", "Fiddlehead", "Blah blah blah!"};
	static FingerSlinger _FingerSlinger{"fingerslinger", "Finger Slinger", "Show middle finger from the hip"};
	static FlyingKiss _FlyingKiss{"flyingkiss", "Flying Kiss", "Do a flying kiss"};
	static GorillaChest _GorillaChest{"gorillachest", "Gorilla Chest", "Shout and pound your chest like a gorilla"};
	static Howl _Howl{"howl", "Howl", "Howl like a wolf"};
	static HushYourMouth _HushYourMouth{"hushyourmouth", "Hush Your Mouth", "Hush your mouth!"};
	static PointLaugh _PointLaugh{"pointlaugh", "Point Laugh", "Point and laugh"};
	static Scheme _Scheme{"scheme", "Scheme", "Planning something devious"};
	static Shrug _Shrug{"shrug", "Shrug", "Shrug"};
	static Shuffle _Shuffle{"shuffle", "Shuffle", "Dance and shuffle around"};
	static Spooky _Spooky{"spooky", "Spooky", "Spooooooooky"};
	static Tada _Tada{"tada", "Tada", "Tadaaaaaaa!"};
	static WagFinger _WagFinger{"wagfinger", "Wag Finger", "Wag your finger"};
	static Warcry _Warcry{"warcry", "Warcry", "Do a warcry"};
}