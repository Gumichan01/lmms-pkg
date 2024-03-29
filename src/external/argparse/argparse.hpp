#ifndef ARGPARSE_HPP_
#define ARGPARSE_HPP_

#if __cplusplus >= 201103L
#include <unordered_map>
typedef std::unordered_map<std::string, size_t> IndexMap;
#else
#include <map>
typedef std::map<std::string, size_t> IndexMap;
#endif
#include <string>
#include <vector>
#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cassert>
#include <algorithm>

namespace argparse {
    // Modified from https://github.com/davisking/dlib/blob/master/dlib/algs.h
    template <typename T> struct is_standard_type        { const static bool value = false; };

    template <> struct is_standard_type<bool>             { const static bool value = true; };
    template <> struct is_standard_type<float>           { const static bool value = true; };
    template <> struct is_standard_type<double>          { const static bool value = true; };
    template <> struct is_standard_type<long double>     { const static bool value = true; };
    template <> struct is_standard_type<short>           { const static bool value = true; };
    template <> struct is_standard_type<int>             { const static bool value = true; };
    template <> struct is_standard_type<long>            { const static bool value = true; };
    template <> struct is_standard_type<unsigned short>  { const static bool value = true; };
    template <> struct is_standard_type<unsigned int>    { const static bool value = true; };
    template <> struct is_standard_type<unsigned long>   { const static bool value = true; };
    template <> struct is_standard_type<char>            { const static bool value = true; };
    template <> struct is_standard_type<signed char>     { const static bool value = true; };
    template <> struct is_standard_type<unsigned char>   { const static bool value = true; };
    template <> struct is_standard_type<std::string>   { const static bool value = true; };

    // Copied from https://github.com/davisking/dlib/blob/master/dlib/enable_if.h
    template <bool B, class T = void>
    struct enable_if_c { typedef T type; };

    template <class T>
    struct enable_if_c<false, T> {};

    template <class Cond, class T = void>
    struct enable_if : public enable_if_c<Cond::value, T> {};

    template <bool B, class T = void>
    struct disable_if_c { typedef T type; };

    template <class T>
    struct disable_if_c<true, T> {};

    template <class Cond, class T = void>
    struct disable_if : public disable_if_c<Cond::value, T> {};

    template <typename T>
    T castTo(const std::string& item) {
        std::istringstream sin(item);
        T value;
        sin >> value;
        return value;
    }

    template <typename T>
    std::string toString(const T& item) {
        std::ostringstream sout;
        sout << item;
        return sout.str();
    }

    void remove_space(std::string& str) {
        str.erase(
            std::remove_if(
                str.begin(), str.end(),
                [](unsigned char x){ return std::isspace(x); }
            ),
            str.end()
        );
    }

    void strip_brackets(std::string& str) {
        auto first_bracket = str.find_first_of('[');
        if(first_bracket == std::string::npos) {
            std::ostringstream sout;
            sout << "Could not find a left bracket in " << str;
            throw std::runtime_error(sout.str());
        }
        str.erase(str.begin() + first_bracket);

        auto last_bracket = str.find_last_of(']');
        if(last_bracket == std::string::npos) {
            std::ostringstream sout;
            sout << "Could not find a right bracket in " << str;
            throw std::runtime_error(sout.str());
        }
        str.erase(str.begin() + last_bracket);
    }

    /// It stores the argument provided by the user and parsed by the application
    struct ParsedArgument {
        const std::string short_name = "";  // example: '-v'
        const std::string long_name = "";   // example: '-verbose'
        const std::string name = "";        // example: 'verbose'
    };

    /*! @class ArgumentParser
     *  @brief A simple command-line argument parser based on the design of
     *  python's parser of the same name.
     *
     *  ArgumentParser is a simple C++ class that can parse arguments from
     *  the command-line or any array of strings. The syntax is familiar to
     *  anyone who has used python's ArgumentParser:
     *  \code
     *    // create a parser and add the options
     *    ArgumentParser parser;
     *    parser.addArgument("-n", "--name", 1);
     *    parser.addArgument("-v", "--verbose", 0);
     *    parser.addArgument("--inputs", '+');
     *
     *    // parse the command-line arguments
     *    parser.parse(argc, argv);
     *
     *    // get the inputs and iterate over them
     *    string name = parser.retrieve("name");
     *    vector<string> inputs = parser.retrieve<vector<string>>("inputs");
     *  \endcode
     *
     */
    class ArgumentParser {
    private:
        class Argument;
        typedef std::string String;
        typedef std::vector<String> StringVector;
        typedef std::vector<Argument> ArgumentVector;

        // --------------------------------------------------------------------------
        // Argument
        // --------------------------------------------------------------------------
        static String delimit(const String& name) noexcept {
            return String(std::min(name.size(), (size_t)2), '-').append(name);
        }
        static String strip(const String& name) {
            size_t begin = 0;
            begin += name.size() > 0 ? name[0] == '-' : 0;
            begin += name.size() > 3 ? name[1] == '-' : 0;
            return name.substr(begin);
        }
        static String upper(const String& in) {
            String out(in);
            std::transform(out.begin(), out.end(), out.begin(), ::toupper);
            return out;
        }
        static String escape(const String& in) {
            String out(in);
            if (in.find(' ') != std::string::npos) out = String("\"").append(out).append("\"");
            return out;
        }

        struct Argument {
            Argument() : short_name(""), name(""), optional(true), fixed_nargs(0), fixed(true) {}
            Argument(const String& _short_name, const String& _name, bool _optional, char nargs)
                    : short_name(_short_name), name(_name), optional(_optional) {
                if (nargs == '+' || nargs == '*') {
                    variable_nargs = nargs;
                    fixed = false;
                } else {
                    fixed_nargs = nargs;
                    fixed = true;
                }
            }
            String short_name;
            String name;
            bool optional;
            union {
                size_t fixed_nargs;
                char variable_nargs;
            };
            bool fixed;
            bool specified = false;
            String canonicalName() const { return (name.empty()) ? short_name : name; }
            String toString(bool named = true) const {
                std::ostringstream s;
                const String& uname = name.empty() ? upper(strip(short_name)) : upper(strip(name));
                if (named && optional) s << "[";
                if (named) s << canonicalName();
                if (fixed) {
                    size_t N = std::min((size_t)3, fixed_nargs);
                    for (size_t n = 0; n < N; ++n) s << " " << uname;
                    if (N < fixed_nargs) s << " ...";
                }
                if (!fixed) {
                    s << " ";
                    if (variable_nargs == '*') s << "[";
                    s << uname << " ";
                    if (variable_nargs == '+') s << "[";
                    s << uname << "...]";
                }
                if (named && optional) s << "]";
                return s.str();
            }
        };

        ArgumentParser& insertArgument(const Argument& arg) {
            size_t N = arguments_.size();
            arguments_.push_back(arg);
            if (arg.fixed && arg.fixed_nargs <= 1) {
                if (arg.fixed_nargs == 1) {
                    variables_.push_back(String());
                } else { // arg.fixed_nargs == 0 because arg.fixed_nargs <= 1 and is type is unsigned
                    variables_.push_back(String("0"));
                }
            } else {
                variables_.push_back(String());
            }
            if (!arg.short_name.empty()) {
                    index_[arg.short_name] = N;
            }

            if (!arg.name.empty()) {
                    index_[arg.name] = N;
            }

            if (!arg.optional) {
                    required_++;
            }

            return *this;
        }

        // --------------------------------------------------------------------------
        // Error handling
        // --------------------------------------------------------------------------
        void argumentError(const std::string& msg, bool show_usage = false) {
            if (use_exceptions_) throw std::invalid_argument(msg);
            std::cerr << "ArgumentParser error: " << msg << "\n";
            if (show_usage) std::cerr << usage() << "\n";
            exit(-5);
        }

        // --------------------------------------------------------------------------
        // Member variables
        // --------------------------------------------------------------------------
        IndexMap index_;
        bool ignore_first_;
        bool use_exceptions_;
        size_t required_;
        String app_name_;
        String final_name_;
        ArgumentVector arguments_;
        StringVector variables_;

    public:
        ArgumentParser() : ignore_first_(true), use_exceptions_(false), required_(0) {}
        // --------------------------------------------------------------------------
        // addArgument
        // --------------------------------------------------------------------------
        ArgumentParser& appName(const String& name) {
            app_name_ = name;
            return *this;
        }
        ArgumentParser& addArgument(const String& name, char nargs = 0, bool optional = true) {
            if (name.size() > 2) {
                Argument arg("", verify(name), optional, nargs);
                return insertArgument(arg);
            } else {
                Argument arg(verify(name), "", optional, nargs);
                return insertArgument(arg);
            }
        }
        ArgumentParser& addArgument(const String& short_name, const String& name, char nargs = 0,
                         bool optional = true) {
            Argument arg(verify(short_name), verify(name), optional, nargs);
            return insertArgument(arg);
        }
        ArgumentParser& addFinalArgument(const String& name, char nargs = 1, bool optional = false) {
            final_name_ = delimit(name);
            Argument arg("", final_name_, optional, nargs);
            return insertArgument(arg);
        }
        ArgumentParser& ignoreFirstArgument(bool ignore_first) {
            ignore_first_ = ignore_first;
            return *this;
        }
        String verify(const String& name) {
            if (name.empty()) argumentError("argument names must be non-empty");
            if ((name.size() == 2 && name[0] != '-') || name.size() == 3)
                argumentError(String("invalid argument '")
                                  .append(name)
                                  .append("'. Short names must begin with '-'"));
            if (name.size() > 3 && (name[0] != '-' || name[1] != '-'))
                argumentError(String("invalid argument '")
                                  .append(name)
                                  .append("'. Multi-character names must begin with '--'"));
            return name;
        }

        // --------------------------------------------------------------------------
        // Parse
        // --------------------------------------------------------------------------
        ArgumentParser& parse(size_t argc, const char** argv) {
            return parse(StringVector(argv, argv + argc));
        }

        ArgumentParser& parse(const StringVector& argv) {
            // check if the app is named
            if (app_name_.empty() && ignore_first_ && !argv.empty()) app_name_ = argv[0];

            // set up the working set
            Argument active;
            Argument final = final_name_.empty() ? Argument() : arguments_[index_[final_name_]];
            size_t consumed = 0;
            size_t nrequired = final.optional ? required_ : required_ - 1;
            size_t nfinal = final.optional ? 0 : (final.fixed ? final.fixed_nargs
                                                              : (final.variable_nargs == '+' ? 1 : 0));

            // iterate over each element of the array
            for (StringVector::const_iterator in = argv.begin() + ignore_first_;
                 in < argv.end() - nfinal; ++in) {
                const String& active_name = active.canonicalName();
                const String& el = *in;

                //  check if the element is a key
                if (index_.count(el) == 0) {
                    // input
                    // is the current active argument expecting more inputs?
                    if (active.fixed && active.fixed_nargs <= consumed)
                        argumentError(String("attempt to pass too many inputs to ").append(active_name),
                                      true);
                    if (active.fixed && active.fixed_nargs == 1) {
                        variables_[index_[active_name]] = el;
                    } else {
                        String& variable = variables_[index_[active_name]];
                        StringVector value = castTo<StringVector>(variable);
                        value.push_back(el);
                        variable = toString(value);
                    }
                    consumed++;
                } else {
                    // new key!
                    arguments_[index_[el]].specified = true;
                    // has the active argument consumed enough elements?
                    if ((active.fixed && active.fixed_nargs != consumed) ||
                        (!active.fixed && active.variable_nargs == '+' && consumed < 1))
                        argumentError(String("encountered argument ")
                                          .append(el)
                                          .append(" when expecting more inputs to ")
                                          .append(active_name),
                                      true);
                    active = arguments_[index_[el]];
                    // check if we've satisfied the required arguments
                    if (active.optional && nrequired > 0)
                        argumentError(String("encountered optional argument ")
                                          .append(el)
                                          .append(" when expecting more required arguments"),
                                      true);
                    // are there enough arguments for the new argument to consume?
                    if ((active.fixed && active.fixed_nargs > (argv.end() - in - nfinal - 1)) ||
                        (!active.fixed && active.variable_nargs == '+' &&
                         !(argv.end() - in - nfinal - 1)))
                        argumentError(String("too few inputs passed to argument ").append(el), true);
                    if (!active.optional) nrequired--;
                    if (active.fixed && active.fixed_nargs == 0) {
                        variables_[index_[active.canonicalName()]] = String("1");
                    }
                    consumed = 0;
                }
            }

            for (StringVector::const_iterator in = std::max(argv.begin() + ignore_first_, argv.end() - nfinal);
                 in != argv.end(); ++in) {
                const String& el = *in;
                // check if we accidentally find an argument specifier
                if (index_.count(el))
                    argumentError(String("encountered argument specifier ")
                                      .append(el)
                                      .append(" while parsing final required inputs"),
                                  true);
                if (final.fixed && final.fixed_nargs == 1) {
                    variables_[index_[final_name_]] = el;
                } else {
                    String& variable = variables_[index_[final_name_]];
                    StringVector value = castTo<StringVector>(variable);
                    value.push_back(el);
                    variable = toString(value);
                }
                nfinal--;
            }

            // check that all of the required arguments have been encountered
            if (nrequired > 0 || nfinal > 0) {
                argumentError(String("too few required arguments passed to ").append(app_name_), true);
            }
            return *this;
        }

        // --------------------------------------------------------------------------
        // Retrieve
        // --------------------------------------------------------------------------
        String retrieve(const String& arg_name) const {
            const auto & dname = delimit(arg_name);
            if (index_.count(dname) == 0) throw std::out_of_range("Key not found");
            const size_t N = index_.at(dname);
            return variables_[N];
        }
        template <typename T>
        T retrieve(const String& arg_name) const {
            const auto & dname = delimit(arg_name);
            if (index_.count(dname) == 0) throw std::out_of_range("Key not found");
            const size_t N = index_.at(dname);
            return castTo<T>(variables_[N]);
        }

        const std::vector<ParsedArgument> retrieveParsedArguments() const {
            std::vector<ParsedArgument> args;
            for (const Argument& arg: arguments_) {
                if (arg.specified) {
                    args.push_back(ParsedArgument{arg.short_name, arg.name, strip(arg.name)});
                }
            }
            return args;
        }

        // --------------------------------------------------------------------------
        // Properties
        // --------------------------------------------------------------------------
        String usage() const {
            // premable app name
            std::ostringstream help;
            help << "Usage: " << escape(app_name_);
            const size_t indent = help.str().size();
            size_t linelength = 0;

            // get the required arguments
            for (ArgumentVector::const_iterator it = arguments_.begin(); it != arguments_.end(); ++it) {
                const Argument& arg = *it;
                if (arg.optional) continue;
                if (arg.name.compare(final_name_) == 0) continue;
                help << " ";
                const String& argstr = arg.toString();
                if (argstr.size() + linelength > 80) {
                    help << "\n" << String(indent, ' ');
                    linelength = 0;
                } else {
                    linelength += argstr.size();
                }
                help << argstr;
            }

            // get the optional arguments
            for (ArgumentVector::const_iterator it = arguments_.begin(); it != arguments_.end(); ++it) {
                const Argument& arg = *it;
                if (!arg.optional) continue;
                if (arg.name.compare(final_name_) == 0) continue;
                help << " ";
                const String& argstr = arg.toString();
                if (argstr.size() + linelength > 80) {
                    help << "\n" << String(indent, ' ');
                    linelength = 0;
                } else {
                    linelength += argstr.size();
                }
                help << argstr;
            }

            // get the final argument
            if (!final_name_.empty()) {
                const Argument& arg = arguments_.at(index_.at(final_name_));
                const String& argstr = arg.toString(false);
                if (argstr.size() + linelength > 80) {
                    help << "\n" << String(indent, ' ');
                    linelength = 0;
                } else {
                    linelength += argstr.size();
                }
                help << argstr;
            }

            return help.str();
        }
        ArgumentParser& useExceptions(bool state) {
            use_exceptions_ = state;
            return *this;
        }
        bool empty() const { return index_.empty(); }
        void clear() {
            ignore_first_ = true;
            required_ = 0;
            index_.clear();
            arguments_.clear();
            variables_.clear();
        }
        bool isRegisteredArgument(const String& arg_name) const { return index_.count(delimit(arg_name)) > 0; }
        bool hasParsedArgument(const String& arg_name) const {
            if (index_.count(delimit(arg_name)) == 0) return false;
            const size_t N = index_.at(delimit(arg_name));
            const Argument& arg = arguments_[N];
            return arg.specified;
        }
    };
}

template<typename T>
std::ostream& operator << (std::ostream& out, const std::vector<T>& v) {
    out << "[";
    for(unsigned long i = 0; i < v.size(); ++i) {
        if (i > 0)
            out << ", ";
        out << v[i];
    }
    out << "]";

    return out;
}

template<typename T>
typename argparse::enable_if<
    argparse::is_standard_type<T>,
    std::istream&
>::type operator >> (std::istream& in, std::vector<T>& v) {
    using namespace argparse;
    v.clear();

    std::string str;
    std::getline(in, str, '\n');

    if(str.empty()) return in;
    remove_space(str);
    strip_brackets(str);

    std::istringstream sin(str);
    while(sin.good())
    {
        std::string substr;
        std::getline(sin, substr, ',');
        if(!substr.empty()) v.push_back(castTo<T>(substr));
    }

    return in;
}

template<typename T>
typename argparse::enable_if<
    argparse::is_standard_type<T>,
    std::istream&
>::type operator >> (std::istream& in, std::vector<std::vector<T> >& v) {
    using namespace argparse;
    static const std::string delimiter = "]";
    v.clear();

    std::string str;
    std::getline(in, str, '\n');

    if(str.empty()) return in;
    remove_space(str);
    strip_brackets(str);

    size_t pos = 0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        std::string substr = str.substr(0, pos + 1);
        v.push_back(castTo<std::vector<T> >(substr));
        str.erase(0, pos + delimiter.length());
    }

    return in;
}

#endif
