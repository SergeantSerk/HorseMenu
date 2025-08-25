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

	constexpr auto oneshot_emote_names = std::to_array({"chicken"_J, "cuckoo"_J, "facepalm"_J, "fiddlehead"_J, "flyingkiss"_J, "fingerslinger"_J, "gorillachest"_J, "howl"_J, "hushyourmouth"_J, "pointlaugh"_J, "scheme"_J, "shrug"_J, "shuffle"_J, "spooky"_J, "tada"_J, "wagfinger"_J, "warcry"_J});

	constexpr auto repeating_emote_names = std::to_array({"cower"_J, "sittingupsleeping"_J, "sittingupsleepy"_J});

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

	// Helper function to request control of a ped
	bool RequestControlOfEntity(int entity, int maxAttempts = 40)
	{
		if (!ENTITY::DOES_ENTITY_EXIST(entity))
			return false;

		int attempts = 0;
		while (!NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(entity) && attempts < maxAttempts)
		{
			NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(entity);
			ScriptMgr::Yield(50ms);
			attempts++;
		}
		if (!NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(entity)) {
			Notifications::Show("Animate", std::format("Failed to get control after {} attempts.", attempts), NotificationType::Error);
		}
		return NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(entity);
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
				RenderActorDef(g_SexFemaleOverride, "Female");
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
				if (g_SelectedAnimationType == AnimationType::PROPOSE)
				{
					// Get actor peds
					int malePed = 0;
					int femalePed = 0;

					// Helper lambda to resolve ActorDefinition to ped
					auto getPedFromDef = [](ActorDefinition& def) -> int {
						switch (def.Type)
						{
							case ActorOverrideType::SELF:
								return Self::GetPed().GetHandle();
							case ActorOverrideType::PLAYER:
								if (def.OverridePlayer.IsValid())
									return def.OverridePlayer.GetPed().GetHandle();
								break;
							default:
								break;
						}
						return 0;
					};

					malePed = getPedFromDef(g_ProposeMaleOverride);
					femalePed = getPedFromDef(g_SexFemaleOverride);

					if (!malePed || !ENTITY::DOES_ENTITY_EXIST(malePed))
					{
						Notifications::Show("Animate", "Invalid male actor.", NotificationType::Error);
						return;
					}
					if (!femalePed || !ENTITY::DOES_ENTITY_EXIST(femalePed))
					{
						Notifications::Show("Animate", "Invalid female actor.", NotificationType::Error);
						return;
					}

					// Clear tasks before starting animation
					ClearPedTasks(malePed);
					ClearPedTasks(femalePed);

					auto animSceneName = "script_re@proposal@accept";
					const char* maleEntityName = "Sean";
					const char* femaleEntityName = "Karen";
					const char* maleAnim = "sean_action";
					const char* femaleAnim = "karen_action";

					Notifications::Show("Animate", "Preparing animation scene...", NotificationType::Info);

					if (!RequestControlOfEntity(malePed)) {
						Notifications::Show("Animate", "Failed to get control of male actor.", NotificationType::Error);
						return;
					}

					if (!ENTITY::DOES_ENTITY_EXIST(femalePed)) {
						Notifications::Show("Animate", "Female ped does not exist.", NotificationType::Error);
						return;
					}
					if (!NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(femalePed)) {
						Notifications::Show("Animate", "Female ped is not networked.", NotificationType::Error);
						return;
					}
					if (!RequestControlOfEntity(femalePed)) {
						Notifications::Show("Animate", "Failed to get control of female actor.", NotificationType::Error);
						return;
					}

					FiberPool::Push([=] {
						// Create the animation scene
						int animScene = ANIMSCENE::_CREATE_ANIM_SCENE(animSceneName, 0, "", false, false);
						if (!ANIMSCENE::DOES_ANIM_SCENE_EXIST(animScene)) {
							Notifications::Show("Animate", "Failed to create anim scene.", NotificationType::Error);
							return;
						}
						Notifications::Show("Animate", "Anim scene created.", NotificationType::Info);

						// Load the animation scene
						ANIMSCENE::LOAD_ANIM_SCENE(animScene);
						int timeout = 0;
						while (!ANIMSCENE::IS_ANIM_SCENE_LOADED(animScene, false, false) && timeout < 100) {
							ScriptMgr::Yield(50ms);
							timeout++;
						}
						if (!ANIMSCENE::IS_ANIM_SCENE_LOADED(animScene, false, false)) {
							Notifications::Show("Animate", "Failed to load anim scene.", NotificationType::Error);
							return;
						}
						Notifications::Show("Animate", "Anim scene loaded.", NotificationType::Info);

						// Set the entities for the scene
						ANIMSCENE::SET_ANIM_SCENE_ENTITY(animScene, maleEntityName, malePed, 0);
						ANIMSCENE::SET_ANIM_SCENE_ENTITY(animScene, femaleEntityName, femalePed, 0);
						Notifications::Show("Animate", "Entities set in anim scene.", NotificationType::Info);

						// Start the animation scene
						ANIMSCENE::START_ANIM_SCENE(animScene);
						Notifications::Show("Animate", "Propose animation scene started.", NotificationType::Success);
					});
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
				// TODO: Implement stop animation
				Notifications::Show("Animate", "Stop animation not yet implemented.", NotificationType::Error);
			}
		}));
		menu->AddItem(animations);

		return menu;
	}
}
