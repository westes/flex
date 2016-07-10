/*  prepare_skel.cpp - skeleton preparation file
 *
 *  Copyright (c) 2016 Egor Pugin
 *
 *  This file is part of flex.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */

#include <fstream>
#include <iostream>

#include <config.h>

#include "flexdef.h"
#include "simple_m4.h"

const simple_m4::M4Functions preprocess_functions =
{
    { "m4preproc_changecom", simple_m4::MacroType::changecom },
    { "m4preproc_define", simple_m4::MacroType::define },
    { "m4preproc_include", simple_m4::MacroType::include },
};

int main(int argc, char *argv[])
try
{
    if (argc != 3)
    {
        std::cerr << "Usage: prepare_skel flex.skl skel.cpp" << "\n";
        return 1;
    }

    auto s = simple_m4::read_file(argv[1]);
    Context ctx;
    ctx.addLine(s);
    ctx.splitLines();

    simple_m4::M4 processor;
    processor.setFunctions(preprocess_functions);

    int major, minor, subminor;
    sscanf(FLEX_VERSION, "%d.%d.%d", &major, &minor, &subminor);
    processor.addDefinition("FLEX_MAJOR_VERSION", std::to_string(major));
    processor.addDefinition("FLEX_MINOR_VERSION", std::to_string(minor));
    processor.addDefinition("FLEX_SUBMINOR_VERSION", std::to_string(subminor));

    for (auto &line : ctx.getLinesRef())
        line.text = processor.processLine(line.text);

    ctx.splitLines();
    //ctx.setMaxEmptyLines(1);

    for (auto &line : ctx.getLinesRef())
    {
        line.text = "R\"skel(" + line.text + ")skel\",";
        line.n_indents = 1;
    }

    ctx.before().addLine("/* File created from flex.skl via prepare_skel */");
    ctx.before().addLine();
    ctx.before().addLine("#include <flexdef.h>");
    ctx.before().addLine();
    ctx.before().addLine("const char *skel[] = {");

    ctx.after().increaseIndent();
    ctx.after().addLine("0");
    ctx.after().decreaseIndent();
    ctx.after().addLine("};");

    std::ofstream ofile(argv[2]);
    if (ofile)
        ofile << ctx.getText();

    return 0;
}
catch (const std::exception &e)
{
    std::cerr << e.what() << "\n";
    return 1;
}
