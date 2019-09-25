#include <iostream>

#include "context.h"
#include "session.h"
#include "get_param.h"
#include "logger.h"
#include "os.h"

void usage()
{
    std::cout << "Usage:" << "\n"
              << "getparam value_name" << std::endl;
}

int main(int argc, char *argv[])
{
    cis1::os std_os;

    std::error_code ec;

    auto ctx_opt = cis1::init_context(ec, std_os);
    if(ec)
    {
        std::cerr << ec.message() << std::endl;;

        return 1;
    }
    auto& ctx = ctx_opt.value();

    init_cis_log(ctx);

    auto session_opt = cis1::invoke_session(ctx, ec, std_os);
    if(ec)
    {
        cis_log() << "action=\"error\" " << ec.message() << std::endl;

        return 1;
    }
    auto& session = session_opt.value();

    init_session_log(ctx, session);

    if(argc != 2)
    {
        tie_log()  << "action=\"error\" "
                    << "Wrong args count in getparam" << std::endl;

        return 1;
    }

    if(session.opened_by_me())
    {
        tie_log()  << "action=\"error\" "
                    << "Cant get param outside the session" << std::endl;

        return 1;
    }

    auto param_opt = cis1::get_param(ctx, session, argv[1], ec, std_os);
    if(ec)
    {
        tie_log() << "action=\"error\" " << ec.message() << std::endl;

        return 1;
    }

    std::cout << param_opt.value() << std::endl;

    return 0;
}
