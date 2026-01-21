// ATTRIBUTES SECTION

namespace meta
{
	enum type
	{
		UniformBuffer            = 2,
		ExcludeMaterialParameter = 0 | 1 << 25,
		SceneView                = 2 | 1 << 31,
		LocalToWorld             = 0 | 1 << 31,

		// Descriptors
		Descriptor = 1 << 0
	}
}


[__AttributeUsage(_AttributeTargets.Var)]
struct parameter_typeAttribute
{
	meta::type type;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct nameAttribute
{
	string name;
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


// BINDLESS SECTION

T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
	return defaultGetDescriptorFromHandle(handle, BindlessDescriptorOptions::None);
}
