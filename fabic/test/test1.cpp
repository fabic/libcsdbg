/**
 * libcsdbg: fabic/test/test1.cpp
 *
 * /me tryin' to debug a situation with Boost's program_options library failing
 * to “convert” program arguments (and anyway I don't want any conversion BTW,
 * but it insists on converting char[] to some std::wstring _with the unexpected
 * intent of converting it back to std::string, it appears).
 *
 * Buid with :
 *     $CXX -std=c++11 -lboost_program_options -o test1 fabic/test/test1.cpp
 *
 * FabiC 2016-06-16 https://github.com/fabic/libcsdbg/
 */
#include <boost/program_options.hpp>
#include <iostream>

namespace fabic {
    namespace plays {

        namespace po   = boost::program_options;

        using program_arguments = po::variables_map;

        /**
         * MAIN
         */
        int main(program_arguments& args) {

            std::cout << "Hello world! (libcsdbg test 1)" << std::endl;

            return 0;
        }

        /**
         * Resort to Boost's program_options lib. for parsing program command
         * line arguments.
         *
         * @param  argc [description]
         * @param  argv [description]
         * @return      [description]
         */
        po::variables_map
        process_program_arguments(int argc, const char *const argv[])
        {
            namespace po = boost::program_options;

            // This is needed so for setting up the
            // “ conversion facet according to the user's selected locale ”
            // based on the LANG environment variable. Else an exception is
            // thrown from some deep abyss "character conversion failed".
            //   http://www.boost.org/doc/libs/1_61_0/doc/html/program_options/howto.html#idp308994400
            std::locale::global(std::locale(""));

            po::options_description desc("Allowed options");
            desc.add_options()
                    ("help", "produce help message")
                    ("address", po::value<char>(), "Hostname or IP address")
                    ("port", po::value<char>(), "TCP port number");

            po::positional_options_description posit;
            posit.add("extra", -1);

            po::variables_map args;

            auto parser = po::command_line_parser(argc, argv);

            parser
                    .options(desc)
                    .positional(posit);

            auto parsed_options = parser.run();

            po::store(parsed_options, args, true);

            po::notify(args);

            return args;
        }
    } // plays ns.
} // fabic ns.



/**
 * Invoqued by main() within a try-catch
 * (caught exception here is forwarded).
 */
int main_bis(int argc, const char *argv[])
{

    try {
        auto args = fabic::plays::process_program_arguments(argc, argv);
        auto exit_status = fabic::plays::main(args);
        return exit_status;
    }
    catch (std::exception& e) {
        std::cerr << "OUPS! Caught sthg: " << e.what() << std::endl;
        std::cerr << "      (forwarding exception)" << std::endl;
        throw e;
    }

    return 127;
}

/**
 * C-style main() actual entry point.
 */
int main(int argc, const char *argv[])
{
    try {
        return main_bis(argc, argv);
    }
    catch (std::exception& e) {
        std::cerr << "HEY! Caught an exception : " << e.what() << std::endl;
        std::cerr << "     (forwarding it, will quite probably end up handled by"
                     " std::terminate() somehow...)" << std::endl;
        throw e;
    }

    return 127;
}
