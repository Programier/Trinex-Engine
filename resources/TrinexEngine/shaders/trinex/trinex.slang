// ATTRIBUTES SECTION

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


// OPERATORS SECTION

[ForceInline]
matrix<T, R, L> operator*<T:__BuiltinFloatingPointType, let L : int, let C : int, let R : int>(matrix<T, C, L> m1, matrix<T, R, C> m2)
{
    return mul(m2, m1);
}

[ForceInline]
vector<T, R> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(vector<T, C> v, matrix<T, R, C> m)
{
    return mul(m, v);
}

[ForceInline]
vector<T, C> operator*<T : __BuiltinFloatingPointType, let C : int, let R : int>(matrix<T, R, C> m, vector<T, R> v)
{
    return mul(v, m);
}