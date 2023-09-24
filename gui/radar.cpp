#include "pch-il2cpp.h"
#include "radar.hpp"
#include "DirectX.h"
#include "utility.h"
#include "state.hpp"
#include "gui-helpers.hpp"

namespace Radar {
	ImU32 GetRadarPlayerColor(GameData_PlayerInfo* playerData) {
		app::GameData_PlayerOutfit* outfit = GetPlayerOutfit(playerData);
		if (outfit == NULL) return ImU32(0);

		return ImGui::ColorConvertFloat4ToU32(AmongUsColorToImVec4((GetPlayerColor(outfit->fields.ColorId))));
	}

	ImU32 GetRadarPlayerColorStatus(GameData_PlayerInfo* playerData) {
		if (State.RevealRoles && playerData->fields.Role != nullptr)
			return ImGui::ColorConvertFloat4ToU32(AmongUsColorToImVec4(GetRoleColor(playerData->fields.Role)));
		else if (playerData->fields.IsDead)
			return ImGui::ColorConvertFloat4ToU32(AmongUsColorToImVec4(app::Palette__TypeInfo->static_fields->HalfWhite));
		else
			return ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0));
	}

	void SquareConstraint(ImGuiSizeCallbackData* data)
	{
		data->DesiredSize = ImVec2(data->DesiredSize.x, data->DesiredSize.y);
	}

	void OnClick() {
		if (!(ImGui::IsKeyPressed(VK_SHIFT) || ImGui::IsKeyDown(VK_SHIFT)) && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
			ImVec2 mouse = ImGui::GetMousePos();
			ImVec2 winpos = ImGui::GetWindowPos();
			ImVec2 winsize = ImGui::GetWindowSize();

			if (mouse.x < winpos.x
				|| mouse.x > winpos.x + winsize.x
				|| mouse.y < winpos.y
				|| mouse.y > winpos.y + winsize.y)
				return;

			const auto& map = maps[(size_t)State.mapType];
			float xOffset = getMapXOffsetSkeld(map.x_offset);
			float yOffset = map.y_offset;

			Vector2 target = {
				((mouse.x - winpos.x) / State.dpiScale - xOffset) / map.scale,
				(((mouse.y - winpos.y) / State.dpiScale - yOffset) * -1.F) / map.scale
			};

			State.rpcQueue.push(new RpcSnapTo(target));
		}
		if (State.TeleportEveryone && !(ImGui::IsKeyPressed(VK_SHIFT) || ImGui::IsKeyDown(VK_SHIFT)) && (ImGui::IsKeyPressed(0x12) || ImGui::IsKeyDown(0x12)) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImVec2 mouse = ImGui::GetMousePos();
			ImVec2 winpos = ImGui::GetWindowPos();
			ImVec2 winsize = ImGui::GetWindowSize();

			if (mouse.x < winpos.x
				|| mouse.x > winpos.x + winsize.x
				|| mouse.y < winpos.y
				|| mouse.y > winpos.y + winsize.y)
				return;

			const auto& map = maps[(size_t)State.mapType];
			float xOffset = getMapXOffsetSkeld(map.x_offset);
			float yOffset = map.y_offset;

			Vector2 target = {
				((mouse.x - winpos.x) / State.dpiScale - xOffset) / map.scale,
				(((mouse.y - winpos.y) / State.dpiScale - yOffset) * -1.F) / map.scale
			};

			for (auto player : GetAllPlayerControl()) {
				State.rpcQueue.push(new RpcForceSnapTo(player, target));
			}
		}
	}

	void Init() {
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), SquareConstraint);
		ImGui::SetNextWindowBgAlpha(0.F);
	}

	bool init = false;
	void Render() {
		if (!init)
			Radar::Init();

		const auto& map = maps[(size_t)State.mapType];
		ImGui::SetNextWindowSize(ImVec2((float)map.mapImage.imageWidth * (0.5F + 10.F), (float)map.mapImage.imageHeight * 0.5f + 10.f) * State.dpiScale, ImGuiCond_None);

		if(State.LockRadar)
			ImGui::Begin("Radar", &State.ShowRadar, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
		else
			ImGui::Begin("Radar", &State.ShowRadar, ImGuiWindowFlags_NoDecoration);

		ImVec2 winpos = ImGui::GetWindowPos();

		ImVec4 RadarColor = ImVec4(1.f, 1.f, 1.f, 0.75f);
		if (State.RgbMenuTheme)
			RadarColor = {State.RgbColor.x, State.RgbColor.y, State.RgbColor.z, State.SelectedColor.w};
		else
			RadarColor = State.SelectedColor;
		
		GameOptions options;

		ImGui::Image((void*)map.mapImage.shaderResourceView,
			ImVec2((float)map.mapImage.imageWidth * (options.GetByte(app::ByteOptionNames__Enum::MapId) == 3 ? (-0.5F) : 0.5F), (float)map.mapImage.imageHeight * 0.5F) * State.dpiScale,
			ImVec2(0.0f, 0.0f),
			(/*options.GetByte(app::ByteOptionNames__Enum::MapId) == 3*/false) ? ImVec2(1.0f, 0.0f) : ImVec2(0.0f, 0.0f),
			(/*options.GetByte(app::ByteOptionNames__Enum::MapId) == 3*/false) ? ImVec2(0.0f, 1.0f) : ImVec2(1.0f, 1.0f),
			RadarColor);

		for (auto player : GetAllPlayerControl()) {
			auto playerData = GetPlayerData(player);

			if (!playerData || (!State.ShowRadar_Ghosts && playerData->fields.IsDead))
				continue;

			if (State.RadarDrawIcons)
				drawPlayerIcon(player, winpos, GetRadarPlayerColor(playerData));
			else
				drawPlayerDot(player, winpos, GetRadarPlayerColor(playerData), GetRadarPlayerColorStatus(playerData));
		}

		if (State.ShowRadar_DeadBodies) {
			for (auto deadBody : GetAllDeadBodies()) {
				auto playerData = GetPlayerDataById(deadBody->fields.ParentId);

				if (State.RadarDrawIcons)
					drawDeadPlayerIcon(deadBody, winpos, GetRadarPlayerColor(playerData));
				else
					drawDeadPlayerDot(deadBody, winpos, GetRadarPlayerColor(playerData));
			}
		}

		if (State.ShowRadar_RightClickTP)
			OnClick();

		ImGui::End();
	}
}