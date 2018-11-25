#include "..\cmenu.hpp"
#include <math.h>
#include <algorithm>

#define gg 19

C_Dropdown::C_Dropdown(std::string n_label, int * n_selected, std::vector< std::string > n_items, std::vector< C_ControlFlag * > n_flags) {
	parent = g_cfg.menu.last_group;
	label = n_label;
	selected = n_selected;
	items = n_items;
	using_pointer = false;

	flags = n_flags;

	area.h = gg;

	if (n_label != "") {
		height_offset = 14;
		area.h += 14;
	}

	parent->add_control(this);
}

C_Dropdown::C_Dropdown(std::string n_label, int * n_selected, std::vector< std::string > * n_items, std::vector< C_ControlFlag * > n_flags) {
	parent = g_cfg.menu.last_group;
	label = n_label;
	selected = n_selected;
	items_pointer = n_items;
	using_pointer = true;

	flags = n_flags;

	area.h = gg;

	if (n_label != "") {
		height_offset = 14;
		area.h += 14;
	}

	parent->add_control(this);
}

void C_Dropdown::draw() {
	rect_t n_area = rect_t(
		area.x,
		area.y + height_offset,
		area.w,
		gg
	);

	POINT mouse; GetCursorPos(&mouse);
	rect_t item_region = rect_t(n_area.x, n_area.y, n_area.w, gg);

	if (label != "")
		render::get().text(fonts[ESPFLAG], area.x, area.y, Color::White, HFONT_CENTERED_NONE, label.c_str());

	if (item_region.contains_point(mouse)) {
		render::get().gradient(n_area.x, n_area.y, n_area.w, gg, Color{ 65, 65, 65, 255 }, Color::Black, GRADIENT_VERTICAL);
	}
	else
		render::get().gradient(n_area.x, n_area.y, n_area.w, gg, Color{ 55, 55, 55, 255 }, Color::Black, GRADIENT_VERTICAL);

	render::get().rect(n_area.x, n_area.y, n_area.w, gg, Color::Black);

	// lambda for the arrow
	auto arrow = [](int x, int y) {
		for (auto i = 5; i >= 2; --i) {
			auto offset = 5 - i;

			render::get().line(x + offset, y + offset, x + offset + math::clamp(i - offset, 0, 5), y + offset, Color(153, 153, 153));
		}
	};

	arrow(n_area.x + n_area.w - 10, n_area.y + 9);

	if (!items.size()) {
		return;
	}

	render::get().text(fonts[ESPFLAG], n_area.x + 10, n_area.y + 10, Color(153, 153, 153), HFONT_CENTERED_Y, items[*selected].c_str());

	if (!open) {
		return;
	}

	n_area.y += 21;
	n_area.h -= gg;

	render::get().gradient(n_area.x, n_area.y, area.w, items.size() * gg, Color{ 55, 55, 55, 255 }, Color::Black, GRADIENT_VERTICAL);

	for (int i = 0; i < items.size(); i++) {
		auto & current = items.at(i);

		rect_t item_region = rect_t(n_area.x, n_area.y + (gg * i), n_area.w, gg);

		if (item_region.contains_point(mouse)) {
			render::get().gradient(n_area.x, n_area.y + (gg * i), n_area.w, gg, Color{ 40, 40, 40, 255 }, Color::Black, GRADIENT_VERTICAL);
		}

		if (i == *selected) {
			render::get().text(fonts[ESPFLAG], area.x + 10, n_area.y + (gg * i) + 10, g_cfg.menu.menu_theme[0], HFONT_CENTERED_Y, current.c_str());
		}
		else {
			render::get().text(fonts[ESPFLAG], area.x + 10, n_area.y + (gg * i) + 10, Color(153, 153, 153), HFONT_CENTERED_Y, current.c_str());
		}
	}

	render::get().rect(n_area.x, n_area.y, area.w, items.size() * gg, Color::Black);
}

void C_Dropdown::update() {
	if (using_pointer)
		items = *items_pointer;

	if (!open || !items.size())
		return;

	POINT mouse; GetCursorPos(&mouse);

	parent->focus_control(this);

	for (int i = 0; i < items.size(); i++) {
		rect_t item_region = rect_t(area.x, area.y + gg + (height_offset)+(i * gg), area.w, gg);

		if (menu::get().key_press(VK_LBUTTON) && item_region.contains_point(mouse)) {
			open = false;
			*selected = i;

			parent->reset_block();
		}
	}

	rect_t full_area = rect_t(area.x, area.y + height_offset, area.w, area.h + (items.size() * gg));
	if (menu::get().key_press(VK_LBUTTON) && !full_area.contains_point(mouse)) {
		open = false;
		parent->reset_block();
	}
}

void C_Dropdown::click() {
	if (open)
		parent->reset_block();

	open = !open;
}