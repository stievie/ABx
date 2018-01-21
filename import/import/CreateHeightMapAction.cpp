#include "stdafx.h"
#include "CreateHeightMapAction.h"
#pragma warning(push)
#pragma warning(disable: 4244 4456)
#include <stb_image.h>
#pragma warning(pop)

void CreateHeightMapAction::Execute()
{
    int width, height, components;
    stbi_uc* data = stbi_load(file_.c_str(), &width, &height, &components, 0);

    if (!data)
    {
        std::cout << "Error loading file " << file_ << std::endl;
        return;
    }

    free(data);
}
