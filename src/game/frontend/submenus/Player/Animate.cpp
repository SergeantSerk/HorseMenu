#include "Animate.hpp"

#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Vehicle.hpp"
#include "game/rdr/Ped.hpp"
#include "util/network.hpp"

namespace YimMenu::Submenus
{
	constexpr auto g_AnimationTypeStrs = std::to_array({"Propose", "Sex", "Bathe"});

	enum class ActorOverrideType
	{
		DEFAULT,
		SELF,
		PLAYER
	};

	struct ActorDefinition
	{
		ActorOverrideType Type = ActorOverrideType::DEFAULT;
		Player OverridePlayer{};
	};

	/*
	Bathing coordinates
	X/Y/Z (leftright/forwardbackward/updown)
	Rhodes: 1336.29, -1377.96, 84.35
	St Denis: 2629.43, -1223.90, 59.61
	Annesburg: 2952.87, 1335.22, 44.50
	/Blackwater: -823.29, -1318.93, 43.72
	Strawberry: -1812.33, -373.24, 166.56
	Vanhorn: 2987.74, 573.77, 47.92
	Tumbleweed: -5513.23, -2972.15, -0.72
	*/

	enum class AnimationType
	{
		PROPOSE,
		SEX,
		BATHE
	};

	static AnimationType g_SelectedAnimationType = AnimationType::PROPOSE;

	// Propose
	static ActorDefinition g_ProposeMaleOverride;
	static ActorDefinition g_ProposeFemaleOverride;

	// Sex
	static ActorDefinition g_SexMaleOverride;
	static ActorDefinition g_SexFemaleOverride;

	// Bathe
	static ActorDefinition g_BatherOverride;
	static ActorDefinition g_BatheMaidOverride;

	// Animation state tracking
	static Ped g_AnimatingMalePed = nullptr;
	static Ped g_AnimatingFemalePed = nullptr;
	static std::string g_CurrentAnimDict = "";
	static bool g_IsAnimating = false;


	constexpr auto oneshot_emote_names = std::to_array({"chicken"_J, "cuckoo"_J, "facepalm"_J, "fiddlehead"_J, "flyingkiss"_J, "fingerslinger"_J, "gorillachest"_J, "howl"_J, "hushyourmouth"_J, "pointlaugh"_J, "scheme"_J, "shrug"_J, "shuffle"_J, "spooky"_J, "tada"_J, "wagfinger"_J, "warcry"_J});

	constexpr auto repeating_emote_names = std::to_array({"cower"_J, "sittingupsleeping"_J, "sittingupsleepy"_J});

	// Helper to get Player object based on ActorDefinition
	Player GetPlayerFromActorDef(const ActorDefinition& def)
	{
		switch (def.Type)
		{
		case ActorOverrideType::SELF:
			return Self::GetPlayer();
		case ActorOverrideType::PLAYER:
			return def.OverridePlayer;
		case ActorOverrideType::DEFAULT:
		default: // Fallback for Default
			Notifications::Show("Animate", "Default actor selected, defaulting to self. Please select 'Yourself' or 'Player'.", NotificationType::Warning, 5000);
			return Self::GetPlayer();
		}
	}

	inline void RenderActorDef(ActorDefinition& def, const std::string& name)
	{
		ImGui::SetNextItemWidth(100);
		if (*Pointers.IsSessionStarted)
			ImGui::Combo(name.c_str(), (int*)&def.Type, "Default\0Yourself\0Player\0");
		else
			ImGui::Combo(name.c_str(), (int*)&def.Type, "Default\0Yourself\0");
		if (def.Type == ActorOverrideType::PLAYER)
		{
			ImGui::SameLine();
			auto player_name = def.OverridePlayer.IsValid() ? def.OverridePlayer.GetName() : "<SELECT>";
			ImGui::SetNextItemWidth(150);
			if (ImGui::BeginCombo(("##playerselect" + name).c_str(), player_name))
			{
				for (auto& [id, plyr] : Players::GetPlayers())
				{
					if (!plyr.IsValid())
						continue;

					if (ImGui::Selectable(plyr.GetName(), plyr == def.OverridePlayer))
					{
						def.OverridePlayer = plyr;
					}

					if (plyr == def.OverridePlayer)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
	}

	static void ClearPedTasks(int ped)
	{
		PED::SET_PED_SHOULD_PLAY_IMMEDIATE_SCENARIO_EXIT(ped);
		TASK::CLEAR_PED_TASKS_IMMEDIATELY(ped, true, true);
		ScriptMgr::Yield(50ms);
	}

	std::shared_ptr<Category> BuildAnimateMenu()
	{
		auto menu = std::make_shared<Category>("Animate");

		auto oneshot_emotes = std::make_shared<Group>("Predefined Emotes");
		for (auto& emote : oneshot_emote_names)
			oneshot_emotes->AddItem(std::make_shared<PlayerCommandItem>(emote));
		menu->AddItem(oneshot_emotes);

		auto repeating_emotes = std::make_shared<Group>("Repeating Emotes");
		for (auto& emote : repeating_emote_names)
			repeating_emotes->AddItem(std::make_shared<PlayerCommandItem>(emote));
		menu->AddItem(repeating_emotes);

		auto animations = std::make_shared<Group>("Animations");
		animations->AddItem(std::make_shared<ImGuiItem>([] {
			ImGui::SetNextItemWidth(100);
			ImGui::Combo("Show", (int*)&g_SelectedAnimationType, g_AnimationTypeStrs.data(), g_AnimationTypeStrs.size(), -1);

			if (g_SelectedAnimationType == AnimationType::PROPOSE)
			{
				RenderActorDef(g_ProposeMaleOverride, "Male");
				// Female is always the selected player for Propose, no need to render definition
				ImGui::Text("Female: %s", Players::GetSelected().IsValid() ? Players::GetSelected().GetName() : "<INVALID>");
			}
			else if (g_SelectedAnimationType == AnimationType::SEX)
			{
				RenderActorDef(g_SexMaleOverride, "Male");
				RenderActorDef(g_SexFemaleOverride, "Female");
			}
			else if (g_SelectedAnimationType == AnimationType::BATHE)
			{
				RenderActorDef(g_BatherOverride, "Bather");
				RenderActorDef(g_BatheMaidOverride, "Maid");
			}

			if (ImGui::Button("Animate"))
			{
				if (g_IsAnimating)
				{
					Notifications::Show("Animate", "An animation is already playing.", NotificationType::Warning);
				}
				else if (g_SelectedAnimationType == AnimationType::PROPOSE)
				{
					FiberPool::Push([] {
						Player female_player = Players::GetSelected(); // Female is always the selected player for propose
						Player male_player   = GetPlayerFromActorDef(g_ProposeMaleOverride);

						if (!female_player.IsValid() || !male_player.IsValid())
						{
							Notifications::Show("Animate", "Invalid player selected for animation.", NotificationType::Error);
							return;
						}

						Ped female_ped = female_player.GetPed();
						Ped male_ped   = male_player.GetPed();

						if (!female_ped || !male_ped)
						{
							Notifications::Show("Animate", "Cannot find peds for selected players.", NotificationType::Error);
							return;
						}

						if (female_ped == male_ped)
						{
							Notifications::Show("Animate", "Male and Female actors cannot be the same.", NotificationType::Error);
							return;
						}

						Network::RequestControlOfEntity(female_ped.GetHandle());
						female_ped.ForceControl();
						female_ped.GetMount().ForceControl();

						// Load Animation Dictionary
						const char* anim_dict = "script_re@proposal@accept";
						STREAMING::REQUEST_ANIM_DICT(anim_dict);
						for (int i = 0; !STREAMING::HAS_ANIM_DICT_LOADED(anim_dict); ++i)
						{
							if (i > 100) // Timeout after ~1 second
							{
								Notifications::Show("Animate", "Failed to load animation dictionary.", NotificationType::Error);
								STREAMING::REMOVE_ANIM_DICT(anim_dict);
								return;
							}
							ScriptMgr::Yield(10ms);
						}

						// Prepare Peds
						ClearPedTasks(female_ped.GetHandle());
						ClearPedTasks(male_ped.GetHandle());

						// Request control
						bool female_controlled = Network::RequestControlOfEntity(female_ped.GetHandle());
						bool male_controlled   = Network::RequestControlOfEntity(male_ped.GetHandle());

						if (!female_controlled)
						{
							Notifications::Show("Animate", "Failed to gain control of female ped.", NotificationType::Error);
							return;
						}
						else if (!male_controlled)
						{
							Notifications::Show("Animate", "Failed to gain control of male ped.", NotificationType::Error);
							return;
						}
						
						female_ped.SetCollision(false);
						male_ped.SetCollision(false);
						female_ped.SetFrozen(true);
						male_ped.SetFrozen(true);

						// Play Animation
						// Flag 0 = One Shot
						TASK::TASK_PLAY_ANIM(male_ped.GetHandle(), anim_dict, "action_male", 1.0f, 1.0f, -1, 0, 0.0f, false, 0, false, "", false);
						TASK::TASK_PLAY_ANIM(female_ped.GetHandle(), anim_dict, "action_female", 1.0f, 1.0f, -1, 0, 0.0f, false, 0, false, "", false);

						// Track state for stopping
						g_AnimatingMalePed   = male_ped;
						g_AnimatingFemalePed = female_ped;
						g_CurrentAnimDict    = anim_dict;
						g_IsAnimating        = true;

						Notifications::Show("Animate", "Playing proposal animation.", NotificationType::Success);
					}); // End FiberPool::Push for Animate button
				}
				else if (g_SelectedAnimationType == AnimationType::SEX)
				{
					// TODO: Implement Sex Animation
					Notifications::Show("Animate", "Sex animation not yet implemented.", NotificationType::Warning);
				}
				else if (g_SelectedAnimationType == AnimationType::BATHE)
				{
					// TODO: Implement Bathe Animation
					Notifications::Show("Animate", "Bathe animation not yet implemented.", NotificationType::Warning);
				}
			} // End Animate button

			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				if (!g_IsAnimating)
				{
					Notifications::Show("Animate", "No animation is currently playing.", NotificationType::Warning);
					return;
				}

				FiberPool::Push([] {
					if (g_AnimatingMalePed && g_AnimatingMalePed.IsValid())
					{
						TASK::CLEAR_PED_TASKS_IMMEDIATELY(g_AnimatingMalePed.GetHandle(), true, true);
						g_AnimatingMalePed.SetFrozen(false);
						g_AnimatingMalePed.SetCollision(true);
					}
					if (g_AnimatingFemalePed && g_AnimatingFemalePed.IsValid())
					{
						TASK::CLEAR_PED_TASKS_IMMEDIATELY(g_AnimatingFemalePed.GetHandle(), true, true);
						g_AnimatingFemalePed.SetFrozen(false);
						g_AnimatingFemalePed.SetCollision(true);
					}

					if (!g_CurrentAnimDict.empty())
					{
						if (STREAMING::HAS_ANIM_DICT_LOADED(g_CurrentAnimDict.c_str()))
						{
							STREAMING::REMOVE_ANIM_DICT(g_CurrentAnimDict.c_str());
						}
					}

					// Reset tracking variables
					g_AnimatingMalePed   = nullptr;
					g_AnimatingFemalePed = nullptr;
					g_CurrentAnimDict    = "";
					g_IsAnimating        = false;

					Notifications::Show("Animate", "Stopped animation.", NotificationType::Info);
				}); // End FiberPool::Push for Stop button
			} // End Stop button
		}));
		menu->AddItem(animations);

		return menu;
	}
}
