#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/imgui/imgui.h>
#pragma clang diagnostic pop
// clang-format on

#include <utils/log.h>

class console_logger : public loggerp
{
public:
	console_logger() = default;
	~console_logger() = default;

	console_logger(const console_logger &) = delete;
	console_logger operator=(const console_logger &) = delete;

	void log(const char *str) override;

	ImGuiTextBuffer m_buffer = {};
};
