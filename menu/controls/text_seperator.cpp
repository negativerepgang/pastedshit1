#include "..\cmenu.hpp"

C_TextSeperator::C_TextSeperator(std::string n_label, TextSeperatorType n_type, std::vector< C_ControlFlag * > n_flags) {
	parent = g_cfg.menu.last_group;
	label = n_label;
	type = n_type;

	flags = n_flags;

	area.h = (type == SEPERATOR_NORMAL) ? 12 : 18;

	parent->add_control(this);
}

void C_TextSeperator::draw() {

	if (type == SEPERATOR_BOLD) {
		render::get().text(fonts[ESPFLAG], area.x + 1, area.y + 4, Color{ 64, 64, 64, 255 }, HFONT_CENTERED_NONE, label.c_str());
		render::get().line(area.x, area.y + 18, area.x + area.w, area.y + 18, Color{ 64, 64, 64, 255 });
	}
	else {
		render::get().text(fonts[ESPFLAG], area.x, area.y, Color::White, HFONT_CENTERED_NONE, label.c_str());
	}
}

void C_TextSeperator::update() {

}

void C_TextSeperator::click() {

}