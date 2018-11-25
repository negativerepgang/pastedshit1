#include "other_esp.h"
#include "..\autowall\autowall.h"
#include "..\ragebot\antiaim.h"
#include <chrono>
#include "../../hooks/hooks.hpp"

std::vector<impact_info> impacts;
std::vector<hitmarker_info> hitmarkers;

void otheresp::antiaim_indicator() {
	auto local_player = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(g_csgo.m_engine()->GetLocalPlayer()));

	static auto degrees_to_radians = [](float deg) -> float { return deg * (DirectX::XM_PI / 180.f); };

	auto rotated_position = [](Vector start, const float rotation, const float distance) -> Vector {
		const auto rad = degrees_to_radians(rotation);
		start.x += cosf(rad) * distance;
		start.y += sinf(rad) * distance;

		return start;
	};

	if (!g_ctx.available() || !local_player)
		return;

	static const auto real_color = Color(0, 255, 0, 255);
	static const auto fake_color = Color(255, 0, 0, 255);
	static const auto lby_color = Color(0, 0, 255, 255);

	if (!local_player->is_alive())
		return;

	if (g_cfg.esp.antiaim_indicator) {
		auto client_viewangles = Vector();
		auto
			screen_width = render::get().viewport().right,
			screen_height = render::get().viewport().bottom;

		g_csgo.m_engine()->GetViewAngles(client_viewangles);

		constexpr auto radius = 80.f;

		const auto screen_center = Vector2D(screen_width / 2.f, screen_height / 2.f);
		const auto real_rot = degrees_to_radians(client_viewangles.y - g_ctx.m_globals.real_angles.y - 90);
		const auto fake_rot = degrees_to_radians(client_viewangles.y - g_ctx.m_globals.fake_angles.y - 90);
		const auto lby_rot = degrees_to_radians(client_viewangles.y - g_ctx.m_local->m_flLowerBodyYawTarget() - 90);

		auto draw_arrow = [&](float rot, Color color) -> void {
			auto pos_one = Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius);
			auto pos_two = Vector2D(screen_center.x + cosf(rot + degrees_to_radians(8)) * (radius - 13.f),
				screen_center.y + sinf(rot + degrees_to_radians(8)) * (radius - 13.f));
			auto pos_three = Vector2D(screen_center.x + cosf(rot - degrees_to_radians(8)) * (radius - 13.f),
				screen_center.y + sinf(rot - degrees_to_radians(8)) * (radius - 13.f));

			render::get().triangle(
				Vector2D(pos_one.x, pos_one.y),
				Vector2D(pos_two.x, pos_two.y),
				Vector2D(pos_three.x, pos_three.y),
				color);
		};

		draw_arrow(real_rot, real_color);
		draw_arrow(fake_rot, fake_color);
		draw_arrow(lby_rot, lby_color);
	}
}

void otheresp::penetration_reticle() {
	auto local_player = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(g_csgo.m_engine()->GetLocalPlayer()));

	if (!local_player->is_alive())
		return;

	auto draw_xhair = [](float size, Color color) -> void {
		int x_1, x_2, y_1, y_2;

		int
			screen_w = render::get().viewport().right / 2,
			screen_h = render::get().viewport().bottom / 2;

		x_1 = screen_w - (size / 4);
		x_2 = screen_w + (size / 4) + 1;

		y_1 = screen_h - (size / 4);
		y_2 = screen_h + (size / 4) + 1;

		render::get().rect_filled(screen_w - 1, screen_h - 1, 3, 3, color);
	};

	if (!g_cfg.esp.penetration_reticle || !local_player->is_alive())
		return;

	auto weapon = local_player->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	if (weapon->is_non_aim())
		return;

	auto weapon_info = weapon->get_csweapon_info();
	if (!weapon_info)
		return;

	vec3_t view_angles;
	g_csgo.m_engine()->GetViewAngles(view_angles);

	vec3_t direction;
	math::angle_vectors(view_angles, direction);

	vec3_t
		start = local_player->get_eye_pos(),
		end = start + (direction * 8192.f);

	Color color = Color(255, 0, 0);
	if (autowall::get().calculate_return_info(start, end, local_player).m_did_penetrate_wall)
		color = Color(163, 225, 21);

	draw_xhair(2, color);

}
#define M_PI 3.14159265358979323846

void otheresp::draw_percent_circle(int x, int y, int size, float amount_full, Color fill)
{
	int texture = g_csgo.m_surface()->CreateNewTextureID(true);
	g_csgo.m_surface()->DrawSetTexture(texture);
	g_csgo.m_surface()->DrawSetColor(fill);

	Vertex_t vertexes[100];
	for (int i = 0; i < 100; i++) {
		float angle = ((float)i / -100) * (M_PI * (2 * amount_full));
		vertexes[i].Init(Vector2D(x + (size * sin(angle)), y + (size * cos(angle))));
	}

	g_csgo.m_surface()->DrawTexturedPolygon(100, vertexes, true);
}


void otheresp::lby_indicator() {
	auto local_player = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(g_csgo.m_engine()->GetLocalPlayer()));

	int screen_width, screen_height;
	g_csgo.m_engine()->GetScreenSize(screen_width, screen_height);

	float breaking_lby_fraction = fabs(math::normalize_yaw(g_ctx.m_globals.real_angles.y - local_player->m_flLowerBodyYawTarget())) / 180.f;
	color colour((1.f - breaking_lby_fraction) * 255.f, breaking_lby_fraction * 255.f, 0);
	Color clr(colour.RGBA[0], colour.RGBA[1], colour.RGBA[2]);
	if (local_player->m_fFlags() & FL_ONGROUND && local_player->m_vecVelocity().Length() < 0.1f) {


		if (local_player->m_bGunGameImmunity() || local_player->m_fFlags() & FL_FROZEN)
			return;

		static float add = 0.000f;
		if (add < 1)
			add += 0.01;

		if (add >= 1)
			add = 0.000f;
		draw_percent_circle(70, screen_height / 2 + 17.5, 8, add * 2, Color{ 0, 255, 0, 255 });


	}
	auto fps = static_cast<int>(1.f / g_csgo.m_globals()->m_frametime);

	render::get().text(fonts[INDICATORFONT], 5, screen_height / 2, clr, HFONT_OFF_SCREEN, "LBY");
	render::get().text(fonts[INDICATORFONT], 5, screen_height / 2 + 20, Color(0, 255, 0, 255), HFONT_OFF_SCREEN, "FPS: ");
	auto name_sizeFPS = render::get().text_width(fonts[INDICATORFONTSMALL], "FPS: ");
	render::get().text(fonts[INDICATORFONTSMALL], name_sizeFPS + 15, screen_height / 2 + 28, Color(0, 255, 0, 255), HFONT_OFF_SCREEN, "%i fps", fps);
	render::get().text(fonts[INDICATORFONT], 5, screen_height / 2 + 42, Color{ 50, 75, 255, 255 }, HFONT_OFF_SCREEN, "Freestanding");
	render::get().text(fonts[INDICATORFONT], 5, screen_height / 2 + 62, Color{ 255, 255, 255, 255 }, HFONT_OFF_SCREEN, "Resolving");
	INetChannelInfo *nci = g_csgo.m_engine()->GetNetChannelInfo();
		int ping = (nci->GetAvgLatency(FLOW_INCOMING) * 1000.0f);	
	render::get().text(fonts[INDICATORFONT], 5, screen_height / 2 - 40, Color(0, 255, 0, 255), HFONT_OFF_SCREEN, "PING: ");
	auto name_sizeFPS2 = render::get().text_width(fonts[INDICATORFONT], "PING: ");
	render::get().text(fonts[INDICATORFONTSMALL], name_sizeFPS + 35, screen_height / 2 - 32, Color(0, 255, 0, 255), HFONT_OFF_SCREEN, "%i ms", ping);

		if (&g_cfg.ragebot.lagcomp) {
		if (local_player->m_vecVelocity().Length() > 2048 / g_cfg.antiaim.fakelag_amount)
		{
			render::get().text(fonts[INDICATORFONT], 5, screen_height / 2 - 20, Color{ 255, 0, 0, 255 }, HFONT_OFF_SCREEN, "LC");
		}
		else
		{
			render::get().text(fonts[INDICATORFONT], 5, screen_height / 2 - 20, Color{ 0, 255, 0, 255 }, HFONT_OFF_SCREEN, "LC");

		}
	}
}
player_t* get_player(int userid) {
	int i = g_csgo.m_engine()->GetPlayerForUserID(userid);
	return static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(i));
}

void otheresp::hitmarker_event(IGameEvent * event) {

	auto local_player = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(g_csgo.m_engine()->GetLocalPlayer()));

	if (!strcmp(event->GetName(), "player_hurt")) {

		auto attacker = event->GetInt("attacker");
		auto victim = get_player(event->GetInt("userid"));

		if (g_csgo.m_engine()->GetPlayerForUserID(attacker) == g_csgo.m_engine()->GetLocalPlayer())

		hurt_time = g_csgo.m_globals()->m_curtime;
		impact_info best_impact;
		float best_impact_distance = -1;
		float time = g_csgo.m_globals()->m_curtime;

		std::vector<impact_info>::iterator iter;
		for (iter = impacts.begin(); iter != impacts.end();) {
			if (time > iter->time + 25) {
				impacts.erase(iter);
				continue;
			}
			vec3_t position = vec3_t(iter->x, iter->y, iter->z);
			vec3_t enemy_pos = victim->m_vecOrigin();
			float distance = position.DistTo(enemy_pos);
			if (distance < best_impact_distance || best_impact_distance == -1) {
				best_impact_distance = distance;
				best_impact = *iter;
			}
			++iter;
		}

		if (best_impact_distance == -1)
			return;


		hitmarker_info info;
		info.impact = best_impact;
		info.alpha = 255;
		hitmarkers.push_back(info);

	}

	if (strstr(event->GetName(), "bullet_impact"))
	{
		auto entity = get_player(event->GetInt("userid"));
		if (!entity || entity != local_player)
			return;

		impact_info info;
		info.x = event->GetFloat("x");
		info.y = event->GetFloat("y");
		info.z = event->GetFloat("z");

		info.time = g_csgo.m_globals()->m_curtime;

		impacts.push_back(info);
	}

}



void otheresp::hitmarker_paint() {
	auto linesize = 4;
	int center_width;
	int center_height;


	if (!g_csgo.m_engine()->IsInGame() || !g_csgo.m_engine()->IsConnected())
		hurt_time = 0.f;

		g_csgo.m_engine()->GetScreenSize(center_width, center_height);
		center_width / 2, center_height / 2;

	
			auto line_size = 10;

			render::get().line(center_width - line_size, center_height - line_size, center_width - (line_size / 2), center_height - (line_size / 2), Color(255, 255, 255, 220));
			render::get().line(center_width - line_size, center_height + line_size, center_width - (line_size / 2), center_height + (line_size / 2), Color(255, 255, 255, 220));
			render::get().line(center_width + line_size, center_height + line_size, center_width + (line_size / 2), center_height + (line_size / 2), Color(255, 255, 255, 220));
			render::get().line(center_width + line_size, center_height - line_size, center_width + (line_size / 2), center_height - (line_size / 2), Color(255, 255, 255, 220));
		
}

void otheresp::bomb_timer() {
	if (!g_cfg.esp.bomb_timer)
		return;

	auto c4_timer = g_csgo.m_cvar()->FindVar("mp_c4timer")->GetFloat();

}

void otheresp::hitmarkerdynamic_paint() {

	float time = g_csgo.m_globals()->m_curtime;

	for (int i = 0; i < hitmarkers.size(); i++) {
		bool expired = time >= hitmarkers.at(i).impact.time + 2.f;

		if (expired)
			hitmarkers.at(i).alpha -= 1;

		if (expired && hitmarkers.at(i).alpha <= 0) {
			hitmarkers.erase(hitmarkers.begin() + i);
			continue;
		}

		Vector pos3D = Vector(hitmarkers.at(i).impact.x, hitmarkers.at(i).impact.y, hitmarkers.at(i).impact.z), pos2D;
		if (!math::world_to_screen(pos3D, pos2D))
			continue;

		auto linesize = 4;

		g_csgo.m_surface()->DrawSetColor(255, 255, 255, hitmarkers.at(i).alpha);
		g_csgo.m_surface()->DrawLine(pos2D.x - linesize, pos2D.y - linesize, pos2D.x - (linesize / 4), pos2D.y - (linesize / 4));
		g_csgo.m_surface()->DrawLine(pos2D.x - linesize, pos2D.y + linesize, pos2D.x - (linesize / 4), pos2D.y + (linesize / 4));
		g_csgo.m_surface()->DrawLine(pos2D.x + linesize, pos2D.y + linesize, pos2D.x + (linesize / 4), pos2D.y + (linesize / 4));
		g_csgo.m_surface()->DrawLine(pos2D.x + linesize, pos2D.y - linesize, pos2D.x + (linesize / 4), pos2D.y - (linesize / 4));
	}
}


void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
	if (s == 0.0f)
	{
		// gray
		out_r = out_g = out_b = v;
		return;
	}

	h = fmodf(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
}

void otheresp::spread_crosshair(IDirect3DDevice9* m_device) {
	if (!g_cfg.esp.show_spread)
		return;

	if (!g_ctx.m_local)
		return;

	if (!g_ctx.m_local->is_alive())
		return;

	if (!g_csgo.m_engine()->IsConnected() && !g_csgo.m_engine()->IsInGame())
		return;

	auto weapon = g_ctx.m_local->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	static float rot = 0.f;

	int w, h;

	g_csgo.m_engine()->GetScreenSize(w, h);

	w /= 2, h /= 2;

	int r, g, b;
	r = g_cfg.esp.show_spread_color.r();
	g = g_cfg.esp.show_spread_color.g();
	b = g_cfg.esp.show_spread_color.b();

	switch (g_cfg.esp.show_spread_type) {
	case 0:
		render::get().circle_dual_colour(w, h, weapon->get_innacuracy() * 500.0f, 0, 1, 50, D3DCOLOR_RGBA(r, g, b, 100), D3DCOLOR_RGBA(0, 0, 0, 0), m_device);
		break;
	case 1:
		render::get().circle_dual_colour(w, h, weapon->get_innacuracy() * 500.0f, rot, 1, 50, m_device);
		break;
	}

	rot += 0.5f;
	if (rot > 360.f)
		rot = 0.f;
}

void otheresp::spread_crosshair() {
	if (g_cfg.esp.show_spread_type != 2)
		return;

	if (!g_ctx.m_local)
		return;

	if (!g_ctx.m_local->is_alive())
		return;

	if (!g_csgo.m_engine()->IsConnected() && !g_csgo.m_engine()->IsInGame())
		return;

	auto weapon = g_ctx.m_local->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (weapon) {
		int screen_w, screen_h;
		g_csgo.m_engine()->GetScreenSize(screen_w, screen_h);
		int cross_x = screen_w / 2, cross_y = screen_h / 2;

		float recoil_step = screen_h / 180;

		cross_x -= (int)(g_ctx.m_local->m_aimPunchAngle().y * recoil_step);
		cross_y += (int)(g_ctx.m_local->m_aimPunchAngle().x * recoil_step);

		weapon->update_accuracy_penality();
		float inaccuracy = weapon->get_innacuracy();
		float spread = weapon->get_spread();

		float cone = inaccuracy * spread;
		cone *= screen_h * 0.7f;
		cone *= 90.f / 180;

		for (int seed{ }; seed < 512; ++seed) {
			math::random_seed(math::random_float(0, 512));

			float	rand_a = math::random_float(0.f, 1.f);
			float	pi_rand_a = math::random_float(0.f, 2.f * DirectX::XM_PI);
			float	rand_b = math::random_float(0.0f, 1.f);
			float	pi_rand_b = math::random_float(0.f, 2.f * DirectX::XM_PI);

			float spread_x = cos(pi_rand_a) * (rand_a * inaccuracy) + cos(pi_rand_b) * (rand_b * spread);
			float spread_y = sin(pi_rand_a) * (rand_a * inaccuracy) + sin(pi_rand_b) * (rand_b * spread);

			float max_x = cos(pi_rand_a) * cone + cos(pi_rand_b) * cone;
			float max_y = sin(pi_rand_a) * cone + sin(pi_rand_b) * cone;

			float step = screen_h / 180 * 90.f;
			int screen_spread_x = (int)(spread_x * step * 0.7f);
			int screen_spread_y = (int)(spread_y * step * 0.7f);

			float percentage = (rand_a * inaccuracy + rand_b * spread) / (inaccuracy + spread);

			render::get().rect(cross_x + screen_spread_x, cross_y + screen_spread_y, 1, 1,
				g_cfg.esp.show_spread_color);
		}
	}
}