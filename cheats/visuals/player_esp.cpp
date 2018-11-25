#include "player_esp.h"

#include "..\lagcompensation\lagcompensation.h"

void playeresp::paint_traverse() {
	if (!g_ctx.available() || !g_cfg.player.enable)
		return;

	for (int i = 1; i < g_csgo.m_globals()->m_maxclients; i++) {
		auto e = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(i));

		if (!e->valid(true, !g_cfg.player.enable))
			continue;

		if (g_cfg.player.enable)
		{
			int idx = e->EntIndex();
			float in = (1.f / 0.2f) * g_csgo.m_globals()->m_frametime;
			float out = (1.f / 2.f) * g_csgo.m_globals()->m_frametime;

			if (!e->IsDormant()) {
				if (esp_alpha_fade[idx] < 1.f)
					esp_alpha_fade[idx] += in;
			}
			else {
				if (esp_alpha_fade[idx] > 0.f)
					esp_alpha_fade[idx] -= out;
			}
			esp_alpha_fade[idx] = (esp_alpha_fade[idx] > 1.f ? 1.f : esp_alpha_fade[idx] < 0.f ? 0.f : esp_alpha_fade[idx]);
		}


		Box box; if (util::get_bbox(e, box)) {
			draw_box(e, box);
			draw_name(e, box);
			draw_health(e, box);
			draw_flags(e, box);
			draw_weapon(e, box);
			draw_lbybar(e, box);

			if (g_cfg.player.skeleton[SKELETON_NORMAL].enabled) {
				Color color = e->IsDormant() ? Color(130, 130, 130, 130) : Color::White;
				color.SetAlpha(g_cfg.player.alpha  * esp_alpha_fade[e->EntIndex()]);

				matrix3x4_t matrix[128];

				if (e->SetupBones(matrix, 128, 0x100, 0.f))
					draw_skeleton(e, color, matrix);
			}
		}
	}
}

void playeresp::draw_box(player_t* m_entity, Box box) {
	if (!g_cfg.player.box)
		return;

	int
		alpha = g_cfg.player.alpha * esp_alpha_fade[m_entity->EntIndex()],
		outline_alpha = alpha * 0.6f;

	Color
		outline_color{ 0, 0, 0, outline_alpha },
		color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.box_color;

	color.SetAlpha(alpha);

	render::get().rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, outline_color);
	render::get().rect(box.x, box.y, box.w, box.h, color);
	render::get().rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, outline_color);
}

void playeresp::draw_health(player_t* m_entity, Box box) {
	if (!g_cfg.player.health)
		return;

	int player_health = m_entity->m_iHealth() > 100 ? 100 : m_entity->m_iHealth();
	if (!player_health)
		return;

	int
		alpha = g_cfg.player.alpha * esp_alpha_fade[m_entity->EntIndex()],
		outline_alpha = alpha * 0.6f,
		battery_alpha = alpha * 0.9f,
		inner_back_alpha = alpha * 0.6f;
	Color
		outline_color{ 0, 0, 0, outline_alpha },
		inner_back_color{ 0, 0, 0, inner_back_alpha },
		battery_color{ 0, 0, 0, inner_back_alpha },
		text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha),
		color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : Color(153, min(255, m_entity->m_iHealth() * 225 / 100), 0, alpha);

	color.SetAlpha(alpha);

	render::get().rect(box.x - 6, box.y, 4, box.h, outline_color);

	Box n_box = {
		box.x - 5,
		box.y + 1,
		2,
		box.h - 2
	};

	float
		fl_height = n_box.h / 10.f;

	int
		bar_height = player_health * n_box.h / 100.0f,
		offset = n_box.h - bar_height;

	render::get().rect_filled(n_box.x, n_box.y, 2, n_box.h, inner_back_color);
	render::get().rect_filled(n_box.x, n_box.y + offset, 2, bar_height, color);


	if (player_health < 100) {
		render::get().text(fonts[ESPHEALTH],
			n_box.x + 1, n_box.y + offset,
			text_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, std::to_string(player_health).c_str()
		);
	}
}


void playeresp::draw_name(player_t* m_entity, Box box) {
	static auto sanitize = [](char * name) -> std::string {
		name[127] = '\0';

		std::string tmp(name);

		if (tmp.length() > 20) {
			tmp.erase(20, (tmp.length() - 20));
			tmp.append("...");
		}

		return tmp;
	};

	if (!g_cfg.player.name)
		return;

	player_info_t player_info;
	if (g_csgo.m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info)) {
		std::string name = sanitize(player_info.szName);

		auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.name_color;
		color.SetAlpha(g_cfg.player.alpha * esp_alpha_fade[m_entity->EntIndex()]);

		std::transform(name.begin(), name.end(), name.begin(), tolower);

		render::get().text(fonts[ESPNAME], box.x + (box.w / 2), box.y - 13, color, HFONT_CENTERED_X, name.c_str());
	}
}

void playeresp::draw_weapon(player_t* m_entity, Box box) {
	player_info_t player_info;
	g_csgo.m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info);

	if (!g_cfg.player.weapon)
		return;

	int bottom_pos = 0;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	int _y;

	if (!weapon)
		return;

	bool drawing_ammo_bar = g_cfg.player.ammo == 1;
	bool drawing_lby_bar = g_cfg.player.lby;

	if (weapon->is_non_aim() || !g_cfg.player.lby || player_info.fakeplayer || !g_cfg.ragebot.antiaim_correction)
		drawing_lby_bar = false;


	if (weapon->is_non_aim())
		drawing_ammo_bar = false;

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.weapon_color;
	color.SetAlpha(g_cfg.player.alpha * esp_alpha_fade[m_entity->EntIndex()]);

	render::get().text(fonts[ESPHEALTH], box.x + (box.w / 2), box.y + box.h, color, HFONT_CENTERED_X, weapon->get_name());
}

void playeresp::draw_flags(player_t * e, Box box) {
	int _x = box.x + box.w + 1, _y = box.y;
	player_info_t player_info;
	g_csgo.m_engine()->GetPlayerInfo(e->EntIndex(), &player_info);

	auto weapon = e->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto draw_flag = [&](Color color, const char * text, ...) -> void {
		render::get().text(fonts[ESPHEALTH], _x, _y, color, HFONT_CENTERED_NONE, text);

		_y += 9;
	};


	auto color = Color(255, 255, 255);
	color.SetAlpha(g_cfg.player.alpha * esp_alpha_fade[e->EntIndex()]);



	/*if ( g_cfg.player.flags[ FLAGS_RESOLVERMODE ].enabled ) {
		auto & player = lagcompensation::get( ).players[ e->EntIndex( ) ];

		const char * resolver_modes[ ] = {
			"brute",
			"lowerbody",
			"update",
			"freestand"
		};

		auto color = Color( 255, 155, 75 );
		color.SetAlpha( g_cfg.player.alpha * esp_alpha_fade[ e->EntIndex( ) ] );

		if ( player.m_e && player.m_resolver )
			draw_flag( color, resolver_modes[ player.m_resolver->m_mode ] );
	}*/
	if (g_cfg.player.flags[FLAGS_BALANCE].enabled) {
		int account = e->m_iAccount();

		auto color = Color(128, 163, 85);
		color.SetAlpha(g_cfg.player.alpha * esp_alpha_fade[e->EntIndex()]);
		util::clamp_value(account, 0, 65000);
		draw_flag(color, "$%i", account);

	}
	if (g_cfg.player.flags[FLAGS_ARMOR].enabled) {
		bool
			kevlar = e->m_ArmorValue() > 0,
			helmet = e->m_bHasHelmet();

		std::string text;
		if (helmet) text += "h";
		if (kevlar) text += "k";

		auto color = Color::White;
		color.SetAlpha(g_cfg.player.alpha  * esp_alpha_fade[e->EntIndex()]);

		if (helmet || kevlar)
			draw_flag(color, text.c_str());
	}

	if (g_cfg.player.flags[FLAGS_SCOPED].enabled) {
		auto color = Color(150, 150, 220);
		color.SetAlpha(g_cfg.player.alpha  * esp_alpha_fade[e->EntIndex()]);

		if (e->m_bIsScoped())
			draw_flag(color, "zoom");
	}





}

void playeresp::draw_skeleton(player_t * e, Color color, matrix3x4_t matrix[128]) {
	auto get_bone_position = [&](int bone) -> vec3_t {
		return vec3_t(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	auto hdr = g_csgo.m_modelinfo()->GetStudiomodel(e->GetModel());

	if (!hdr)
		return;

	constexpr int chest_bone = 6;
	vec3_t
		upper_direction = get_bone_position(chest_bone + 1) - get_bone_position(chest_bone),
		breast_bone = get_bone_position(chest_bone) + upper_direction / 2;
	vec3_t child, parent;

	for (int i = 0; i < hdr->numbones; i++) {
		mstudiobone_t * bone = hdr->pBone(i);

		if (bone && (bone->flags & BONE_USED_BY_HITBOX) && (bone->parent != -1)) {
			child = vec3_t(matrix[i][0][3], matrix[i][1][3], matrix[i][2][3]);
			parent = vec3_t(matrix[bone->parent][0][3], matrix[bone->parent][1][3], matrix[bone->parent][2][3]);

			vec3_t
				delta_child = child - breast_bone,
				delta_parent = parent - breast_bone;

			if ((delta_parent.Length() < 9 && delta_child.Length() < 9))
				parent = breast_bone;

			if (i == chest_bone - 1)
				child = breast_bone;

			if (abs(delta_child.z) < 5 && (delta_parent.Length() < 5 && delta_child.Length() < 5) || i == chest_bone)
				continue;

			vec3_t schild, sparent;
			if (math::world_to_screen(child, schild) && math::world_to_screen(parent, sparent))
				render::get().line(schild.x, schild.y, sparent.x, sparent.y, color);
		}
	}
}

void playeresp::draw_lbybar(player_t* m_entity, Box box) {
	player_info_t player_info;
	g_csgo.m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info);

	if (player_info.fakeplayer || !g_cfg.player.lby || !g_cfg.ragebot.antiaim_correction)
		return;

	int
		alpha = g_cfg.player.alpha * esp_alpha_fade[m_entity->EntIndex()],
		outline_alpha = alpha * 0.7f,
		inner_back_alpha = alpha * 0.6f;

	Color
		outline_color{ 0, 0, 0, outline_alpha },
		inner_back_color{ 0, 0, 0, inner_back_alpha },
		text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha),
		color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.lbybar_color;

	color.SetAlpha(alpha);

	bool drawing_ammo_bar = g_cfg.player.ammo == 1;

	Box n_box = {
		box.x + 1,
		box.y + box.h + 8,
		box.w - 1,
		2
	};

	if (!drawing_ammo_bar)
		n_box.y -= 5;

	float cycle = g_ctx.m_globals.next_update[m_entity->EntIndex()] - m_entity->m_flSimulationTime();
	float bar_width = (((cycle * (n_box.w / 1.1f))));

	util::clamp_value<float>(g_ctx.m_globals.add_time[m_entity->EntIndex()], 0.f, 1.1f);

	render::get().rect_filled(n_box.x - 1, n_box.y - 1, box.w + 2, 4, inner_back_color);
	render::get().rect_filled(n_box.x, n_box.y, bar_width, 2, color);
}

void playeresp::pov_arrow() {
	if (!g_cfg.player.out_pov_arrow)
		return;

	static float alpha = 0.f;
	static int plus_or_minus = false;
	if (alpha <= 0.f || alpha >= 255.f)
		plus_or_minus = !plus_or_minus;

	alpha += plus_or_minus ? (255.f / 0.5f * g_csgo.m_globals()->m_frametime) : -(255.f / 0.5f * g_csgo.m_globals()->m_frametime);
	alpha = math::clamp< float >(alpha, 0.f, 255.f);

	int screen_width, screen_height;
	g_csgo.m_engine()->GetScreenSize(screen_width, screen_height);

	vec3_t client_viewangles;
	g_csgo.m_engine()->GetViewAngles(client_viewangles);

	const float radius = g_cfg.player.out_pov_distance;

	auto draw_arrow = [&](float rotation, Color color, player_t* e, vec3_t poopvec) -> void {
		Vector newangle = math::calculate_angle(Vector(g_ctx.m_local->abs_origin().x, g_ctx.m_local->abs_origin().y, 0), Vector(e->abs_origin().x, e->abs_origin().y, 0));
		math::angle_vectors(Vector(0, 270, 0) - newangle + Vector(0, client_viewangles.y, 0), poopvec);
		auto circlevec = Vector(screen_width / 2, screen_height / 2, 0) + (poopvec * radius);
		render::get().circle_filled(circlevec.x, circlevec.y, 4, 6, color);

	};

	for (auto i = 1; i <= g_csgo.m_globals()->m_maxclients; i++) {
		auto e = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(i));

		if (!e->valid(true))
			continue;

		vec3_t screen_point;
		g_csgo.m_debugoverlay()->ScreenPosition(e->abs_origin(), screen_point);

		if (screen_point.x < 0 || screen_point.y < 0 || screen_point.x > screen_width || screen_point.y > screen_height) {
			auto angle = math::calculate_angle(g_ctx.m_local->get_eye_pos(), e->abs_origin());
			const auto angle_yaw_rad = DEG2RAD(client_viewangles.y - angle.y);

			Color color = g_cfg.player.out_pov_arrow_color;
			color.SetAlpha(alpha);

			draw_arrow(angle_yaw_rad, color, e, screen_point);
		}
	}
}
