#include "Animate.hpp"

#include "core/frontend/Notifications.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/Players.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Vehicle.hpp"

namespace YimMenu::Submenus
{
	constexpr auto g_AnimationTypeStrs = std::to_array({"Propose", "Sex"});

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

	enum class AnimationType
	{
		PROPOSE,
		SEX
	};

	static AnimationType g_SelectedAnimationType = AnimationType::PROPOSE;

	static ActorDefinition g_ProposeMaleOverride;
	static ActorDefinition g_ProposeFemaleOverride;

	static ActorDefinition g_SexMaleOverride;
	static ActorDefinition g_SexFemaleOverride;

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

		auto emotes = std::make_shared<Group>("Emotes");
		emotes->AddItem(std::make_shared<PlayerCommandItem>("chicken"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("chicken1"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("chicken2"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("cuckoo"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("facepalm"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("fiddlehead"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("flyingkiss"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("fingerslinger"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("gorillachest"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("howl"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("hushyourmouth"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("pointlaugh"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("scheme"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("shrug"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("shuffle"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("spooky"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("tada"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("wagfinger"_J));
		emotes->AddItem(std::make_shared<PlayerCommandItem>("warcry"_J));
		menu->AddItem(emotes);

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

			if (ImGui::Button("Animate"))
			{
				if (g_SelectedAnimationType == AnimationType::PROPOSE)
				{

				}
				else if (g_SelectedAnimationType == AnimationType::SEX)
				{

				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{

			}
		}));
		menu->AddItem(animations);

		return menu;
	}
}