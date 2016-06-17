/**
 * libcsdbg: fabic/test/test1.cpp
 *
 * /me tryin' to debug a situation with Boost's program_options library failing
 * to “convert” program arguments (and anyway I don't want any conversion BTW,
 * but it insists on converting char[] to some std::wstring _with the unexpected
 * intent of converting it back to std::string, it appears).
 *
 * Buid with :
 *     $CXX -g -rdynamic -std=c++14 -frtti -L ~/boost-1.61.0-$CC/lib -I.. -L. -Wl,-rpath=. -lboost_program_options -lcsdbg -ldl -lbfd -lpthread -o test1 fabic/test/test1.cpp
 *
 * FabiC 2016-06-16 https://github.com/fabic/libcsdbg/
 */

#include <dlfcn.h>
#include <cxxabi.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <typeinfo>

// We pass -I.. for compiling so that we may do this
// (because include/... isn't "namespaced").
//#include "libcsdbg/include/tracer.hpp"

namespace fabic {
    namespace plays {

        namespace po   = boost::program_options;

        using program_arguments = po::variables_map;

        /**
         * MAIN which accepts
         */
        int main(program_arguments& args) {

            std::cout << "Hello world, you reached this point, can't believe it!"
                         " (libcsdbg test 1)" << std::endl;

            return 0;
        }


        /**
         * Resort to Boost's program_options lib.
         * for parsing command line arguments.
         *
         * @param  argc [description]
         * @param  argv [description]
         * @return      [description]
         */
        program_arguments
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


        void terminate_handler() {
            std::cerr << "Hey! that's terminate!"
            << '(' << __FILE__
            << ':' << __LINE__ << ')' << std::endl;
        }


        extern "C" {
            void __cxa_throw(void *ex, std::type_info *info, void (*dest)(void *)) {
                // exception_name = demangle(reinterpret_cast<const std::type_info*>(info)->name());
                // last_size = backtrace(last_frames, sizeof last_frames/sizeof(void*));

                typedef void (*const cxa_throw_func_ptr_type)(void *_ex, std::type_info *_info, void (*_dest)(void *))
                        __attribute__ ((noreturn));

                // man dlsym
                auto rethrow = (cxa_throw_func_ptr_type) dlsym(RTLD_NEXT, "__cxa_throw");

                if (rethrow == NULL) {
                    abort();
                }

                auto exception_name = [&info]() {
                    if (info == nullptr)
                        return std::string("no_exception_type_info");

                    int status = 0xdeadbeef;
                    char *name = abi::__cxa_demangle(
                            info->name(),
                            nullptr, // buffer
                            nullptr, // buffer length
                            &status  // demangling exit status
                    );

                    if (name == nullptr)
                        return std::string("unknown_exception");

                    assert( status == 0 );

                    auto retv = std::string(name);

                    free( name );

                    return retv;
                }();

                std::cout << "Thrown ex. " << exception_name << std::endl;

                rethrow(ex, info, dest);
            }
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
    std::set_terminate(fabic::plays::terminate_handler);

//    static void (*const rethrow)(void*,void*,void(*)(void*)) __attribute__ ((noreturn))
//            = (void (*)(void*,void*,void(*)(void*)))dlsym(RTLD_NEXT, "__cxa_throw");

    // Init. libcsdbg's tracer thing :
    // using namespace csdbg;
    //
    // tracer *iface = tracer::interface();
    // if ( unlikely(iface == NULL) ) {
    //     std::cerr << "FAILED! couldn't initialize libcsdbg's tracer thing." << std::endl;
    //     return 126;
    // }


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
