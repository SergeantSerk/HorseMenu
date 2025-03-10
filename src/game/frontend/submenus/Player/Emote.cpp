#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/commands/PlayerCommand.hpp"
#include "game/rdr/Natives.hpp"

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

	class Emote : public PlayerCommand
	{
		virtual void OnCall(Player player) override
		{
			if (player.GetPed().IsValid())
			{
				ClearPedTasks(player.GetPed().GetHandle());
				PlayAnim(anim_dict, anim_name, AnimFlag::ONE_SHOT);
			}
		}

	public:
		Emote(std::string name, std::string label, std::string description, std::string anim_dict, std::string anim_name, int num_args = 0, bool all_version = true) :
		    PlayerCommand(name, label, description, num_args, all_version),
		    anim_dict(anim_dict),
		    anim_name(anim_name)
		{
		}

	private:
		std::string anim_dict;
		std::string anim_name;
	};

	static Emote _Chicken{"chicken", "Chicken", "Bawk bawk bawk!", "script_mp@emotes@chicken@male@unarmed@full", "fullbody"};
	static Emote _Cuckoo{"cuckoo", "Cuckoo", "Cuckoo cuckoo", "script_mp@emotes@cuckoo@male@unarmed@full", "fullbody"};
	static Emote _Facepalm{"facepalm", "Facepalm", "Facepalm in disappointment", "script_mp@emotes@facepalm@male@unarmed@full", "fullbody"};
	static Emote _Fiddlehead{"fiddlehead", "Fiddlehead", "Blah blah blah!", "script_mp@emotes@fiddlehead@male@unarmed@full", "fullbody"};
	static Emote _FingerSlinger{"fingerslinger", "Finger Slinger", "Show middle finger from the hip", "script_mp@emotes@finger_slinger@male@unarmed@full", "fullbody"};
	static Emote _FlyingKiss{"flyingkiss", "Flying Kiss", "Do a flying kiss", "script_mp@emotes@flying_kiss@male@unarmed@full", "fullbody"};
	static Emote _GorillaChest{"gorillachest", "Gorilla Chest", "Shout and pound your chest like a gorilla", "script_mp@emotes@gorilla_chest@male@unarmed@full", "fullbody"};
	static Emote _Howl{"howl", "Howl", "Howl like a wolf", "script_mp@emotes@howl@male@unarmed@full", "fullbody"};
	static Emote _HushYourMouth{"hushyourmouth", "Hush Your Mouth", "Hush your mouth!", "script_mp@emotes@hush_your_mouth@male@unarmed@full", "fullbody"};
	static Emote _PointLaugh{"pointlaugh", "Point Laugh", "Point and laugh", "script_mp@emotes@pointlaugh@male@unarmed@full", "fullbody"};
	static Emote _Scheme{"scheme", "Scheme", "Planning something devious", "script_mp@emotes@scheme@male@unarmed@full", "fullbody"};
	static Emote _Shrug{"shrug", "Shrug", "Shrug", "script_mp@emotes@shrug@male@unarmed@full", "fullbody"};
	static Emote _Shuffle{"shuffle", "Shuffle", "Dance and shuffle around", "script_mp@emotes@shuffle@male@unarmed@full", "fullbody"};
	static Emote _Spooky{"spooky", "Spooky", "Spooooooooky", "script_mp@emotes@spooky@male@unarmed@full", "fullbody"};
	static Emote _Tada{"tada", "Tada", "Tadaaaaaaa!", "script_mp@emotes@tada@male@unarmed@full", "fullbody"};
	static Emote _WagFinger{"wagfinger", "Wag Finger", "Wag your finger", "script_mp@emotes@wagfinger@male@unarmed@full", "fullbody"};
	static Emote _Warcry{"warcry", "Warcry", "Do a warcry", "script_mp@emotes@war_cry@male@unarmed@full", "fullbody"};
}