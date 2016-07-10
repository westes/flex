#include <iostream>
#include <string>

#include "simple_m4.h"

#define THROW throw "err"

#define SUCCESS exit(0)
#define ERROR THROW

#define CMP(r, y) \
    if ((r) != (y)) \
    { \
        std::cout << "result: '" << (r) << "'"; \
        ERROR; \
    } \
    else \
        SUCCESS

#define LINE(x) processor.processLine(x)
#define LINE_CMP(x, y) \
    do \
    { \
        auto r = LINE(x); \
        CMP(r, y); \
    } \
    while (0)

const simple_m4::M4Functions preprocess_functions = {
    { "m4preproc_changecom", simple_m4::MacroType::changecom },
    { "m4preproc_define", simple_m4::MacroType::define },
    { "m4preproc_include", simple_m4::MacroType::include },
};

const simple_m4::M4Functions main_functions = {
    { "m4_dnl",simple_m4::MacroType::dnl },
    { "m4_changecom",simple_m4::MacroType::changecom },
    { "m4_changequote",simple_m4::MacroType::changequote },
    { "m4_define",simple_m4::MacroType::define },
    { "m4_ifdef",simple_m4::MacroType::ifdef },
    { "m4_ifelse",simple_m4::MacroType::ifelse },
    { "m4_undefine",simple_m4::MacroType::undefine },
};

int main(int argc, char *argv[])
try
{
    int t = 10;

    if (argc == 2)
        t = std::stoi(argv[1]);

    simple_m4::M4 processor;
    LINE("m4preproc_changecom");
    LINE("m4_changecom");

    auto t1 = [&]
    {
        processor.setFunctions(preprocess_functions);
        LINE("m4preproc_define(`M4_GEN_PREFIX',``m4_define(yy[[$1]], [[M4_YY_PREFIX[[$1]]m4_ifelse($'`#,0,,[[($'`@)]])]])'')");
    };

    auto t2 = [&]
    {
        processor.setFunctions(main_functions);
        LINE("m4_changequote([[, ]])");
        LINE("m4_define([[M4_YY_PREFIX]], [[yy]])");
        LINE("m4_define(yy[[_create_buffer]], [[M4_YY_PREFIX[[_create_buffer]]m4_ifelse($#,0,,[[($@)]])]])");
    };

    std::string s;
    switch (t)
    {
    case 1:
        t1();
        LINE_CMP("M4_GEN_PREFIX(`_create_buffer')",
            "m4_define(yy[[_create_buffer]], [[M4_YY_PREFIX[[_create_buffer]]m4_ifelse($#,0,,[[($@)]])]])");
        break;
    case 2:
        t2();
        LINE_CMP("yy_create_buffer", "yy_create_buffer");
        break;
    case 3:
        t2();
        CMP(processor.getDefinition("yy_create_buffer"),
            "M4_YY_PREFIX[[_create_buffer]]m4_ifelse($#,0,,[[($@)]])");
        break;
    case 4:
        t2();
        LINE_CMP("yy_create_buffer(text i)",
            "yy_create_buffer(text i)");
        break;
    case 5:
        t2();
        LINE_CMP("'$'", "'$'");
        break;
    case 6:
        t2();
        LINE_CMP("'\0'", "'\0'");
        break;
    case 7:
        t2();
        LINE("m4_define([[M4_YY_CALL_LAST_ARG]], [[]])");
        LINE_CMP("yy_create_buffer(yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG);",
            "yy_create_buffer(yyin,YY_BUF_SIZE );");
        break;
    case 8:
        t2();
        LINE_CMP("yy_create_buffer(yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG);",
            "yy_create_buffer(yyin,YY_BUF_SIZE M4_YY_CALL_LAST_ARG);");
        break;
    case 9:
        t2();
        LINE("m4_define([[M4_YY_NOT_IN_HEADER]])");
        LINE_CMP(R"m(
m4_ifdef( [[M4_YY_NOT_IN_HEADER]],
[[
#define yy_set_interactive(is_interactive) \
	{ \
	if ( ! YY_CURRENT_BUFFER ){ \
        yyensure_buffer_stack (M4_YY_CALL_ONLY_ARG); \
		YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer( yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG); \
	} \
	YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
	}
]])
)m", R"m(

#define yy_set_interactive(is_interactive) \
	{ \
	if ( ! YY_CURRENT_BUFFER ){ \
        yyensure_buffer_stack (M4_YY_CALL_ONLY_ARG); \
		YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer(yyin,YY_BUF_SIZE M4_YY_CALL_LAST_ARG); \
	} \
	YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
	}

)m");
        break;
    case 10:
        t2();
        LINE("m4_define(yy[[in]], [[M4_YY_PREFIX[[in]]m4_ifelse($#,0,,[[($@)]])]])");
        LINE("m4_define([[M4_YY_CALL_LAST_ARG]], [[]])");
        LINE_CMP("yy_create_buffer(yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG);",
            "yy_create_buffer(yyin,YY_BUF_SIZE );");
        break;
    case 11:
        t2();
        LINE("m4_define(yy[[in]], [[M4_YY_PREFIX[[in]]m4_ifelse($#,0,,[[($@)]])]])");
        LINE_CMP("yy_create_buffer(yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG);",
            "yy_create_buffer(yyin,YY_BUF_SIZE M4_YY_CALL_LAST_ARG);");
        break;
    case 12:
        t2();
        LINE("m4_define(yy[[in]], [[M4_YY_PREFIX[[in]]m4_ifelse($#,0,,[[($@)]])]])");
        LINE("m4_define([[M4_YY_NOT_IN_HEADER]])");
        LINE_CMP(R"m(m4_ifdef( [[M4_YY_NOT_IN_HEADER]],
[[
#define yy_set_interactive(is_interactive) \
	{ \
	if ( ! YY_CURRENT_BUFFER ){ \
        yyensure_buffer_stack (M4_YY_CALL_ONLY_ARG); \
		YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer( yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG); \
	} \
	YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
	}
]]))m", R"m(
#define yy_set_interactive(is_interactive) \
	{ \
	if ( ! YY_CURRENT_BUFFER ){ \
        yyensure_buffer_stack (M4_YY_CALL_ONLY_ARG); \
		YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer(yyin,YY_BUF_SIZE M4_YY_CALL_LAST_ARG); \
	} \
	YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
	}
)m");
        break;
    case 13:
        t2();
        LINE("m4_define(yy[[in]], [[M4_YY_PREFIX[[in]]m4_ifelse($#,0,,[[($@)]])]])");
        LINE_CMP("yy_create_buffer(yyin, YY_BUF_SIZE M4_YY_CALL_LAST_ARG);",
            "yy_create_buffer(yyin,YY_BUF_SIZE M4_YY_CALL_LAST_ARG);");
        break;
    default:
        SUCCESS;
        break;
    }

    return 0;
}
catch (...)
{
    return 1;
}
