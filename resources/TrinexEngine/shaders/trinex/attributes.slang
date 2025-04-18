#ifndef TRINEX_ATTRIBUTES_SLANG
#define TRINEX_ATTRIBUTES_SLANG

[__AttributeUsage(_AttributeTargets.Var)]
struct parameter_typeAttribute
{
	string type;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct vertex_streamAttribute
{
	int stream;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct vertex_offsetAttribute
{
	uint offset;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct per_instanceAttribute
{
};

[__AttributeUsage(_AttributeTargets.Var)]
struct nameAttribute
{
	string name;
}

#endif
