#include "Animate.hpp"

#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/rdr/Natives.hpp"
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
				if (g_SelectedAnimationType == AnimationType::PROPOSE) {}
				else if (g_SelectedAnimationType == AnimationType::SEX) {}
				else if (g_SelectedAnimationType == AnimationType::BATHE) {}
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {}
		}));
		menu->AddItem(animations);

		return menu;
	}
}