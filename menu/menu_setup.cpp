#include "cmenu.hpp"
#include "..\cheats\misc\logs.h"

#include <shlobj.h>
#include <shlwapi.h>

std::string get_config_dir() {
	std::string folder;
	static TCHAR path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
		folder = std::string(path) + "\\m-cheats\\";
	}

	CreateDirectory(folder.c_str(), NULL);

	return folder;
}

void load_config() {
	if (!cfg_manager->files.size()) {
		return;
	}


	cfg_manager->load(cfg_manager->files[g_cfg.selected_config]);
}

void save_config() {
	if (!cfg_manager->files.size()) {
		return;
	}

	eventlogs::get().add("saved " + cfg_manager->files[g_cfg.selected_config], Color::White);

	cfg_manager->save(cfg_manager->files[g_cfg.selected_config]);
	cfg_manager->config_files();
}

void remove_config() {
	if (!cfg_manager->files.size()) {
		return;
	}

	cfg_manager->remove(cfg_manager->files[g_cfg.selected_config]);
	cfg_manager->config_files();

	if (g_cfg.selected_config > cfg_manager->files.size() - 1) {
		g_cfg.selected_config = cfg_manager->files.size() - 1;
	}
}

void add_config() {
	if (g_cfg.new_config_name.find(".json") == -1)
		g_cfg.new_config_name += ".json";

	cfg_manager->save(g_cfg.new_config_name.c_str());
	cfg_manager->config_files();

	g_cfg.new_config_name = "";

	g_cfg.selected_config = cfg_manager->files.size() - 1;
}

void unload_cheat() {
	unload = true;
}


void setup_main_menu() {
	static auto set_sub = [](int sub) -> void {
		g_cfg.menu.group_sub = sub;
	};

	static auto set_tabsub = [](int sub) -> void {
		g_cfg.menu.tab_sub = sub;
	};

	auto legit = new C_Tab(&menu::get(), 0, "s", "I"); {
		auto legit_aimbot = new C_GroupBox(GROUP_RIGHT, 8, ""); {
			auto aim_assist = new C_CheckBox("aim assist", &g_cfg.legit.aimassist);
			auto aim_assist_key = new C_KeyBind("", &g_cfg.legit.aim_assist_bind);
			auto fov = new C_SliderFloat("field of view", &g_cfg.legit.fov, 0, 180, "");
			auto smoothing = new C_SliderFloat("smoothing", &g_cfg.legit.smoothing, 0, 15, "");
		}

		auto legit_backtrack = new C_GroupBox(GROUP_LEFT, 8, ""); {
			auto backtracking = new C_CheckBox("backtracking", &g_cfg.legit.backtracking);
			auto max_time = new C_SliderFloat("max time", &g_cfg.legit.backtracking_max_time, 0.00f, 0.20f, "s");
			auto visualize = new C_CheckBox("visualize", &g_cfg.legit.backtracking_visualize,
				{ new C_ColorSelector(&g_cfg.legit.backtracking_visualize_color) });
		}
	}

	auto rage_tab = new C_Tab(&menu::get(), 1, "b", "A", {
		C_Tab::subtab_t("A", 0, fonts[USIAFONT]),
		C_Tab::subtab_t("E", 1, fonts[SUBTABWEAPONS]),
		C_Tab::subtab_t("A", 2, fonts[SUBTABWEAPONS]),
		C_Tab::subtab_t("W", 3, fonts[SUBTABWEAPONS]),
		C_Tab::subtab_t("O", 4, fonts[SUBTABWEAPONS]),
		C_Tab::subtab_t("a", 5, fonts[SUBTABWEAPONS]),
		C_Tab::subtab_t("Y", 6, fonts[SUBTABWEAPONS]),
		C_Tab::subtab_t("e", 7, fonts[SUBTABWEAPONS])


		}); {
		//set_sub
		set_tabsub(0);

		auto rage = new C_GroupBox(GROUP_LEFT, 8, ""); {
			auto enable = new C_CheckBox("enable", &g_cfg.ragebot.enable);
			auto anti_recoil = new C_CheckBox("no recoil", &g_cfg.ragebot.anti_recoil);
			auto anti_spread = new C_CheckBox("anti spread", &g_cfg.ragebot.anti_spread);
			auto auto_stop = new C_CheckBox("automatic stop", &g_cfg.ragebot.autostopye);
			auto silent_aim = new C_CheckBox("silent aimbot", &g_cfg.ragebot.silent_aim);
			auto zeus_bot = new C_CheckBox("zeus bot", &g_cfg.ragebot.zeus_bot);
			auto fov = new C_SliderInt("field of view", &g_cfg.ragebot.field_of_view, 0, 360, "");
			auto ping_spike = new C_CheckBox("ping spike", &g_cfg.misc.ping_spike);
			auto ping_spike_amount = new C_SliderFloat("amount", &g_cfg.misc.ping_spike_value, 0, 800, "ms");
		}

		auto rage2 = new C_GroupBox(GROUP_RIGHT, 8, ""); {
			auto selection_type = new C_Dropdown("selection type", &g_cfg.ragebot.selection_type, { "fov", "distance", "health" });
			auto aa_correction = new C_CheckBox("resolver", &g_cfg.ragebot.antiaim_correction);
			auto extrapolation = new C_CheckBox("extrapolation", &g_cfg.ragebot.extrapolation);
			auto override_key = new C_KeyBind("override key", &g_cfg.ragebot.override_key);
			auto aimstep = new C_CheckBox("aim step", &g_cfg.ragebot.aimstep);
			auto lagcomp = new C_CheckBox("backtrack", &g_cfg.ragebot.lagcomp);
			auto lag_compensation_type = new C_Dropdown("backtrack type", &g_cfg.ragebot.lag_compensation_type, { "lby bt" });
		}

		for (int i = 0; i < 7; i++) {
			set_tabsub(i + 1);

			auto weapon = new C_GroupBox(GROUP_LEFT, 8, ""); {
				auto hitchance = new C_CheckBox("spread limit", &g_cfg.ragebot.hitchance[i]);
				auto hitchance_amount = new C_SliderInt("", &g_cfg.ragebot.hitchance_amount[i], 0, 100, "%");
				auto minimum_damage = new C_SliderInt("minimum damage", &g_cfg.ragebot.minimum_damage[i], 1, 120, "");
				auto pointscale = new C_SliderFloat("pointscale", &g_cfg.ragebot.pointscale[i], 0, 1, "");

				auto autoshoot = new C_CheckBox("automatic shoot", &g_cfg.ragebot.autoshoot[i]);
				auto autowall = new C_CheckBox("automatic wall", &g_cfg.ragebot.autowall[i]);


				if (i == 2 || i == 4 || i == 5) {
					int autoscope_element;

					if (i == 2) autoscope_element = 0;
					if (i == 4) autoscope_element = 1;
					if (i == 5) autoscope_element = 2;

					auto autoscope = new C_CheckBox("automatic scope", &g_cfg.ragebot.autoscope[autoscope_element]);
				}

			}

			auto weapon2 = new C_GroupBox(GROUP_RIGHT, 8, ""); {

				auto baim = new C_MultiDropdown("body aim when", &g_cfg.ragebot.baim_settings[i]);


				auto hitscan = new C_MultiDropdown("hitscan hitboxes", &g_cfg.ragebot.hitscan[i]);

				auto hitscan_history = new C_MultiDropdown("hitscan backtrack hitobxes", &g_cfg.ragebot.hitscan_history[i]);

				auto multipoint = new C_MultiDropdown("multipoint hitboxes", &g_cfg.ragebot.multipoint_hitboxes[i]);

			}
		}
	}

	auto antiaim_tab = new C_Tab(&menu::get(), 2, "i", "H", {
		C_Tab::subtab_t("still", 0),
			C_Tab::subtab_t("moving", 1),
			C_Tab::subtab_t("air", 2)

		}); {
		for (int i = 0; i < 3; i++) {
			set_tabsub(i);
			auto antiaim = new C_GroupBox(GROUP_LEFT, 8, ""); {




				auto base_angle = new C_Dropdown("antiaim based on angle", &g_cfg.antiaim.base_angle[i], { "default", "crosshair", "at targets" });



				auto pitch = new C_Dropdown("antiaim pitch", &g_cfg.antiaim.pitch[i], { "NULL", "down"});


				std::vector< std::string > reals{ "NULL", "left/right", "180", "spin", "shuffle", "eye angles", "lby", "auto" };

				auto yaw = new C_Dropdown("antiaim yaw", &g_cfg.antiaim.yaw[i], reals);
				auto yaw_offset = new C_SliderInt("", &g_cfg.antiaim.yaw_offset[i], -180, 180, "");

				std::vector< std::string > fakes{ "NULL", "left/right", "180", "spin", "shuffle", "side shuffle", "eye angles", "lby", "auto" };

				auto fakeyaw = new C_Dropdown("antiaim fake-yaw", &g_cfg.antiaim.fake_yaw[i], fakes);
				auto fakeyaw_offset = new C_SliderInt("", &g_cfg.antiaim.fake_yaw_offset[i], -180, 180, "");

				auto lby_breaker = new C_CheckBox("lby breaker", &g_cfg.antiaim.lby_breaker);
				auto lby_delta = new C_SliderInt("", &g_cfg.antiaim.lby_delta, 0, 180, "");
				auto balance_breaker = new C_CheckBox("lby break double flick", &g_cfg.antiaim.break_adjustment);
				auto flick_up = new C_CheckBox("flick up", &g_cfg.antiaim.pitch_flick);
				auto balance_breaker2 = new C_CheckBox("balance breaker", &g_cfg.antiaim.break_adjustment2);


			}
		}

			for (int i = 0; i < 3; i++) {
				set_tabsub(i);
				auto antiaim_2 = new C_GroupBox(GROUP_RIGHT, 8, ""); {

				auto anti_untrusted = new C_CheckBox("clamp angles", &g_cfg.misc.anti_untrusted);

				auto fakewalk = new C_CheckBox("fakewalk", &g_cfg.misc.fakewalk);
				auto fakewalk_key = new C_KeyBind("", &g_cfg.misc.fakewalk_key);

				auto fakelag_enablers = new C_MultiDropdown("fakelag", &g_cfg.antiaim.fakelag_enablers);
				auto fakelag_mode = new C_Dropdown("", &g_cfg.antiaim.fakelag_mode, { "factor", "dynamic" });
				auto fakelag_amount = new C_SliderInt("", &g_cfg.antiaim.fakelag_amount, 1, 12, "");

				auto manualkey_back = new C_KeyBind("manual antiaim back", &g_cfg.antiaim.back_manualkey);
				auto manualkey_left = new C_KeyBind("manual antiaim left", &g_cfg.antiaim.left_manualkey);
				auto manualkey_right = new C_KeyBind("manual antiaim right", &g_cfg.antiaim.right_manualkey);
				auto animation_fix = new C_CheckBox("animation fix", &g_cfg.antiaim.animfix);



			}


			//	auto fakelag_onpeek = new C_CheckBox( "on peek *todo*", &g_cfg.antiaim.fakelag_onpeek );
			//	auto fakelag_onpeek_amt = new C_SliderInt( "", &g_cfg.antiaim.fakelag_onpeek_amt, 1, 14, "" );

		}
	}

	auto esp_tab = new C_Tab(&menu::get(), 3, "d", "D", {
		C_Tab::subtab_t("entities", 0),
		C_Tab::subtab_t("misc", 1)
		}); {
		set_tabsub(0);

		auto player = new C_GroupBox(GROUP_LEFT, 8, ""); {
			auto enable = new C_CheckBox("enable", &g_cfg.player.enable);

			auto pov_arrow = new C_CheckBox("enemy offscreen esp", &g_cfg.player.out_pov_arrow,
				{ new C_ColorSelector(&g_cfg.player.out_pov_arrow_color) });

			auto pov_distance = new C_SliderInt("", &g_cfg.player.out_pov_distance, 25, 400, "");

			auto box = new C_CheckBox("box", &g_cfg.player.box,
				{ new C_ColorSelector(&g_cfg.player.box_color) });

			auto health = new C_CheckBox("health", &g_cfg.player.health);

			auto name = new C_CheckBox("name", &g_cfg.player.name,
				{ new C_ColorSelector(&g_cfg.player.name_color) });

			auto weapon = new C_CheckBox("weapon", &g_cfg.player.weapon,
				{ new C_ColorSelector(&g_cfg.player.weapon_color) });

			auto flags = new C_MultiDropdown("flags", &g_cfg.player.flags);

			auto skeleton = new C_MultiDropdown("bones", &g_cfg.player.skeleton);
			auto glow_opacity = new C_SliderInt("glow opacity", &g_cfg.player.glowopacity, 1, 100, "%%");
			auto glow_bloom = new C_SliderInt("glow pearlescence", &g_cfg.player.glowbloom, 1, 100, "%%");
			auto chams_opacity = new C_SliderInt("chams opacity", &g_cfg.player.chams_opacity, 0, 100, "%%");




		}

		auto player_2 = new C_GroupBox(GROUP_RIGHT, 8, ""); {
			auto glow = new C_MultiDropdown("glow", &g_cfg.player.glow);

			auto glow_color_enemy = new C_TextSeperator("enemy colour", SEPERATOR_NORMAL,
				{ new C_ColorSelector(&g_cfg.player.glow_color_enemy) });

			auto glow_color_teammate = new C_TextSeperator("teammate colour", SEPERATOR_NORMAL,
				{ new C_ColorSelector(&g_cfg.player.glow_color_teammate) });


			auto glow_type = new C_Dropdown("", &g_cfg.player.glow_type, { "glowstyle 0", "glowstyle 1" });

			auto chams = new C_MultiDropdown("chams", &g_cfg.player.chams);
			auto chams_type = new C_Dropdown("", &g_cfg.player.chams_type, { "regular", "flat", "rubber", "metallic" });

			auto backtrack_chams = new C_CheckBox("backtrack model", &g_cfg.player.backtrack_chams,
				{ new C_ColorSelector(&g_cfg.player.backtrack_chams_color) });

			auto backtrack_chams_opacity = new C_SliderInt("", &g_cfg.player.backtrack_chams_opacity, 0, 100, "%%");


			auto chams_color_visible = new C_TextSeperator("visible colour", SEPERATOR_NORMAL,
				{ new C_ColorSelector(&g_cfg.player.chams_color) });

			auto chams_color_invisible = new C_TextSeperator("invisible colour", SEPERATOR_NORMAL,
				{ new C_ColorSelector(&g_cfg.player.xqz_color) });

			auto ghost_chams = new C_CheckBox("ghost chams", &g_cfg.esp.ghost_chams,
				{ new C_ColorSelector(&g_cfg.esp.ghost_chams_color) });

			auto local_chams = new C_CheckBox("local chams", &g_cfg.esp.local_chams,
				{ new C_ColorSelector(&g_cfg.esp.local_chams_color) });


			auto local_chams_alpha = new C_SliderInt("local chams opacity", &g_cfg.esp.local_chams_alpha, 0, 100, "");



		}

		set_tabsub(1);

		auto esp = new C_GroupBox(GROUP_LEFT, 8, ""); {
			auto dropped_weapon = new C_CheckBox("draw dropped weapons", &g_cfg.esp.dropped_weapons,
				{ new C_ColorSelector(&g_cfg.esp.dropped_weapons_color) });

			auto projectiles = new C_CheckBox("draw projectiles", &g_cfg.esp.projectiles,
				{ new C_ColorSelector(&g_cfg.esp.projectiles_color) });

			auto planted_bomb = new C_CheckBox("planted bomb", &g_cfg.esp.planted_bomb,
				{ new C_ColorSelector(&g_cfg.esp.planted_bomb_color) });

			auto bomb_timer = new C_CheckBox("bomb timer", &g_cfg.esp.bomb_timer);

			auto override_fov_scoped = new C_CheckBox("override fov if scoped", &g_cfg.esp.override_fov_scoped);
		
			auto fov = new C_SliderInt("override fov", &g_cfg.esp.fov, 0, 89, "");

			auto penetration_reticle = new C_CheckBox("autowall crosshair", &g_cfg.esp.penetration_reticle);
			
			auto antiaim_indicator = new C_CheckBox("anti-aim indicator", &g_cfg.esp.antiaim_indicator);

		}

		auto esp_2 = new C_GroupBox(GROUP_RIGHT, 8, ""); {

			auto hitmarker = new C_MultiDropdown("hitmarker", &g_cfg.esp.hitmarker);

			auto hitsound = new C_CheckBox("hitsound", &g_cfg.esp.hitsound);
	


			auto nightmode = new C_CheckBox("nightmode", &g_cfg.esp.nightmode);


			auto removals = new C_MultiDropdown("removals", &g_cfg.esp.removals);

			auto bullet_tracer = new C_CheckBox("bullet tracers", &g_cfg.esp.bullet_tracer,
				{ new C_ColorSelector(&g_cfg.esp.bullet_tracer_color) });

			auto show_spread = new C_CheckBox("show spread", &g_cfg.esp.show_spread,
				{ new C_ColorSelector(&g_cfg.esp.show_spread_color) });
			auto show_spread_type = new C_Dropdown("", &g_cfg.esp.show_spread_type, { "regular", "rainbow", "seed" });

		//	auto sleeve_transparency = new C_SliderInt("sleeve transparency", &g_cfg.esp.sleeve_transparency, 0, 100, " ");


		}
	}

	auto misc_tab = new C_Tab(&menu::get(), 4, "c", "G"); {

		auto miscellaneous = new C_GroupBox(GROUP_LEFT, 8, ""); {
			auto funcc = new C_TextSeperator("miscelleanous", SEPERATOR_BOLD);

			auto bunnyhop = new C_CheckBox("automatic jump", &g_cfg.misc.bunnyhop);
			auto airstrafe = new C_CheckBox("automatic strafe", &g_cfg.misc.airstrafe);
			auto circle_stafer = new C_KeyBind("circle strafe", &g_cfg.misc.circle_strafe);
			auto thirdperson = new C_KeyBind("thirdperson", &g_cfg.misc.thirdperson_toggle);
			auto thirdperson_distance = new C_SliderInt("thirdperson distance", &g_cfg.misc.thirdperson_distance, 100, 150, "");

			auto events_to_log = new C_MultiDropdown("logs", &g_cfg.misc.events_to_log);
			auto clantag_spammer = new C_CheckBox("clantag spammer", &g_cfg.misc.clantag_spammer);
			auto sniper_crosshair = new C_CheckBox("sniper crosshair", &g_cfg.misc.sniper_crosshair);
			auto exaggerated_ragdoll = new C_CheckBox("force ragadolls", &g_cfg.misc.exaggerated_ragdolls);
			auto menu_color = new C_TextSeperator("menu colour", SEPERATOR_NORMAL,
				{ new C_ColorSelector(&g_cfg.menu.menu_theme[0]) });

			auto menu_bind = new C_KeyBind("menu bind", &g_cfg.menu.menu_bind);



		}

		auto miscellaneous_2 = new C_GroupBox(GROUP_RIGHT, 8, ""); {


			auto function_seperator = new C_TextSeperator("configuration", SEPERATOR_BOLD);

			auto config_dropdown = new C_Dropdown("", &g_cfg.selected_config, &cfg_manager->files);
			auto load = new C_Button("load", []() { load_config(); });
			auto save = new C_Button("save", []() { save_config(); });
			auto remove = new C_Button("remove", []() { remove_config(); });
			new C_TextField("config name", &g_cfg.new_config_name);
			auto add = new C_Button("add", []() { add_config(); });
			new C_Button("unhook all", []() { unload_cheat(); });
			auto buybot_enable = new C_CheckBox("enable buybot", &g_cfg.misc.buybot_enable);
			auto buybot_rifleschoice2 = new C_MultiDropdown("rifles choice", &g_cfg.misc.buybot_rifleschoice2);
			auto buybot_pistolchoice2 = new C_MultiDropdown("pistols choice", &g_cfg.misc.buybot_pistolchoice2);
		}
	}

}
