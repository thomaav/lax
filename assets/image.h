#pragma once

#include <vector>

#include <utils/type.h>

namespace assets
{

class image
{
public:
	image() = default;
	~image() = default;

	image(const image &) = delete;
	image operator=(const image &) = delete;

	void load(const char *path);

	std::vector<u8> m_data = {};
	int m_width = -1;
	int m_height = -1;

private:
};

} /* namespace assets */
