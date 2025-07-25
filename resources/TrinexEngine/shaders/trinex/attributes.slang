namespace meta
{
	[UnscopedEnum]
	enum ParameterFlags
	{
		UniformBuffer            = 0x00010013,
		ExcludeMaterialParameter = 0x40000000,
		Globals                  = 0x00010038,
		LocalToWorld             = 0x20010039,
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