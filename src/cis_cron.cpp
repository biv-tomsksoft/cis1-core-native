#include <iostream>
#include <iomanip>

#include "context.h"
#include "logger.h"
#include "os.h"
#include "cron.h"

void usage()
{
    std::cout << "Usage:" << '\n'
              << std::setw(16) << std::right << "start daemon: "
              << "--daemon"
              << '\n'
              << std::setw(16) << std::right << "list: "
              << "--list \"mask\""
              << '\n'
              << std::setw(16) << std::right << "add: "
              << "--add \"cron_expr\" \"project/job\""
              << '\n'
              << std::setw(16) << std::right << "del: "
              << "--del \"cron_expr\" \"project/job\""
              << std::endl;
}

int add(cis1::context_interface& ctx,
        const char* cron,
        const char* job,
        cis1::os_interface& os)
{
    std::error_code ec;

    auto cronexpr_opt = make_cron(cron, ec);

    if(!cronexpr_opt)
    {
        std::cout << ec.message() << std::endl;

        return EXIT_FAILURE;
    }

    auto opt_cron_list = load_cron_list(
            ctx.base_dir() / "core" / "crons",
            ec,
            os);
    if(!opt_cron_list)
    {
        cis_log() << "action=\"error\" "
                  << ec.message() << std::endl;

        std::cout << ec.message() << std::endl;

        return EXIT_FAILURE;
    }
    auto& cron_list = opt_cron_list.value();

    cron_list.add(cron_entry{job, cron});

    cron_list.save(ec);
    
    if(ec)
    {
        std::cout << ec.message() << std::endl;

        return EXIT_FAILURE;
    }

    notify_daemon();

    return EXIT_SUCCESS;
}

int del(cis1::context_interface& ctx,
        const char* cron,
        const char* job,
        cis1::os_interface& os)
{
    std::error_code ec;

    auto cronexpr_opt = make_cron(cron, ec);

    if(!cronexpr_opt)
    {
        std::cout << "Invalid cron expression" << std::endl;

        return EXIT_FAILURE;
    }

    auto opt_cron_list = load_cron_list(
            ctx.base_dir() / "core" / "crons",
            ec,
            os);
    if(!opt_cron_list)
    {
        cis_log() << "action=\"error\" " << ec.message() << std::endl;

        std::cout << ec.message() << std::endl;

        return EXIT_FAILURE;
    }
    auto& cron_list = opt_cron_list.value();

    if(cron_list.del(cron_entry{job, cron}) != 1)
    {
        std::cout << "Entry doesn't exists" << std::endl;

        return EXIT_FAILURE;
    }

    cron_list.save(ec);
    
    if(ec)
    {
        std::cout << ec.message() << std::endl;

        return EXIT_FAILURE;
    }

    notify_daemon();

    return EXIT_SUCCESS;
}

int list(
        cis1::context_interface& ctx,
        const char* mask,
        cis1::os_interface& os)
{
    std::error_code ec;

    auto opt_cron_list = load_cron_list(
            ctx.base_dir() / "core" / "crons",
            ec,
            os);

    if(!opt_cron_list)
    {
        cis_log() << "action=\"error\" " << "Can't load crons file." << std::endl;

        std::cout << "Can't load crons file." << std::endl;

        return EXIT_FAILURE;
    }
    auto& cron_list = opt_cron_list.value();

    std::regex rx(mask);

    for(const auto& entry : cron_list.list())
    {
        if(std::regex_match(entry.job(), rx))
        {
            std::cout << entry.job() << " "
                      << entry.expr() << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    cis1::os std_os;

    std::error_code ec;

    auto ctx_opt = cis1::init_context(ec, std_os);
    if(ec)
    {
        std::cerr << ec.message() << std::endl;

        return 1;
    }
    auto& ctx = ctx_opt.value();

    init_cis_log(ctx);

    switch(argc)
    {
        case 2:
        {
            if(strcmp(argv[1], "--usage") == 0)
            {
                usage();

                return EXIT_SUCCESS;
            }
            else if(strcmp(argv[1], "--daemon") == 0)
            {
                return start_daemon(ctx, std_os);
            }
            else
            {
                usage();

                return EXIT_FAILURE;
            }
        }
        case 3:
        {
            if(strcmp(argv[1], "--list") == 0)
            {
                if(validate_mask(argv[2]))
                {
                    return list(ctx, argv[2], std_os);
                }
                else
                {
                    std::cout << "Invalid mask." << std::endl;

                    return EXIT_FAILURE;
                }
            }
            else
            {
                usage();

                return EXIT_SUCCESS;
            }
        }
        case 4:
        {
            if(strcmp(argv[1], "--add") == 0)
            {
                return add(ctx, argv[2], argv[3], std_os);
            }
            else if(strcmp(argv[1], "--del") == 0)
            {
                return del(ctx, argv[2], argv[3], std_os);
            }
            else
            {
                usage();

                return EXIT_FAILURE;
            }
        }
        default:
        {
            usage();

            return EXIT_FAILURE;
        }
    }
}
