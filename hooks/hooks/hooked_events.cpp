#include "..\hooks.hpp"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\visuals\bullet_tracers.h"
#include "..\..\cheats\ragebot\antiaim.h"
#include "..\..\resources\custom_sounds.hpp"

#pragma comment(lib, "Winmm.lib")

void C_HookedEvents::FireGameEvent(IGameEvent* event) {
	static ConVar* sv_showimpacts = g_csgo.m_cvar()->FindVar("sv_showimpacts");
	static auto hit_sound = [&event]() -> void {
		if (!g_ctx.available())
			return;

		auto attacker = event->GetInt("attacker");
		auto user = event->GetInt("userid");

		if (g_cfg.esp.hitsound) {
			if (g_csgo.m_engine()->GetPlayerForUserID(user) != g_csgo.m_engine()->GetLocalPlayer() && g_csgo.m_engine()->GetPlayerForUserID(attacker) == g_csgo.m_engine()->GetLocalPlayer()) {
					PlaySoundA(hitmarker_sound, NULL, SND_ASYNC | SND_MEMORY);
			}
		}
	};

	auto event_name = event->GetName();

	if (!strcmp(event_name, "round_prestart"))
		antiaim::get().freeze_check = true;
	else if (!strcmp(event_name, "round_freeze_end"))
		antiaim::get().freeze_check = false;

	if (g_cfg.esp.hitsound) {
		hitmarker_time = 255;

		if (!strcmp(event_name, "player_hurt"))
			hit_sound();
	}

	if (g_cfg.esp.hitmarker[HITMARKER_STATIC].enabled || g_cfg.esp.hitmarker[HITMARKER_DYNAMIC].enabled)
		otheresp::get().hitmarker_event(event);

	if (g_cfg.esp.bullet_tracer)
		bullettracers::get().events(event);

	if (g_cfg.esp.bullet_tracer)
		sv_showimpacts->SetValue(1);
	else
		sv_showimpacts->SetValue(0);

	eventlogs::get().events(event);

	g_ctx.m_globals.remove_shot(event);

	if (g_cfg.misc.buybot_enable && !strcmp(event_name, "round_prestart")) // Checks if buybot is enabled, and then will buy stuff if round before round
	{

		if (g_cfg.misc.buybot_rifleschoice2[GALILAR].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy galilar");
		}
		if (g_cfg.misc.buybot_rifleschoice2[FAMAS].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy famas");
		}
		if (g_cfg.misc.buybot_rifleschoice2[AK47].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy ak47");
		}
		if (g_cfg.misc.buybot_rifleschoice2[M4A1].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy m4a1");
		}
		if (g_cfg.misc.buybot_rifleschoice2[M4A1_SILENCER].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy m4a1_silencer");
		}
		if (g_cfg.misc.buybot_rifleschoice2[SSG08].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy ssg08");
		}
		if (g_cfg.misc.buybot_rifleschoice2[AUG].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy aug");
		}
		if (g_cfg.misc.buybot_rifleschoice2[SG556].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy sg556");
		}
		if (g_cfg.misc.buybot_rifleschoice2[AWP].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy awp");
		}
		if (g_cfg.misc.buybot_rifleschoice2[SCAR20].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy scar20");
		}
		if (g_cfg.misc.buybot_rifleschoice2[G3SG1].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy g3sg1");
		}
		if (g_cfg.misc.buybot_rifleschoice2[NOVA].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy nova");
		}
		if (g_cfg.misc.buybot_rifleschoice2[XM1014].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy xm1014");
		}
		if (g_cfg.misc.buybot_rifleschoice2[MAG7].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy mag7");
		}
		if (g_cfg.misc.buybot_rifleschoice2[M249].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy m249");
		}
		if (g_cfg.misc.buybot_rifleschoice2[NEGEV].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy negev");
		}
		if (g_cfg.misc.buybot_rifleschoice2[MAC10].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy mac10");
		}
		if (g_cfg.misc.buybot_rifleschoice2[MP9].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy mp9");
		}
		if (g_cfg.misc.buybot_rifleschoice2[MP7].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy mp7");
		}
		if (g_cfg.misc.buybot_rifleschoice2[UMP45].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy ump45");
		}
		if (g_cfg.misc.buybot_rifleschoice2[P90].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy p90");
		}
		if (g_cfg.misc.buybot_rifleschoice2[BIZON].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy bizon");
		}


		if (g_cfg.misc.buybot_pistolchoice2[GLOCK].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy glock");
		}
		if (g_cfg.misc.buybot_pistolchoice2[HKP2000].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy hkp2000");
		}
		if (g_cfg.misc.buybot_pistolchoice2[USP_SILENCER].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy usp_silencer");
		}
		if (g_cfg.misc.buybot_pistolchoice2[ELITE].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy elite");
		}
		if (g_cfg.misc.buybot_pistolchoice2[P250].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy p250");
		}
		if (g_cfg.misc.buybot_pistolchoice2[TEC9].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy tec9");
		}
		if (g_cfg.misc.buybot_pistolchoice2[FIVESEVEN].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy fn57");
		}
		if (g_cfg.misc.buybot_pistolchoice2[DEAGLE].enabled)
		{
			g_csgo.m_engine()->ClientCmd("buy deagle; buy revolver");
		}

	}
}

int C_HookedEvents::GetEventDebugID(void) {
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::RegisterSelf() {
	m_iDebugId = EVENT_DEBUG_ID_INIT;
	g_csgo.m_eventmanager()->AddListener(this, "player_hurt", false);
	g_csgo.m_eventmanager()->AddListener(this, "item_purchase", false);
	g_csgo.m_eventmanager()->AddListener(this, "bomb_beginplant", false);
	g_csgo.m_eventmanager()->AddListener(this, "bomb_begindefuse", false);
	g_csgo.m_eventmanager()->AddListener(this, "bullet_impact", false);
	g_csgo.m_eventmanager()->AddListener(this, "round_prestart", false);
	g_csgo.m_eventmanager()->AddListener(this, "round_freeze_end", false);
}

void C_HookedEvents::RemoveSelf() {
	g_csgo.m_eventmanager()->RemoveListener(this);
}