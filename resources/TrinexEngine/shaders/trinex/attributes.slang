#ifndef TRINEX_ATTRIBUTES_SLANG
#define TRINEX_ATTRIBUTES_SLANG

[__AttributeUsage(_AttributeTargets.Var)]
struct parameter_typeAttribute
{
	string type;
};

#endif
