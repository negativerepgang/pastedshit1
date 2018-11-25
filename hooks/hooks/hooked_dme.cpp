#include "..\hooks.hpp"
#include "..\..\cheats\lagcompensation\lagcompensation.h"

using DrawModelExecute_t = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);

void __stdcall hooks::hooked_dme(IMatRenderContext * ctx, const DrawModelState_t & state, const ModelRenderInfo_t & info, matrix3x4_t * bone_to_world) {
	static auto original_fn = modelrender_hook->get_func_address< DrawModelExecute_t >(21);

	auto btchams = [&](player_t * e) -> bool {
		auto & player = lagcompensation::get().players[e->EntIndex()];
		auto track = player.get_valid_track();

		if (!player.m_e || track.empty())
			return false;

		for (int i = track.size() - 1; i >= 0; i--) {
			auto & record = track.at(i);

			original_fn(g_csgo.m_modelrender(), ctx, state, info, record.m_bone_matrix);

			return true;
		}

		return false;
	};

	bool isplayer = strstr(info.pModel->name, "models/player") != nullptr;
	auto model_entity = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(info.entity_index));

	//auto sleeve_transparency = g_cfg.esp.sleeve_transparency / 100.f;

	if (strstr(info.pModel->name, "v_sleeve"))
		g_csgo.m_renderview()->SetBlend(0.3);

	if (g_cfg.esp.local_chams && isplayer && model_entity->m_iTeamNum() == g_ctx.m_local->m_iTeamNum() && model_entity == g_ctx.m_local) {
		g_csgo.m_renderview()->SetBlend(g_cfg.esp.local_chams_alpha * 0.01f);


	}
	/*
	
	if (strstr(info.pModel->name, "arms") &&  g_cfg.esp.local_chams)
	{

		float clr[4] = {
			g_cfg.esp.local_chams_color[0] / 255.f,
			g_cfg.esp.local_chams_color[1] / 255.f,
			g_cfg.esp.local_chams_color[2] / 255.f,
			g_cfg.esp.local_chams_alpha / 255.f
		};
		IMaterial* Hands = g_csgo.m_materialsystem()->FindMaterial(info.pModel->name, "Model textures");
		Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
		g_csgo.m_modelrender()->ForcedMaterialOverride(Hands);
		g_csgo.m_renderview()->SetColorModulation(clr);


	}*/

	original_fn(g_csgo.m_modelrender(), ctx, state, info, bone_to_world);
	if (g_ctx.backtrack_chams && model_entity->valid(true)) {
		if (btchams(model_entity))
			return;
	}
}