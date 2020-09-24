/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "GamesResource.h"
#include "Application.h"
#include "Version.h"
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>

namespace Resources {

bool GamesResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;

    return true;
}

GamesResource::GamesResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/games.lpp";
    styles_.insert(styles_.begin(), "vendors/css/footable.bootstrap.min.css");
    footerScripts_.push_back("vendors/js/footable.min.js");
}

void GamesResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
