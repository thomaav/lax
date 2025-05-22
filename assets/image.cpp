#include <third_party/stb/stb_image.h>

#include <utils/util.h>

#include "image.h"

namespace assets
{

void image::load(const char *path)
{
	int channels;
	unsigned char *data = stbi_load(path, &m_width, &m_height, &channels, STBI_rgb_alpha);
	assert_if(nullptr == data, "Could not stbi_load image %s\n", path);
	m_data.assign(data, data + m_width * m_height * 4);
	stbi_image_free(data);
}

} /* namespace assets */
