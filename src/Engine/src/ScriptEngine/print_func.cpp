/*

MIT License

Copyright (c) 2021 SpehleonLP

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <Core/logger.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>
#include <array>
#include <cassert>
#include <cstring>
#include <scriptarray.h>
#include <scriptdictionary.h>
#include <string>

#include <Core/engine_loading_controllers.hpp>
#include <iostream>
#include <sstream>


class asIScriptEngine;


namespace Print
{

    typedef ::asIScriptEngine asIScriptEngine;
    typedef bool PrintNonPrimitiveType(std::ostream& dst, void const* objPtr, int typeId, int depth);
    extern PrintNonPrimitiveType* g_PrintRegisteredType;
    extern PrintNonPrimitiveType* g_PrintScriptObjectType;

    void PrintTemplate(std::ostream& dst, void const* objPtr, int typeId, int depth = 0);
    void PrintFormat(std::ostream& stream, std::string const& in, std::pair<void const*, int> const* args, int argc);

    //currently only string and array
    bool PrintAddonTypes(std::ostream& dst, void const* objPtr, int typeId, int depth);

    template<typename... Args>
    inline void PrintTemplate(std::ostream& dst, void const* objPtr, int typeId, Args... args)
    {
        PrintTemplate(dst, objPtr, typeId, 0);
        PrintTemplate(dst, std::move(args)...);
    }

    void asRegister(asIScriptEngine* engine);

};// namespace Print


Print::PrintNonPrimitiveType* Print::g_PrintRegisteredType{&Print::PrintAddonTypes};
Print::PrintNonPrimitiveType* Print::g_PrintScriptObjectType{nullptr};
class CScriptDictionary;

#define INS_1 "const ?&in = null"
#define INS_2 INS_1 ", " INS_1
#define INS_3 INS_2 ", " INS_1
#define INS_4 INS_2 ", " INS_2
#define INS_8 INS_4 ", " INS_4
#define INS_15 INS_8 ", " INS_4 ", " INS_3
#define INS_16 INS_8 ", " INS_8

#define OUTS_1 "?&out = null"
#define OUTS_2 OUTS_1 ", " OUTS_1
#define OUTS_3 OUTS_2 ", " OUTS_1
#define OUTS_4 OUTS_2 ", " OUTS_2
#define OUTS_8 OUTS_4 ", " OUTS_4
#define OUTS_15 OUTS_8 ", " OUTS_4 ", " OUTS_3
#define OUTS_16 OUTS_8 ", " OUTS_8

#define V_ARG(n, q) void q *objPtr##n, int typeId##n
#define V_ARGS_1(q) V_ARG(0, q)
#define V_ARGS_2(q) V_ARGS_1(q), V_ARG(1, q)
#define V_ARGS_4(q) V_ARGS_2(q), V_ARG(2, q), V_ARG(3, q)
#define V_ARGS_8(q) V_ARGS_4(q), V_ARG(4, q), V_ARG(5, q), V_ARG(6, q), V_ARG(7, q)
#define V_ARGS_16(q)                                                                                                             \
    V_ARGS_8(q), V_ARG(8, q), V_ARG(9, q), V_ARG(10, q), V_ARG(11, q), V_ARG(12, q), V_ARG(13, q), V_ARG(14, q), V_ARG(15, q)

#define IN_ARGS_1 V_ARGS_1(const)
#define IN_ARGS_2 V_ARGS_2(const)
#define IN_ARGS_4 V_ARGS_4(const)
#define IN_ARGS_8 V_ARGS_8(const)
#define IN_ARGS_16 V_ARGS_16(const)

#define W_ARG(n) objPtr##n, typeId##n
#define W_ARGS_1 W_ARG(0)
#define W_ARGS_2 W_ARGS_1, W_ARG(1)
#define W_ARGS_4 W_ARGS_2, W_ARG(2), W_ARG(3)
#define W_ARGS_8 W_ARGS_4, W_ARG(4), W_ARG(5), W_ARG(6), W_ARG(7)
#define W_ARGS_16 W_ARGS_8, W_ARG(8), W_ARG(9), W_ARG(10), W_ARG(11), W_ARG(12), W_ARG(13), W_ARG(14), W_ARG(15)

#define A_ARG(n)                                                                                                                 \
    std::pair<void const*, int>                                                                                                  \
    {                                                                                                                            \
        objPtr##n, typeId##n                                                                                                     \
    }
#define A_ARGS_1 A_ARG(0)
#define A_ARGS_2 A_ARGS_1, A_ARG(1)
#define A_ARGS_4 A_ARGS_2, A_ARG(2), A_ARG(3)
#define A_ARGS_8 A_ARGS_4, A_ARG(4), A_ARG(5), A_ARG(6), A_ARG(7)
#define A_ARGS_16 A_ARGS_8, A_ARG(8), A_ARG(9), A_ARG(10), A_ARG(11), A_ARG(12), A_ARG(13), A_ARG(14), A_ARG(15)

bool Print::PrintAddonTypes(std::ostream& dst, void const* objPtr, int typeId, int depth)
{
    auto engine = Engine::ScriptEngine::engine();

    int stringTypeId = engine->GetStringFactoryReturnTypeId();

    if (stringTypeId == typeId)
    {
        auto const& string = *((std::string const*) objPtr);
        dst << string;
        return true;
    }


    auto typeInfo    = engine->GetTypeInfoById(typeId);
    const char* name = typeInfo->GetName();


    if (strcmp(name, "StringView") == 0)
    {
        auto const& string = *((std::string_view const*) objPtr);
        dst << string;
        return true;
    }

    if (depth < 2 && strcmp(name, "array") == 0)
    {
        CScriptArray const* array{};

        if (typeId & asTYPEID_OBJHANDLE)
            array = *reinterpret_cast<CScriptArray* const*>(objPtr);
        else
            array = reinterpret_cast<CScriptArray const*>(objPtr);

        if (array->GetSize() == 0)
            dst << "{}";
        else
        {
            dst << "{";

            for (uint32_t i = 0; i < array->GetSize(); ++i)
            {
                Print::PrintTemplate(dst, array->At(i), array->GetElementTypeId(), depth + 1);
                dst << ((i + 1 == array->GetSize()) ? "}" : ", ");
            }
        }

        return true;
    }

    if (strcmp(name, "dictionary") == 0)
    {
        CScriptDictionary const* dictionary{};

        if (typeId & asTYPEID_OBJHANDLE)
            dictionary = *reinterpret_cast<CScriptDictionary* const*>(objPtr);
        else
            dictionary = reinterpret_cast<CScriptDictionary const*>(objPtr);

        std::string indent(depth + 1, '\t');

        dst << "{\n";

        bool printed = false;
        for (auto const& pair : *dictionary)
        {
            if (printed)
                dst << ",\n";
            printed = true;

            dst << indent << '"' << pair.GetKey() << "\":";
            Print::PrintTemplate(dst, pair.GetAddressOfValue(), pair.GetTypeId(), depth + 1);
        }

        dst << '\n' << indent.substr(0, indent.size() - 1) << '}';

        return true;
    }

    if (strcmp(name, "dictionaryValue") == 0)
    {
        auto value = reinterpret_cast<CScriptDictValue const*>(objPtr);

        Print::PrintTemplate(dst, value->GetAddressOfValue(), value->GetTypeId(), depth + 1);
    }

    return false;
}

void Print::PrintTemplate(std::ostream& dst, void const* objPtr, int typeId, int depth)
{
    switch (typeId)
    {
        case asTYPEID_VOID:
            return;
        case asTYPEID_BOOL:
            dst << ((*(bool const*) objPtr) ? "true" : "false");
            return;
        case asTYPEID_INT8:
            dst << (int64_t) * (int8_t const*) objPtr;
            return;
        case asTYPEID_INT16:
            dst << (int64_t) * (int16_t const*) objPtr;
            return;
        case asTYPEID_INT32:
            dst << (int64_t) * (int32_t const*) objPtr;
            return;
        case asTYPEID_INT64:
            dst << (int64_t) * (int64_t const*) objPtr;
            return;
        case asTYPEID_UINT8:
            dst << (uint64_t) * (uint8_t const*) objPtr;
            return;
        case asTYPEID_UINT16:
            dst << (uint64_t) * (uint16_t const*) objPtr;
            return;
        case asTYPEID_UINT32:
            dst << (uint64_t) * (uint32_t const*) objPtr;
            return;
        case asTYPEID_UINT64:
            dst << (uint64_t) * (uint64_t const*) objPtr;
            return;
        case asTYPEID_FLOAT:
            dst << (double) *(float const*) objPtr;
            return;
        case asTYPEID_DOUBLE:
            dst << (double) *(double const*) objPtr;
            return;
        default:
            break;
    }

    auto engine = Engine::ScriptEngine::engine();

    auto typeInfo = engine->GetTypeInfoById(typeId);

    if (!typeInfo)
    {
        dst << "BAD_TYPEID";
        return;
    }

    if (objPtr == nullptr)
    {
        dst << typeInfo->GetName();
        return;
    }

    if (typeInfo->GetFuncdefSignature())
    {
        auto func = *reinterpret_cast<asIScriptFunction* const*>(objPtr);
        dst << func->GetDeclaration(true, true, true);
        return;
    }

    auto enumValueCount = typeInfo->GetEnumValueCount();
    if (enumValueCount)
    {
        int value = *(uint32_t const*) objPtr;

        dst << typeInfo->GetName();

        for (uint32_t i = 0; i < enumValueCount; ++i)
        {
            int val;
            const char* text = typeInfo->GetEnumValueByIndex(i, &val);

            if (val == value)
            {
                dst << "::" << text;
            }
        }

        return;
    }

    if (typeId & asTYPEID_SCRIPTOBJECT)
    {
        if (g_PrintScriptObjectType && g_PrintScriptObjectType(dst, objPtr, typeId, depth))
            return;

        if (typeId & asTYPEID_OBJHANDLE)
        {
            dst << "@" << typeInfo->GetName() << "(" << *(void**) objPtr << ")";
        }
        else
        {
            dst << typeInfo->GetName() << "(" << objPtr << ")";
        }
        return;
    }

    if (typeId & (asTYPEID_APPOBJECT | asTYPEID_TEMPLATE))
    {
        if (g_PrintRegisteredType != nullptr)
        {
            if (typeId & asTYPEID_OBJHANDLE)
            {
                typeId &= ~(asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST);
                objPtr = *(void**) objPtr;
            }

            if (g_PrintRegisteredType(dst, objPtr, typeId, depth))
                return;
        }

        dst << "Registered Object";

        return;
    }

    dst << "UNKNOWN";

    return;
}

void Print::PrintFormat(std::ostream& stream, std::string const& in, std::pair<void const*, int> const* args, int argc)
{
    if (argc <= 0)
    {
        stream << in;
        return;
    }

    for (size_t itr = 0, next = 0; itr < in.size(); itr = next)
    {
        next = in.find_first_of('%', itr);
        stream << in.substr(itr, next - itr);

        if (next == std::string::npos)
            break;

        if (!isdigit(in[++next]))
            stream << '%';
        else
        {
            auto arg = atoi(&in[next]) % argc;
            while (next < in.size() && isdigit(in[next])) ++next;

            Print::PrintTemplate(stream, args[arg].first, args[arg].second, 0);
        }
    }
}

static void PrintFunc(IN_ARGS_16)
{
    std::stringstream ss;
    Print::PrintTemplate(ss, W_ARGS_16);
    info_log("ScriptEngine", "%s", ss.str().c_str());
}

static void PrettyPrinting(IN_ARGS_16, std::string* thisPointer)
{
    std::stringstream ss;
    Print::PrintTemplate(ss, W_ARGS_16);
    new (thisPointer) std::string(ss.str());
}

static void asPrintFormat(std::string const& in, IN_ARGS_16)
{
    std::array<std::pair<void const*, int>, 16> args{A_ARGS_16};
    std::stringstream ss;
    Print::PrintFormat(ss, in, args.data(), args.size());
    info_log("ScriptEngine", "%s", ss.str().c_str());
}

static std::string PrettyPrintingF(std::string* This, IN_ARGS_16)
{
    std::array<std::pair<void const*, int>, 16> args{A_ARGS_16};
    std::stringstream ss;
    Print::PrintFormat(ss, *This, args.data(), args.size());
    return ss.str();
}


void Print::asRegister(asIScriptEngine* engine)
{
    int r;
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f(const ?&in, " INS_15 ")",
                                        asFUNCTION(PrettyPrinting), asCALL_CDECL_OBJLAST);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("string", "string format(" INS_16 ") const", asFUNCTION(PrettyPrintingF),
                                     asCALL_CDECL_OBJFIRST);
    assert(r >= 0);

    r = engine->RegisterGlobalFunction("void print(" INS_16 ")", asFUNCTION(PrintFunc), asCALL_CDECL);
    assert(r == asALREADY_REGISTERED || r >= 0);

    r = engine->RegisterGlobalFunction("void printf(const string &in format, " INS_16 ")", asFUNCTION(asPrintFormat),
                                       asCALL_CDECL);
    assert(r == asALREADY_REGISTERED || r >= 0);
}


namespace Engine
{
    String ScriptEngine::to_string(const void* object, int_t type_id)
    {
        std::stringstream stream;
        Print::PrintTemplate(stream, object, type_id);
        return stream.str();
    }
}// namespace Engine
