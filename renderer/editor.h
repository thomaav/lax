#pragma once

#include <renderer/scene.h>

class editor
{
public:
	editor() = default;
	~editor() = default;

	editor(const editor &) = delete;
	editor operator=(const editor &) = delete;

	scene m_scene = {};

private:
};
