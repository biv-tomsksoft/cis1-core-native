/*
 *    TomskSoft CIS1 Core
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include <iostream>

#include "context.h"
#include "session.h"
#include "set_value.h"
#include "logger.h"
#include "os.h"
#include "webui_session.h"
#include "cis_version.h"

void usage()
{
    std::cout << "Usage:" << "\n"
              << "setvalue value_name value_value" << std::endl;
}

int main(int argc, char *argv[])
{
    if(argc == 2 && strcmp(argv[1], "--version") == 0)
    {
        print_version();

        return EXIT_SUCCESS;
    }

    cis1::os std_os;

    std::error_code ec;

    auto ctx_opt = cis1::init_context(ec, std_os);
    if(ec)
    {
        std::cerr << ec.message() << std::endl;

        return 1;
    }
    auto& ctx = ctx_opt.value();

    auto webui_session = init_webui_session(ctx);

    if(webui_session != nullptr)
    {
        init_webui_log(webui_session);
    }
    init_cis_log(ctx);

    auto session_opt = cis1::invoke_session(ctx, ec, std_os);
    if(ec)
    {
        std::cerr << ec.message() << std::endl;
        cis_log() << "action=\"error\" " << ec.message() << std::endl;

        return 1;
    }
    auto& session = session_opt.value();

    if(webui_session)
    {
        webui_session->auth(session);
    }

    init_session_log(ctx, session);

    if(argc != 3)
    {
        std::cerr << "Wrong arguments" << std::endl;
        tee_log()  << "action=\"error\" "
                    << "Wrong args count in setvalue" << std::endl;

        return 1;
    }

    if(session.opened_by_me())
    {
        std::cerr << "Cant start setvalue outside of session" << std::endl;
        tee_log()  << "action=\"error\" "
                    << "Cant set value outside the session" << std::endl;

        return 1;
    }

    cis1::set_value(ctx, session, argv[1], argv[2], ec, std_os);
    if(ec)
    {
        std::cerr << ec.message() << std::endl;
        tee_log() << "action=\"error\" " << ec.message() << std::endl;

        return 1;
    }

    session_log()   << "action=\"setvalue\" \""
                    << argv[1] << "\"=\"" << argv[2] << "\"" << std::endl;

    return 0;
}
