#include "Animate.hpp"

#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Vehicle.hpp"

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
	static AnimScene g_CurrentAnimScene = 0; // Store the current animation scene handle

	// Propose
	static ActorDefinition g_ProposeMaleOverride;
	static ActorDefinition g_ProposeFemaleOverride;

	// Sex
	static ActorDefinition g_SexMaleOverride;
	static ActorDefinition g_SexFemaleOverride;

	// Bathe
	static ActorDefinition g_BatherOverride;
	static ActorDefinition g_BatheMaidOverride;

	constexpr auto oneshot_emote_names = std::to_array({
		"chicken"_J,
		"cuckoo"_J,
		"facepalm"_J,
		"fiddlehead"_J,
		"flyingkiss"_J,
		"fingerslinger"_J,
		"gorillachest"_J,
		"howl"_J,
		"hushyourmouth"_J,
		"pointlaugh"_J,
		"scheme"_J,
		"shrug"_J,
		"shuffle"_J,
		"spooky"_J,
		"tada"_J,
		"wagfinger"_J,
		"warcry"_J
	});

	constexpr auto repeating_emote_names = std::to_array({
		"cower"_J,
		"sittingupsleeping"_J,
		"sittingupsleepy"_J
	});

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

	static Ped GetPedFromActorDef(ActorDefinition& def, bool is_male_role)
	{
		if (def.Type == ActorOverrideType::DEFAULT)
			return is_male_role ? Self::GetPed() : Players::GetSelected().GetPed();
		else if (def.Type == ActorOverrideType::SELF)
		{
			return Self::GetPed();
		}
		else
		{
			if (!def.OverridePlayer.IsValid())
				return -1;
			else
				return def.OverridePlayer.GetPed();
		}
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
				RenderActorDef(g_ProposeFemaleOverride, "Female");
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
				FiberPool::Push([] {
					if (g_CurrentAnimScene != 0 && ANIMSCENE::DOES_ANIM_SCENE_EXIST(g_CurrentAnimScene))
					{
						Notifications::Show("Animation", "An animation scene is already running. Please stop it first.", NotificationType::Warning);
						return;
					}

					g_CurrentAnimScene = 0; // Reset in case previous scene was invalid

					if (g_SelectedAnimationType == AnimationType::PROPOSE)
					{
						Ped malePed   = GetPedFromActorDef(g_ProposeMaleOverride, true);
						Ped femalePed = GetPedFromActorDef(g_ProposeFemaleOverride, false);

						if (!malePed.IsValid() || !femalePed.IsValid())
						{
							Notifications::Show("Animation Error", "One or both selected actors are invalid.", NotificationType::Error);
							return;
						}

						if (malePed == femalePed)
						{
							Notifications::Show("Animation Error", "Actors cannot be the same person.", NotificationType::Error);
							return;
						}

						const char* animDict = "script_re@proposal@accept";
						const char* sceneName = "Proposal"; // Can be anything descriptive

						while (!STREAMING::HAS_ANIM_DICT_LOADED(animDict))
						{
							STREAMING::REQUEST_ANIM_DICT(animDict);
							ScriptMgr::Yield();
						}

						g_CurrentAnimScene = ANIMSCENE::_CREATE_ANIM_SCENE(animDict, 0, sceneName, FALSE, TRUE);

						if (!ANIMSCENE::DOES_ANIM_SCENE_EXIST(g_CurrentAnimScene))
						{
							Notifications::Show("Animation Error", "Failed to create animation scene.", NotificationType::Error);
							g_CurrentAnimScene = 0;
							return;
						}

						ANIMSCENE::SET_ANIM_SCENE_ENTITY(g_CurrentAnimScene, "male", malePed.GetHandle(), 0);
						ANIMSCENE::SET_ANIM_SCENE_ENTITY(g_CurrentAnimScene, "female", femalePed.GetHandle(), 0);

						ANIMSCENE::LOAD_ANIM_SCENE(g_CurrentAnimScene);

						int timeout = 200; // 200 ticks = ~10 seconds
						while (!ANIMSCENE::IS_ANIM_SCENE_LOADED(g_CurrentAnimScene, true, false) && --timeout > 0)
						{
							ScriptMgr::Yield();
						}

						if (timeout <= 0)
						{
							Notifications::Show("Animation Error", "Animation scene failed to load.", NotificationType::Error);
							ANIMSCENE::_DELETE_ANIM_SCENE(g_CurrentAnimScene);
							g_CurrentAnimScene = 0;
							return;
						}

						ANIMSCENE::START_ANIM_SCENE(g_CurrentAnimScene);
						Notifications::Show("Animation", "Proposal animation started.", NotificationType::Success);
					}
					else if (g_SelectedAnimationType == AnimationType::SEX)
					{
						// TODO: Implement Sex Animation
						Notifications::Show("Animation", "Sex animation not yet implemented.", NotificationType::Warning);
					}
					else if (g_SelectedAnimationType == AnimationType::BATHE)
					{
						// TODO: Implement Bathe Animation
						Notifications::Show("Animation", "Bathe animation not yet implemented.", NotificationType::Warning);
					}
				});
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				FiberPool::Push([] {
					if (g_CurrentAnimScene != 0 && ANIMSCENE::DOES_ANIM_SCENE_EXIST(g_CurrentAnimScene))
					{
						ANIMSCENE::_DELETE_ANIM_SCENE(g_CurrentAnimScene);
						g_CurrentAnimScene = 0;
						Notifications::Show("Animation", "Animation scene stopped.", NotificationType::Info);
					}
					else
					{
						Notifications::Show("Animation", "No animation scene is currently running.", NotificationType::Warning);
					}
				});
			}
		}));
		menu->AddItem(animations);

		return menu;
	}
}
