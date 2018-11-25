#include "antiaim.h"
#include "manual_aa.h"
#include "..\sdk\animation_state.h"
#include "..\autowall\autowall.h"

bool antiaim::nade_out(CUserCmd *pUserCmd)
{
	auto m_weapon = g_ctx.m_local->m_hActiveWeapon().Get();

	if (m_weapon->is_grenade())
	{

		if (!m_weapon->m_bPinPulled() || g_ctx.get_command()->m_buttons & IN_ATTACK || g_ctx.get_command()->m_buttons & IN_ATTACK2)
		{

			float fThrowTime = m_weapon->m_fThrowTime();

			if ((fThrowTime > 0) && (fThrowTime < g_csgo.m_globals()->m_curtime))
				return true;
		}

		return false;
	}
	else
		return (g_ctx.get_command()->m_buttons & IN_ATTACK && m_weapon->can_fire());
}

void antiaim::create_move() {
	manual_aa::get().update();

	auto m_weapon = g_ctx.m_local->m_hActiveWeapon().Get();

	if (!m_weapon)
		return;

	if (g_ctx.get_command()->m_buttons & IN_USE)
		return;

	if (g_ctx.m_local->m_bGunGameImmunity() || g_ctx.m_local->m_fFlags() & FL_FROZEN || freeze_check)
		return;

	if (m_weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) {

	}

	if (m_weapon->m_iItemDefinitionIndex() == WEAPON_HEGRENADE)
		return;

	if (m_weapon->m_iItemDefinitionIndex() == WEAPON_INCGRENADE)
		return;

	if (m_weapon->m_iItemDefinitionIndex() == WEAPON_MOLOTOV)
		return;

	if (m_weapon->m_iItemDefinitionIndex() == WEAPON_SMOKEGRENADE)
		return;
	else if (m_weapon->is_grenade()) {
		return;
	}
	else {
		if (m_weapon->get_csweapon_info()->type == WEAPONTYPE_KNIFE && ((g_ctx.get_command()->m_buttons & IN_ATTACK) || (g_ctx.get_command()->m_buttons & IN_ATTACK2))) {
			return;
		}
		else if ((g_ctx.get_command()->m_buttons & IN_ATTACK) && (m_weapon->m_iItemDefinitionIndex() != WEAPON_C4)) {
			if (m_weapon->can_fire())
				return;
		}
	}

	if (g_ctx.m_local->get_move_type() == MOVETYPE_NOCLIP || g_ctx.m_local->get_move_type() == MOVETYPE_LADDER)
		return;

	if (!g_ctx.m_globals.fakelagging)
		g_ctx.send_packet = g_ctx.get_command()->m_command_number % 2;

	g_ctx.get_command()->m_viewangles.x = get_pitch();

	if (!manual_aa::get().run())
		g_ctx.get_command()->m_viewangles.y = g_ctx.send_packet ? get_fake() : get_real();

	if (!g_ctx.send_packet && g_cfg.antiaim.lby_breaker) {
		if (m_bBreakBalance && g_cfg.antiaim.break_adjustment) {
			bool negative = g_cfg.antiaim.lby_delta < 0;

			if (abs(g_cfg.antiaim.lby_delta) > 120) {
				float new_delta = g_cfg.antiaim.lby_delta / 2;

				g_ctx.get_command()->m_viewangles.y += negative ? -new_delta : new_delta;
			}
			else {
				float new_delta = g_cfg.antiaim.lby_delta - 180;

				g_ctx.get_command()->m_viewangles.y += negative ? -new_delta : new_delta;
			}
		}

		if (m_bBreakLowerBody) {
			g_ctx.get_command()->m_viewangles.y += g_cfg.antiaim.lby_delta;

			if (g_cfg.antiaim.pitch_flick) g_ctx.get_command()->m_viewangles.x = -89;
		}
	}
}

void antiaim::update_lowerbody_breaker() {
	if (!g_ctx.m_local->m_iHealth())
		return;

	float
		server_time = util::server_time(),
		speed = animation_state::get().animstate()->m_velocity;

	if (speed > 0.1) {
		m_flNextBodyUpdate = server_time + 0.22;
		m_flLastMoveTime = util::server_time();
	}

	m_bBreakLowerBody = false;
	m_bBreakBalance = false;

	if (m_flNextBodyUpdate < server_time) {
		m_flNextBodyUpdate = server_time + 1.1;
		m_bBreakLowerBody = true;
	}

	if (m_flNextBodyUpdate - g_csgo.m_globals()->m_interval_per_tick < server_time)
		m_bBreakBalance = true;

	if (!(g_ctx.m_local->m_fFlags() & FL_ONGROUND)) {
		m_bBreakLowerBody = false;
		m_bBreakBalance = false;
	}
}

float antiaim::at_target() {
	int cur_tar = 0;
	float last_dist = 999999999999.0f;

	for (int i = 0; i < g_csgo.m_globals()->m_maxclients; i++) {
		auto m_entity = static_cast<entity_t*>(g_csgo.m_entitylist()->GetClientEntity(i));

		if (!m_entity || m_entity == g_ctx.m_local)
			continue;

		if (!m_entity->is_player())
			continue;

		auto m_player = (player_t*)m_entity;
		if (!m_player->IsDormant() && m_player->is_alive() && m_player->m_iTeamNum() != g_ctx.m_local->m_iTeamNum()) {
			float cur_dist = (m_entity->m_vecOrigin() - g_ctx.m_local->m_vecOrigin()).Length();

			if (!cur_tar || cur_dist < last_dist) {
				cur_tar = i;
				last_dist = cur_dist;
			}
		}
	}

	if (cur_tar) {
		auto m_entity = static_cast<player_t*>(g_csgo.m_entitylist()->GetClientEntity(cur_tar));
		if (!m_entity) {
			return g_ctx.get_command()->m_viewangles.y;
		}

		Vector target_angle = math::calculate_angle(g_ctx.m_local->m_vecOrigin(), m_entity->m_vecOrigin());
		return target_angle.y;
	}

	return g_ctx.get_command()->m_viewangles.y;
}


float antiaim::get_pitch() {
	int	index;

	if (!(g_ctx.m_local->m_fFlags() & FL_ONGROUND)) {
		index = g_cfg.antiaim.pitch[1];
	}
	else {
		index = g_cfg.antiaim.pitch[1];

		if (g_ctx.m_local->m_vecVelocity().Length() > 0.1f)
			index = g_cfg.antiaim.pitch[1];
	}

	float pitch = g_ctx.get_command()->m_viewangles.x;

	switch (index) {
	case 1:
		pitch = 89.0f;
		break;
	case 2:
		pitch = -89.0f;
		break;
	case 3: {
		pitch = 89.0f;
		break;
	}
	case 4:
		pitch = 1080.0f;
		break;
	case 5:
		pitch = -179.990005f;
		break;
	case 6:
		pitch = 971.0f;
		break;
	}

	return pitch;
}

float antiaim::get_fake() {
	int
		index,
		base_angle,
		offset;

	if (!(g_ctx.m_local->m_fFlags() & FL_ONGROUND)) {
		index = g_cfg.antiaim.fake_yaw[2];
		base_angle = g_cfg.antiaim.base_angle[2];
		offset = g_cfg.antiaim.fake_yaw_offset[2];
	}
	else {
		index = g_cfg.antiaim.fake_yaw[0];
		base_angle = g_cfg.antiaim.base_angle[0];
		offset = g_cfg.antiaim.fake_yaw_offset[0];

		if (g_ctx.m_local->m_vecVelocity().Length() > 0.1f) {
			index = g_cfg.antiaim.fake_yaw[1];
			base_angle = g_cfg.antiaim.base_angle[1];
			offset = g_cfg.antiaim.fake_yaw_offset[1];
		}
	}

	float yaw = g_ctx.get_command()->m_viewangles.y;

	if (index) {
		yaw = 0.0f;

		if (base_angle == 1) {
			vec3_t angles; g_csgo.m_engine()->GetViewAngles(angles);
			yaw = angles.y;
		}

		if (base_angle == 2)
			yaw = at_target();

		switch (index) {
		case 1:
			yaw += 90.0f;
			break;
		case 2:
			yaw += 180;
			break;
		case 3:
			yaw += g_csgo.m_globals()->m_curtime * 425.0f;
			break;
		case 4:
			yaw += 180 + (math::random_float(0, 50) - 25);
			break;
		case 5: {
			static bool side[2] = { false, true };

			float range = side[1] ? 100 : 80;

			yaw += side[0] ? range : -range;

			side[1] = !side[1];

			if (side[1])
				side[0] = !side[0];

			break;
		}
		case 6: {
			vec3_t angles; g_csgo.m_engine()->GetViewAngles(angles);

			yaw = angles.y;

			break;
		}
		case 7:
			yaw = g_ctx.m_local->m_flLowerBodyYawTarget();
			break;
		case 8:
			auto weapon = g_ctx.m_local->m_hActiveWeapon().Get();

			if (g_ctx.get_command()->m_buttons & IN_ATTACK && weapon->can_fire())
				yaw = freestanding() + (math::random_float(0, 50) - 25);
			else
				yaw = freestanding();


			break;
		}

		yaw += offset;
	}

	return yaw;
}

float antiaim::get_real() {
	int
		index,
		base_angle,
		offset;

	if (!(g_ctx.m_local->m_fFlags() & FL_ONGROUND)) {
		index = g_cfg.antiaim.yaw[2];
		base_angle = g_cfg.antiaim.base_angle[2];
		offset = g_cfg.antiaim.yaw_offset[2];
	}
	else {
		index = g_cfg.antiaim.yaw[0];
		base_angle = g_cfg.antiaim.base_angle[0];
		offset = g_cfg.antiaim.yaw_offset[0];

		if (g_ctx.m_local->m_vecVelocity().Length() > 0.1f) {
			index = g_cfg.antiaim.yaw[1];
			base_angle = g_cfg.antiaim.base_angle[1];
			offset = g_cfg.antiaim.yaw_offset[1];
		}
	}

	float yaw = g_ctx.get_command()->m_viewangles.y;

	if (index) {
		yaw = 0.0f;

		if (base_angle == 1) {
			vec3_t angles; g_csgo.m_engine()->GetViewAngles(angles);
			yaw = angles.y;
		}

		if (base_angle == 2)
			yaw = at_target();

		switch (index) {
		case 1:
			yaw += 90.0f;
			break;
		case 2:
			yaw += 180;
			break;
		case 3:
			yaw += g_csgo.m_globals()->m_curtime * 425.0f;
			break;
		case 4:
			yaw += 180 + (math::random_float(0, 50) - 25);
			break;
		case 5: {
			vec3_t angles; g_csgo.m_engine()->GetViewAngles(angles);
			yaw = angles.y;
			break;
		}
		case 6:
			yaw = g_ctx.m_local->m_flLowerBodyYawTarget();
			break;
		case 7:
			auto weapon = g_ctx.m_local->m_hActiveWeapon().Get();

			if (g_ctx.get_command()->m_buttons & IN_ATTACK && weapon->can_fire())
				yaw = freestanding() + (math::random_float(0, 50) - 25);
			else
				yaw = freestanding();
			break;
		}

		yaw += offset;
	}

	return yaw;
}

void antiaim::get_unpredicted_velocity() {
	if (!g_ctx.m_local)
		return;

	unpredicted_velocity = g_ctx.m_local->m_vecVelocity();
}

float antiaim::freestanding() {
	enum {
		back,
		right,
		left
	};

	vec3_t view_angles;
	g_csgo.m_engine()->GetViewAngles(view_angles);

	static constexpr int damage_tolerance = 30;

	std::vector< player_t * > enemies;

	auto get_target = [&]() -> player_t * {
		player_t * target = nullptr;
		float best_fov = 360.f;

		for (int id = 1; id <= g_csgo.m_globals()->m_maxclients; id++) {
			auto e = static_cast<player_t *>(g_csgo.m_entitylist()->GetClientEntity(id));

			if (!e->valid(true))
				continue;

			float fov = math::get_fov(view_angles, math::calculate_angle(g_ctx.m_local->get_eye_pos(), e->m_vecOrigin()));

			if (fov < best_fov) {
				target = e;
				best_fov = fov;
			}

			enemies.push_back(e);
		}

		return target;
	};

	player_t * e = get_target();

	if (!e)
		return view_angles.y + 180;

	auto calculate_damage = [&](vec3_t point) -> int {
		int damage = 0;
		for (auto& enemy : enemies)
			damage += autowall::get().calculate_return_info(enemy->get_eye_pos(), point, enemy, g_ctx.m_local, 1).m_damage;

		return damage;
	};

	auto rotate_and_extend = [](vec3_t position, float yaw, float distance) -> vec3_t {
		vec3_t direction;
		math::angle_vectors(vec3_t(0, yaw, 0), direction);

		return position + (direction * distance);
	};

	vec3_t
		head_position = g_ctx.m_local->get_eye_pos(),
		at_target = math::calculate_angle(g_ctx.m_local->m_vecOrigin(), e->m_vecOrigin());

	float angles[3] = {
			at_target.y + 160.f + (math::random_float(0, 50) - 25),
			at_target.y - 70.f,
			at_target.y + 70.f
	};

	vec3_t head_positions[3] = {
		rotate_and_extend(head_position, at_target.y + 180.f, 17.f + (math::random_float(0, 50) - 25)),
		rotate_and_extend(head_position, at_target.y - 70.f, 17.f),
		rotate_and_extend(head_position, at_target.y + 70.f, 17.f)
	};

	int damages[3] = {
		calculate_damage(head_positions[back]),
		calculate_damage(head_positions[right]),
		calculate_damage(head_positions[left])
	};

	if (damages[right] > damage_tolerance && damages[left] > damage_tolerance)
		return angles[back];

	if (at_target.x > 15.f)
		return angles[back];

	if (damages[right] == damages[left]) {
		auto trace_to_end = [](vec3_t start, vec3_t end) -> vec3_t {
			trace_t trace;
			CTraceFilterWorldOnly filter;
			Ray_t ray;

			ray.Init(start, end);
			g_csgo.m_trace()->TraceRay(ray, MASK_ALL, &filter, &trace);

			return trace.endpos;
		};

		vec3_t
			trace_right_endpos = trace_to_end(head_position, head_positions[right]),
			trace_left_endpos = trace_to_end(head_position, head_positions[left]);

		Ray_t ray;
		trace_t trace;
		CTraceFilterWorldOnly filter;

		ray.Init(trace_right_endpos, e->get_eye_pos());
		g_csgo.m_trace()->TraceRay(ray, MASK_ALL, &filter, &trace);
		float distance_1 = (trace.startpos - trace.endpos).Length();

		ray.Init(trace_left_endpos, e->get_eye_pos());
		g_csgo.m_trace()->TraceRay(ray, MASK_ALL, &filter, &trace);
		float distance_2 = (trace.startpos - trace.endpos).Length();

		if (fabs(distance_1 - distance_2) > 15.f)
			return (distance_1 < distance_2) ? angles[right] : angles[left];
		else
			return angles[back];
	}
	else
		return (damages[right] < damages[left]) ? angles[right] : angles[left];
}