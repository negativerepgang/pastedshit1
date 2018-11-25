#include "fakewalk.h"
#include "..\ragebot\antiaim.h"
#include "..\sdk\animation_state.h"

Vector unpredictedVelocity = Vector(0, 0, 0);

void fakewalk::create_move() {
	auto friction = [](vec3_t & out_vel) -> void {
		float speed, newspeed, control;
		float friction;
		float drop;

		speed = out_vel.Length();

		if (speed <= 0.1f)
			return;

		drop = 0;

		if (g_ctx.m_local->m_fFlags() & FL_ONGROUND) {
			friction = g_csgo.m_cvar()->FindVar("sv_friction")->GetFloat() * g_ctx.m_local->m_surfaceFriction();

			control = (speed < g_csgo.m_cvar()->FindVar("sv_stopspeed")->GetFloat()) ? g_csgo.m_cvar()->FindVar("sv_stopspeed")->GetFloat() : speed;

			drop += control * friction * g_csgo.m_globals()->m_frametime;
		}

		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;

		if (newspeed != speed) {
			newspeed /= speed;

			VectorMultiply(out_vel, newspeed, out_vel);
		}
	};

	auto walk_move = [](player_t * e, vec3_t & out_vel) -> void {
		auto accelerate = [](player_t * e, vec3_t & wishdir, float wishspeed, float accel, vec3_t & out_vel) {
			float currentspeed = out_vel.Dot(wishdir);

			float addspeed = wishspeed - currentspeed;

			if (addspeed <= 0)
				return;

			float accelspeed = accel * g_csgo.m_globals()->m_frametime * wishspeed * e->m_surfaceFriction();

			if (accelspeed > addspeed)
				accelspeed = addspeed;

			for (int i = 0; i < 3; i++)
				out_vel[i] += accelspeed * wishdir[i];
		};

		vec3_t forward, right, up, wishvel, wishdir, dest;
		float fmove, smove, wishspeed;

		math::angle_vectors(e->m_angEyeAngles(), &forward, &right, &up);

		g_csgo.m_movehelper()->SetHost(e);
		fmove = g_csgo.m_movehelper()->m_flForwardMove;
		smove = g_csgo.m_movehelper()->m_flSideMove;
		g_csgo.m_movehelper()->SetHost(nullptr);

		if (forward[2] != 0) {
			forward[2] = 0;
			math::normalize_vector(forward);
		}

		if (right[2] != 0) {
			right[2] = 0;
			math::normalize_vector(right);
		}

		for (int i = 0; i < 2; i++)
			wishvel[i] = forward[i] * fmove + right[i] * smove;

		wishvel[2] = 0;

		wishdir = wishvel;
		wishspeed = wishdir.Normalize();


		g_csgo.m_movehelper()->SetHost(e);
		if ((wishspeed != 0.0f) && (wishspeed > g_csgo.m_movehelper()->m_flMaxSpeed)) {
			VectorMultiply(wishvel, e->m_flMaxspeed() / wishspeed, wishvel);
			wishspeed = e->m_flMaxspeed();
		}
		g_csgo.m_movehelper()->SetHost(nullptr);

		out_vel[2] = 0;
		accelerate(e, wishdir, wishspeed, g_csgo.m_cvar()->FindVar("sv_accelerate")->GetFloat(), out_vel);
		out_vel[2] = 0;

		VectorAdd(out_vel, e->m_vecBaseVelocity(), out_vel);

		float spd = out_vel.Length();

		if (spd < 1.0f) {
			out_vel.Init();

			VectorSubtract(out_vel, e->m_vecBaseVelocity(), out_vel);
			return;
		}

		g_csgo.m_movehelper()->SetHost(e);
		g_csgo.m_movehelper()->m_outWishVel += wishdir * wishspeed;
		g_csgo.m_movehelper()->SetHost(nullptr);

		if (!(e->m_fFlags() & FL_ONGROUND)) {
			VectorSubtract(out_vel, e->m_vecBaseVelocity(), out_vel);
			return;
		}

		VectorSubtract(out_vel, e->m_vecBaseVelocity(), out_vel);
	};

	g_ctx.m_globals.fakewalking = false;

	if (g_cfg.misc.fakewalk && g_csgo.m_inputsys()->IsButtonDown(g_cfg.misc.fakewalk_key))
	{
		g_ctx.m_globals.fakewalking = true;

		g_ctx.send_packet = true;
		static int choked = 0;
		g_ctx.send_packet > 14;
		choked = choked > 12 ? 0 : choked + 1;
		g_ctx.get_command()->m_forwardmove = choked < 1 || choked > 14 ? 0 : g_ctx.get_command()->m_forwardmove;
		g_ctx.get_command()->m_sidemove = choked < 1 || choked > 14 ? 0 : g_ctx.get_command()->m_sidemove;
		g_ctx.send_packet = choked < 1;


		Vector velocity = g_ctx.m_globals.unpredictedVelocity;

		if (choked > 5)
		{
			if (choked < 7 && velocity.Length2D() < 0.1f)
			{
				Vector direction = velocity.Direction();
				float speed = velocity.Length();
				direction.y = g_ctx.get_command()->m_viewangles.y - direction.y;

				Vector forward;
				math::angle_vectors(direction, forward);

				Vector negated_direction = forward * -speed;

				g_ctx.get_command()->m_forwardmove = negated_direction.x;
				g_ctx.get_command()->m_sidemove = negated_direction.y;
			}
			else
			{
				g_ctx.get_command()->m_forwardmove = 0;
				g_ctx.get_command()->m_sidemove = 0;
			}
		}
	}
}








































