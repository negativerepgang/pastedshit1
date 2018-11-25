#include "..\cmenu.hpp"

C_CheckBox::C_CheckBox(std::string n_label, bool * n_state, std::vector< C_ControlFlag * > n_flags) {
	parent = g_cfg.menu.last_group;
	label = n_label;
	state = n_state;

	flags = n_flags;

	area.h = 9;

	parent->add_control(this);
}

void C_CheckBox::draw() {
	render::get().gradient(area.x, area.y, 9, 9, Color{ 62, 62, 62, 255 }, Color::Black, GRADIENT_VERTICAL);

	POINT mouse; GetCursorPos(&mouse);
	rect_t item_region = rect_t(area.x, area.y, 9, 9);

	if (item_region.contains_point(mouse)) {
		render::get().gradient(area.x, area.y, 9, 9, Color{ 72, 72, 72, 255 }, Color::Black, GRADIENT_VERTICAL);
	}

	if (*state) {
		render::get().gradient(area.x, area.y, 9, 9, g_cfg.menu.menu_theme[0], Color::Black, GRADIENT_VERTICAL);
	}

	render::get().rect(area.x, area.y, 9, 9, Color::Black);

	render::get().text(fonts[ESPFLAG], area.x + 15, area.y + 5, Color::White, HFONT_CENTERED_Y, label.c_str());
}

void C_CheckBox::update() {
}

void C_CheckBox::click() {
	*state = !(*state);
}