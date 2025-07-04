namespace meta
{
	[UnscopedEnum]
	enum ParameterFlags
	{
		MemoryBlock              = 0x8010013,
		ExcludeMaterialParameter = 0x8000000,
		Globals                  = 0x0010018,
		LocalToWorld             = 0x4010019,
	}
}


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

[__AttributeUsage(_AttributeTargets.Var)]
struct parameter_flagsAttribute
{
	meta::ParameterFlags flags;
}