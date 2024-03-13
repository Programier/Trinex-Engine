SLANG_RAW("// Slang `core` library\n")
SLANG_RAW("\n")
SLANG_RAW("// Aliases for base types\n")
SLANG_RAW("typedef half float16_t;\n")
SLANG_RAW("typedef float float32_t;\n")
SLANG_RAW("typedef double float64_t;\n")
SLANG_RAW("\n")
SLANG_RAW("typedef int int32_t;\n")
SLANG_RAW("typedef uint uint32_t;\n")
SLANG_RAW("\n")
SLANG_RAW("typedef uintptr_t size_t;\n")
SLANG_RAW("typedef uintptr_t usize_t\n")
SLANG_RAW("typedef intptr_t ssize_t;\n")
SLANG_RAW("\n")
SLANG_RAW("// Modifier for variables that must resolve to compile-time constants\n")
SLANG_RAW("// as part of translation.\n")
SLANG_RAW("syntax constexpr : ConstExprModifier;\n")
SLANG_RAW("\n")
SLANG_RAW("// Modifier for variables that should have writes be made\n")
SLANG_RAW("// visible at the global-memory scope\n")
SLANG_RAW("syntax globallycoherent : GloballyCoherentModifier;\n")
SLANG_RAW("\n")
SLANG_RAW("/// Modifier to disable inteprolation and force per-vertex passing of a varying attribute.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// When a varying attribute passed to the fragment shader is marked `pervertex`, it will\n")
SLANG_RAW("/// not be interpolated during rasterization (similar to `nointerpolate` attributes).\n")
SLANG_RAW("/// Unlike a plain `nointerpolate` attribute, this modifier indicates that the attribute\n")
SLANG_RAW("/// should *only* be acccessed through the `GetAttributeAtVertex()` operation, so access its\n")
SLANG_RAW("/// distinct per-vertex values.\n")
SLANG_RAW("///\n")
SLANG_RAW("syntax pervertex : PerVertexModifier;\n")
SLANG_RAW("\n")
SLANG_RAW("/// Modifier to indicate a buffer or texture element type is\n")
SLANG_RAW("/// backed by data in an unsigned normalized format.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// The `unorm` modifier is only valid on `float` and `vector`s\n")
SLANG_RAW("/// with `float` elements.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// This modifier does not affect the semantics of any variable,\n")
SLANG_RAW("/// parameter, or field that uses it. The semantics of a `float`\n")
SLANG_RAW("/// or vector are the same with or without `unorm`.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// The `unorm` modifier can be used for the element type of a\n")
SLANG_RAW("/// buffer or texture, to indicate that the data that is bound\n")
SLANG_RAW("/// to that buffer or texture is in a matching normalized format.\n")
SLANG_RAW("/// Some platforms may require a `unorm` qualifier for such buffers\n")
SLANG_RAW("/// and textures, and others may operate correctly without it.\n")
SLANG_RAW("///\n")
SLANG_RAW("syntax unorm : UNormModifier;\n")
SLANG_RAW("\n")
SLANG_RAW("/// Modifier to indicate a buffer or texture element type is\n")
SLANG_RAW("/// backed by data in an signed normalized format.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// The `snorm` modifier is only valid on `float` and `vector`s\n")
SLANG_RAW("/// with `float` elements.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// This modifier does not affect the semantics of any variable,\n")
SLANG_RAW("/// parameter, or field that uses it. The semantics of a `float`\n")
SLANG_RAW("/// or vector are the same with or without `snorm`.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// The `snorm` modifier can be used for the element type of a\n")
SLANG_RAW("/// buffer or texture, to indicate that the data that is bound\n")
SLANG_RAW("/// to that buffer or texture is in a matching normalized format.\n")
SLANG_RAW("/// Some platforms may require a `unorm` qualifier for such buffers\n")
SLANG_RAW("/// and textures, and others may operate correctly without it.\n")
SLANG_RAW("///\n")
SLANG_RAW("syntax snorm : SNormModifier;\n")
SLANG_RAW("\n")
SLANG_RAW("/// Modifier to indicate that a function name should not be mangled\n")
SLANG_RAW("/// by the Slang compiler.\n")
SLANG_RAW("///\n")
SLANG_RAW("/// The `__extern_cpp` modifier makes a symbol to have unmangled\n")
SLANG_RAW("/// name in source/output C++ code.\n")
SLANG_RAW("///\n")
SLANG_RAW("syntax __extern_cpp : ExternCppModifier;\n")
SLANG_RAW("\n")
SLANG_RAW("interface IComparable\n")
SLANG_RAW("{\n")
SLANG_RAW("    bool equals(This other);\n")
SLANG_RAW("    bool lessThan(This other);\n")
SLANG_RAW("    bool lessThanOrEquals(This other);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("interface IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const This maxValue;\n")
SLANG_RAW("    static const This minValue;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [TreatAsDifferentiable] : TreatAsDifferentiableAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("interface IArithmetic : IComparable\n")
SLANG_RAW("{\n")
SLANG_RAW("    This add(This other);\n")
SLANG_RAW("    This sub(This other);\n")
SLANG_RAW("    This mul(This other);\n")
SLANG_RAW("    This div(This other);\n")
SLANG_RAW("    This mod(This other);\n")
SLANG_RAW("    This neg();\n")
SLANG_RAW("\n")
SLANG_RAW("    __init(int val);\n")
SLANG_RAW("\n")
SLANG_RAW("        /// Initialize from the same type.\n")
SLANG_RAW("    __init(This value);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("interface ILogical : IComparable\n")
SLANG_RAW("{\n")
SLANG_RAW("    This shl(int value);\n")
SLANG_RAW("    This shr(int value);\n")
SLANG_RAW("    This bitAnd(This other);\n")
SLANG_RAW("    This bitOr(This other);\n")
SLANG_RAW("    This bitXor(This other);\n")
SLANG_RAW("    This bitNot();\n")
SLANG_RAW("    This and(This other);\n")
SLANG_RAW("    This or(This other);\n")
SLANG_RAW("    This not();\n")
SLANG_RAW("    __init(int val);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("interface IInteger : IArithmetic, ILogical\n")
SLANG_RAW("{\n")
SLANG_RAW("    int toInt();\n")
SLANG_RAW("    int64_t toInt64();\n")
SLANG_RAW("    uint toUInt();\n")
SLANG_RAW("    uint64_t toUInt64();\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("interface IFloat : IArithmetic, IDifferentiable\n")
SLANG_RAW("{\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    __init(float value);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    float toFloat();\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This add(This other);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This sub(This other);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This mul(This other);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This div(This other);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This mod(This other);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This neg();\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    __init(This value);\n")
SLANG_RAW("\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    This scale<T:__BuiltinFloatingPointType>(T scale);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that can be used as an operand for builtins\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __BuiltinType {}\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that can be used for arithmetic operations\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __BuiltinArithmeticType : __BuiltinType, IArithmetic\n")
SLANG_RAW("{\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that can be used for logical/bitwise operations\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __BuiltinLogicalType : __BuiltinType, ILogical\n")
SLANG_RAW("{\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that logically has a sign (positive/negative/zero)\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __BuiltinSignedArithmeticType : __BuiltinArithmeticType {}\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that can represent integers\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __BuiltinIntegerType : __BuiltinArithmeticType\n")
SLANG_RAW("{}\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that can represent non-integers\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __BuiltinRealType : __BuiltinSignedArithmeticType {}\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(AggTypeDecl)\n")
SLANG_RAW("attribute_syntax [__NonCopyableType] : NonCopyableTypeAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [__NoSideEffect] : NoSideEffectAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("/// Marks a function for forward-mode differentiation.\n")
SLANG_RAW("/// i.e. the compiler will automatically generate a new function\n")
SLANG_RAW("/// that computes the jacobian-vector product of the original.\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("/// Marks a function for backward-mode differentiation.\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [BackwardDifferentiable(order:int = 0)] : BackwardDifferentiableAttribute;\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [Differentiable(order:int = 0)] : BackwardDifferentiableAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_RequirePrelude
)
SLANG_RAW(")\n")
SLANG_RAW("void __requirePrelude(constexpr String preludeText);\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_RequireGLSLExtension
)
SLANG_RAW(")\n")
SLANG_RAW("void __requireGLSLExtension(constexpr String preludeText);\n")
SLANG_RAW("\n")
SLANG_RAW("/// Interface to denote types as differentiable.\n")
SLANG_RAW("/// Allows for user-specified differential types as\n")
SLANG_RAW("/// well as automatic generation, for when the associated type\n")
SLANG_RAW("/// hasn't been declared explicitly.\n")
SLANG_RAW("/// Note that the requirements must currently be defined in this exact order\n")
SLANG_RAW("/// since the auto-diff pass relies on the order to grab the struct keys.\n")
SLANG_RAW("/// \n")
SLANG_RAW("__magic_type(DifferentiableType)\n")
SLANG_RAW("interface IDifferentiable\n")
SLANG_RAW("{\n")
SLANG_RAW("    // Note: the compiler implementation requires the `Differential` associated type to be defined\n")
SLANG_RAW("    // before anything else.\n")
SLANG_RAW("\n")
SLANG_RAW("    __builtin_requirement(")
SLANG_SPLICE( (int)BuiltinRequirementKind::DifferentialType
)
SLANG_RAW(" )\n")
SLANG_RAW("    associatedtype Differential : IDifferentiable;\n")
SLANG_RAW("\n")
SLANG_RAW("    __builtin_requirement(")
SLANG_SPLICE( (int)BuiltinRequirementKind::DZeroFunc
)
SLANG_RAW(" )\n")
SLANG_RAW("    static Differential dzero();\n")
SLANG_RAW("\n")
SLANG_RAW("    __builtin_requirement(")
SLANG_SPLICE( (int)BuiltinRequirementKind::DAddFunc
)
SLANG_RAW(" )\n")
SLANG_RAW("    static Differential dadd(Differential, Differential);\n")
SLANG_RAW("\n")
SLANG_RAW("    __builtin_requirement(")
SLANG_SPLICE( (int)BuiltinRequirementKind::DMulFunc
)
SLANG_RAW(" )\n")
SLANG_RAW("    __generic<T : __BuiltinRealType>\n")
SLANG_RAW("    static Differential dmul(T, Differential);\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("/// Pair type that serves to wrap the primal and\n")
SLANG_RAW("/// differential types of an arbitrary type T.\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T : IDifferentiable>\n")
SLANG_RAW("__magic_type(DifferentialPairType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_DifferentialPairUserCodeType
)
SLANG_RAW(")\n")
SLANG_RAW("struct DifferentialPair : IDifferentiable\n")
SLANG_RAW("{\n")
SLANG_RAW("    typedef DifferentialPair<T.Differential> Differential;\n")
SLANG_RAW("    typedef T.Differential DifferentialElementType;\n")
SLANG_RAW("\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeDifferentialPairUserCode
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T _primal, T.Differential _differential);\n")
SLANG_RAW("\n")
SLANG_RAW("    property p : T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_DifferentialPairGetPrimalUserCode
)
SLANG_RAW(")\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    property v : T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_DifferentialPairGetPrimalUserCode
)
SLANG_RAW(")\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    property d : T.Differential\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_DifferentialPairGetDifferentialUserCode
)
SLANG_RAW(")\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    T.Differential getDifferential()\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return d;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    T getPrimal()\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return p;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    static Differential dzero()\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return Differential(T.dzero(), T.Differential.dzero());\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    static Differential dadd(Differential a, Differential b)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return Differential(\n")
SLANG_RAW("            T.dadd(\n")
SLANG_RAW("                a.p,\n")
SLANG_RAW("                b.p\n")
SLANG_RAW("            ),\n")
SLANG_RAW("            T.Differential.dadd(a.d, b.d));\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<U : __BuiltinRealType>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    static Differential dmul(U a, Differential b)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return Differential(\n")
SLANG_RAW("            T.dmul<U>(a, b.p),\n")
SLANG_RAW("            T.Differential.dmul<U>(a, b.d));\n")
SLANG_RAW("    }\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("/// A type that uses a floating-point representation\n")
SLANG_RAW("[sealed]\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("[TreatAsDifferentiable]\n")
SLANG_RAW("interface __BuiltinFloatingPointType : __BuiltinRealType, IFloat\n")
SLANG_RAW("{\n")
SLANG_RAW("        /// Get the value of the mathematical constant pi in this type.\n")
SLANG_RAW("    [Differentiable]\n")
SLANG_RAW("    static This getPi();\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("//@ hidden:\n")
SLANG_RAW("\n")
SLANG_RAW("// A type resulting from an `enum` declaration.\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("__magic_type(EnumTypeType)\n")
SLANG_RAW("interface __EnumType\n")
SLANG_RAW("{\n")
SLANG_RAW("    // The type of tags for this `enum`\n")
SLANG_RAW("    //\n")
SLANG_RAW("    // Note: using `__Tag` instead of `Tag` to avoid any\n")
SLANG_RAW("    // conflict if a user had an `enum` case called `Tag`\n")
SLANG_RAW("    associatedtype __Tag : __BuiltinIntegerType;\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("// Use an extension to declare that every `enum` type\n")
SLANG_RAW("// inherits an initializer based on the tag type.\n")
SLANG_RAW("//\n")
SLANG_RAW("// Note: there is an important and subtle point here.\n")
SLANG_RAW("// If we declared these initializers inside the `interface`\n")
SLANG_RAW("// declaration above, then they would implicitly be\n")
SLANG_RAW("// *requirements* of the `__EnumType` interface, and any\n")
SLANG_RAW("// type that declares conformance to it would need to\n")
SLANG_RAW("// provide implementations. That would put the onus on\n")
SLANG_RAW("// the semantic checker to synthesize such initializers\n")
SLANG_RAW("// when conforming an `enum` type to `__EnumType` (just\n")
SLANG_RAW("// as it currently synthesizes the `__Tag` requirement.\n")
SLANG_RAW("// Putting the declaration in an `extension` makes them\n")
SLANG_RAW("// concrete declerations rather than interface requirements.\n")
SLANG_RAW("// (Admittedly, they are \"concrete\" declarations with\n")
SLANG_RAW("// no bodies, because currently all initializers are\n")
SLANG_RAW("// assumed to be intrinsics).\n")
SLANG_RAW("//\n")
SLANG_RAW("// TODO: It might be more accurate to express this as:\n")
SLANG_RAW("//\n")
SLANG_RAW("//      __generic<T:__EnumType> extension T { ... }\n")
SLANG_RAW("//\n")
SLANG_RAW("// That alternative would express an extension of every\n")
SLANG_RAW("// type that conforms to `__EnumType`, rather than an\n")
SLANG_RAW("// extension of `__EnumType` itself. The distinction\n")
SLANG_RAW("// is subtle, and unfortunately not one the Slang type\n")
SLANG_RAW("// checker is equiped to handle right now. For now we\n")
SLANG_RAW("// will stick with the syntax that actually works, even\n")
SLANG_RAW("// if it might be the less technically correct one.\n")
SLANG_RAW("//\n")
SLANG_RAW("//\n")
SLANG_RAW("extension __EnumType\n")
SLANG_RAW("{\n")
SLANG_RAW("    // TODO: this should be a single initializer using\n")
SLANG_RAW("    // the `__Tag` associated type from the `__EnumType`\n")
SLANG_RAW("    // interface, but right now the scoping for looking\n")
SLANG_RAW("    // up that type isn't working right.\n")
SLANG_RAW("    //\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(int value);\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(uint value);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("// A type resulting from an `enum` declaration\n")
SLANG_RAW("// with the `[flags]` attribute.\n")
SLANG_RAW("[builtin]\n")
SLANG_RAW("interface __FlagsEnumType : __EnumType\n")
SLANG_RAW("{\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("interface IArray<T>\n")
SLANG_RAW("{\n")
SLANG_RAW("    int getCount();\n")
SLANG_RAW("    __subscript(int index) -> T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, let N:int>\n")
SLANG_RAW("__magic_type(ArrayExpressionType)\n")
SLANG_RAW("struct Array : IArray<T>\n")
SLANG_RAW("{\n")
SLANG_RAW("    [ForceInline]\n")
SLANG_RAW("    int getCount() { return N; }\n")
SLANG_RAW("\n")
SLANG_RAW("    __subscript(int index) -> T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_GetElement
)
SLANG_RAW(")\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("// The \"comma operator\" is effectively just a generic function that returns its second\n")
SLANG_RAW("// argument. The left-to-right evaluation order guaranteed by Slang then ensures that\n")
SLANG_RAW("// `left` is evaluated before `right`.\n")
SLANG_RAW("//\n")
SLANG_RAW("__generic<T,U>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("U operator,(T left, U right)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return right;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("// The ternary `?:` operator does not short-circuit in HLSL, and Slang no longer\n")
SLANG_RAW("// follow that definition for the scalar condition overload, so this declaration just serves\n")
SLANG_RAW("// for type-checking purpose only.\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T> __intrinsic_op(select) T operator?:(bool condition, T ifTrue, T ifFalse);\n")
SLANG_RAW("__generic<T, let N : int> __intrinsic_op(select) vector<T,N> operator?:(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);\n")
SLANG_RAW("\n")
SLANG_RAW("// Users are advised to use `select` instead if non-short-circuiting behavior is intended.\n")
SLANG_RAW("__generic<T> __intrinsic_op(select) T select(bool condition, T ifTrue, T ifFalse);\n")
SLANG_RAW("__generic<T, let N : int> __intrinsic_op(select) vector<T,N> select(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);\n")
SLANG_RAW("\n")
SLANG_RAW("// Allow real-number types to be cast into each other\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_FloatCast
)
SLANG_RAW(")\n")
SLANG_RAW("    T __realCast<T : __BuiltinRealType, U : __BuiltinRealType>(U val);\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(")\n")
SLANG_RAW("    T __intCast<T : __BuiltinType, U : __BuiltinType>(U val);\n")

// We are going to use code generation to produce the
// declarations for all of our base types.
static const int kBaseTypeCount = sizeof(kBaseTypes) / sizeof(kBaseTypes[0]);
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
SLANG_RAW("#line 455 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__builtin_type(")
SLANG_SPLICE(int(kBaseTypes[tt].tag)
)
SLANG_RAW(")\n")
SLANG_RAW("struct ")
SLANG_SPLICE(kBaseTypes[tt].name
)
SLANG_RAW("\n")
SLANG_RAW("    : __BuiltinType\n")
SLANG_RAW("\n")

    switch (kBaseTypes[tt].tag)
    {
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
SLANG_RAW("#line 467 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("    ,  __BuiltinFloatingPointType\n")
SLANG_RAW("    ,  __BuiltinRealType\n")
SLANG_RAW("    ,  __BuiltinSignedArithmeticType\n")
SLANG_RAW("    ,  __BuiltinArithmeticType\n")

        break;
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
SLANG_RAW("#line 479 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("    ,  __BuiltinSignedArithmeticType\n")

        ; // fall through
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::UIntPtr:
SLANG_RAW("#line 488 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("    ,  __BuiltinArithmeticType\n")
SLANG_RAW("    ,  __BuiltinIntegerType\n")

        ; // fall through
    case BaseType::Bool:
SLANG_RAW("#line 494 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("    ,  __BuiltinLogicalType\n")

        break;

    default:
        break;
    }
SLANG_RAW("#line 502 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("{\n")

    // Declare initializers to convert from various other types
    for (int ss = 0; ss < kBaseTypeCount; ++ss)
    {
        // Don't allow conversion to or from `void`
        if (kBaseTypes[tt].tag == BaseType::Void)
            continue;
        if (kBaseTypes[ss].tag == BaseType::Void)
            continue;

        // We need to emit a modifier so that the semantic-checking
        // layer will know it can use these operations for implicit
        // conversion.
        ConversionCost conversionCost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ss]);

        IROp intrinsicOpCode = getBaseTypeConversionOp(
            kBaseTypes[tt],
            kBaseTypes[ss]);

        BuiltinConversionKind builtinConversionKind = kBuiltinConversion_Unknown;
        if (kBaseTypes[tt].tag == BaseType::Double &&
            kBaseTypes[ss].tag == BaseType::Float)
            builtinConversionKind = kBuiltinConversion_FloatToDouble;

        const char* attrib = "";
        if ((kBaseTypes[tt].flags & kBaseTypes[ss].flags & FLOAT_MASK) != 0)
            attrib = "[TreatAsDifferentiable]";
SLANG_RAW("#line 533 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("    ")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(intrinsicOpCode
)
SLANG_RAW(")\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(conversionCost
)
SLANG_RAW(", ")
SLANG_SPLICE(builtinConversionKind
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(")
SLANG_SPLICE(kBaseTypes[ss].name
)
SLANG_RAW(" value);\n")
SLANG_RAW("\n")

    }

    // Integer type implementations.
    switch (kBaseTypes[tt].tag)
    {
    case BaseType::Bool:
SLANG_RAW("#line 546 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("        [__unsafeForceInlineEarly] __intrinsic_op(")
SLANG_SPLICE(kIROp_Eql
)
SLANG_RAW(") bool equals(This other);\n")
SLANG_RAW("        [__unsafeForceInlineEarly] __intrinsic_op(")
SLANG_SPLICE(kIROp_Less
)
SLANG_RAW(") bool lessThan(This other);\n")
SLANG_RAW("        [__unsafeForceInlineEarly] __intrinsic_op(")
SLANG_SPLICE(kIROp_Leq
)
SLANG_RAW(") bool lessThanOrEquals(This other);\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This shl(int other) { return __intCast<This>(__shl(__intCast<int>(this), other)); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This shr(int other) { return __intCast<This>(__shr(__intCast<int>(this), other)); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This bitAnd(This other) { return __intCast<This>(__and(__intCast<int>(this), __intCast<int>(other))); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This bitOr(This other) { return __intCast<This>(__or(__intCast<int>(this), __intCast<int>(other))); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] __intrinsic_op(")
SLANG_SPLICE(kIROp_And
)
SLANG_RAW(") This and(This other);\n")
SLANG_RAW("        [__unsafeForceInlineEarly] __intrinsic_op(")
SLANG_SPLICE(kIROp_Or
)
SLANG_RAW(") This or(This other);\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This bitXor(This other)  { return __intCast<This>(__xor(__intCast<int>(this), __intCast<int>(other))); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This bitNot()  { return __intCast<This>(__not(__intCast<int>(this))); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] __intrinsic_op(")
SLANG_SPLICE(kIROp_Not
)
SLANG_RAW(") This not();\n")

        break;
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
    case BaseType::UIntPtr:
SLANG_RAW("#line 571 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("        // If this is a basic integer type, then define explicit\n")
SLANG_RAW("        // initializers that take a value of an `enum` type.\n")
SLANG_RAW("        //\n")
SLANG_RAW("        // TODO: This should actually be restricted, so that this\n")
SLANG_RAW("        // only applies `where T.__Tag == Self`, but we don't have\n")
SLANG_RAW("        // the needed features in our type system to implement\n")
SLANG_RAW("        // that constraint right now.\n")
SLANG_RAW("        //\n")
SLANG_RAW("        __generic<T:__EnumType>\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(")\n")
SLANG_RAW("        __init(T value);\n")
SLANG_RAW("\n")
SLANG_RAW("        // Implementation of the `IInteger` interface.\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Less
)
SLANG_RAW(") bool lessThan(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Leq
)
SLANG_RAW(")  bool lessThanOrEquals(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Eql
)
SLANG_RAW(")  bool equals(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Add
)
SLANG_RAW(")  This add(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Sub
)
SLANG_RAW(")  This sub(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(")  This mul(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Div
)
SLANG_RAW(")  This div(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_FRem
)
SLANG_RAW(") This mod(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Neg
)
SLANG_RAW(")  This neg();\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Lsh
)
SLANG_RAW(") This shl(int other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_Rsh
)
SLANG_RAW(") This shr(int other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_BitAnd
)
SLANG_RAW(") This bitAnd(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_BitOr
)
SLANG_RAW(") This bitOr(This other);\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This and(This other) {return __intCast<This>(__intCast<bool>(this) && __intCast<bool>(other)); }\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This or(This other) {return __intCast<This>(__intCast<bool>(this) || __intCast<bool>(other)); }\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_BitXor
)
SLANG_RAW(") This bitXor(This other);\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_BitNot
)
SLANG_RAW(") This bitNot();\n")
SLANG_RAW("        [__unsafeForceInlineEarly] This not() {return __intCast<This>(!__intCast<bool>(this)); }\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(") int toInt();\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(") int64_t toInt64();\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(") uint toUInt();\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_IntCast
)
SLANG_RAW(") uint64_t toUInt64();\n")

        break;

    default:
        break;
    }

    // If this is a floating-point type, then we need to
    // implement the `IFloat` interface, which defines the basic `getPi()`
    // function that is used to implement generic versions of `degrees()` and
    // `radians()`.
    //
    switch (kBaseTypes[tt].tag)
    {
    default:
        break;
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
SLANG_RAW("#line 626 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("        [Differentiable]\n")
SLANG_RAW("        static ")
SLANG_SPLICE(kBaseTypes[tt].name
)
SLANG_RAW(" getPi() { return ")
SLANG_SPLICE(kBaseTypes[tt].name
)
SLANG_RAW("(3.14159265358979323846264338328); }\n")
SLANG_RAW("\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Less
)
SLANG_RAW(") bool lessThan(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Leq
)
SLANG_RAW(")  bool lessThanOrEquals(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Eql
)
SLANG_RAW(")  bool equals(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Add
)
SLANG_RAW(")  This add(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Sub
)
SLANG_RAW(")  This sub(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(")  This mul(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Div
)
SLANG_RAW(")  This div(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_FRem
)
SLANG_RAW(") This mod(This other);\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Neg
)
SLANG_RAW(")  This neg();\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_FloatCast
)
SLANG_RAW(") float toFloat();\n")
SLANG_RAW("         __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(") This scale<T:__BuiltinFloatingPointType>(T s) { return __mul(this, __realCast<This>(s)); }\n")
SLANG_RAW("        typedef ")
SLANG_SPLICE(kBaseTypes[tt].name
)
SLANG_RAW(" Differential;\n")
SLANG_RAW("    \n")
SLANG_RAW("        [__unsafeForceInlineEarly]\n")
SLANG_RAW("        [BackwardDifferentiable]\n")
SLANG_RAW("        static Differential dzero()\n")
SLANG_RAW("        {\n")
SLANG_RAW("            return Differential(0);\n")
SLANG_RAW("        }\n")
SLANG_RAW("\n")
SLANG_RAW("        [__unsafeForceInlineEarly]\n")
SLANG_RAW("        [BackwardDifferentiable]\n")
SLANG_RAW("        static Differential dadd(Differential a, Differential b)\n")
SLANG_RAW("        {\n")
SLANG_RAW("            return a + b;\n")
SLANG_RAW("        }\n")
SLANG_RAW("        \n")
SLANG_RAW("        __generic<U : __BuiltinRealType>\n")
SLANG_RAW("        [__unsafeForceInlineEarly]\n")
SLANG_RAW("        [BackwardDifferentiable]\n")
SLANG_RAW("        static Differential dmul(U a, Differential b)\n")
SLANG_RAW("        {\n")
SLANG_RAW("            return __realCast<Differential, U>(a) * b;\n")
SLANG_RAW("        }\n")

        break;
    }

    // If this is the `void` type, then we want to allow
    // explicit conversion to it from any other type, using
    // `(void) someExpression`.
    //
    if( kBaseTypes[tt].tag == BaseType::Void )
    {
SLANG_RAW("#line 674 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("        __generic<T>\n")
SLANG_RAW("        [__readNone]\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_CastToVoid
)
SLANG_RAW(")\n")
SLANG_RAW("        __init(T value)\n")
SLANG_RAW("        {}\n")

    }

SLANG_RAW("#line 683 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")

}

// Declare built-in pointer type
// (eventually we can have the traditional syntax sugar for this)
SLANG_RAW("#line 692 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__magic_type(NullPtrType)\n")
SLANG_RAW("struct NullPtr\n")
SLANG_RAW("{\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__magic_type(NoneType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_VoidType
)
SLANG_RAW(")\n")
SLANG_RAW("struct __none_t\n")
SLANG_RAW("{\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(PtrType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_PtrType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Ptr\n")
SLANG_RAW("{\n")
SLANG_RAW("    __generic<U>\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_BitCast
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<U> ptr);\n")
SLANG_RAW("\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastIntToPtr
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(uint64_t val);\n")
SLANG_RAW("\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastIntToPtr
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(int64_t val);\n")
SLANG_RAW("\n")
SLANG_RAW("    __subscript(int index) -> T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_GetOffsetPtr
)
SLANG_RAW(")\n")
SLANG_RAW("        ref;\n")
SLANG_RAW("    }\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Load
)
SLANG_RAW(")\n")
SLANG_RAW("T __load<T>(Ptr<T> ptr);\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Store
)
SLANG_RAW(")\n")
SLANG_RAW("void __store<T>(Ptr<T> ptr, T val);\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetElementPtr
)
SLANG_RAW(")\n")
SLANG_RAW("Ptr<T> __getElementPtr<T>(Ptr<T> ptr, int index);\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetElementPtr
)
SLANG_RAW(")\n")
SLANG_RAW("Ptr<T> __getElementPtr<T>(Ptr<T> ptr, int64_t index);\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetOffsetPtr
)
SLANG_RAW(")\n")
SLANG_RAW("Ptr<T> __getOffsetPtr<T>(Ptr<T> ptr, int index);\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetOffsetPtr
)
SLANG_RAW(")\n")
SLANG_RAW("Ptr<T> __getOffsetPtr<T>(Ptr<T> ptr, int64_t index);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Less
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator<(Ptr<T> p1, Ptr<T> p2);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Leq
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator<=(Ptr<T> p1, Ptr<T> p2);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Greater
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator>(Ptr<T> p1, Ptr<T> p2);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Geq
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator>=(Ptr<T> p1, Ptr<T> p2);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Neq
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator!=(Ptr<T> p1, Ptr<T> p2);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Eql
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator==(Ptr<T> p1, Ptr<T> p2);\n")
SLANG_RAW("\n")
SLANG_RAW("extension bool : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(kConversionCost_PtrToBool
)
SLANG_RAW(")\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastPtrToBool
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<T> ptr);\n")
SLANG_RAW("\n")
SLANG_RAW("    static const bool maxValue = true;\n")
SLANG_RAW("    static const bool minValue = false;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension uint64_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastPtrToInt
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<T> ptr);\n")
SLANG_RAW("\n")
SLANG_RAW("    static const uint64_t maxValue = 0xFFFFFFFFFFFFFFFFULL;\n")
SLANG_RAW("    static const uint64_t minValue = 0;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension int64_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastPtrToInt
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<T> ptr);\n")
SLANG_RAW("\n")
SLANG_RAW("    static const int64_t maxValue =  0x7FFFFFFFFFFFFFFFLL;\n")
SLANG_RAW("    static const int64_t minValue = -0x8000000000000000LL;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension intptr_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastPtrToInt
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<T> ptr);\n")
SLANG_RAW("    static const intptr_t maxValue = ")
SLANG_SPLICE(SLANG_PROCESSOR_X86_64?"0x7FFFFFFFFFFFFFFFz":"0x7FFFFFFFz"
)
SLANG_RAW(";\n")
SLANG_RAW("    static const intptr_t minValue = ")
SLANG_SPLICE(SLANG_PROCESSOR_X86_64?"0x8000000000000000z":"0x80000000z"
)
SLANG_RAW(";\n")
SLANG_RAW("    static const int size = ")
SLANG_SPLICE(SLANG_PROCESSOR_X86_64?"8":"4"
)
SLANG_RAW(";\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension uintptr_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_CastPtrToInt
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<T> ptr);\n")
SLANG_RAW("    static const uintptr_t maxValue = ")
SLANG_SPLICE(SLANG_PROCESSOR_X86_64?"0xFFFFFFFFFFFFFFFFz":"0xFFFFFFFFz"
)
SLANG_RAW(";\n")
SLANG_RAW("    static const uintptr_t minValue = 0z;\n")
SLANG_RAW("    static const int size = ")
SLANG_SPLICE(SLANG_PROCESSOR_X86_64?"8":"4"
)
SLANG_RAW(";\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(OutType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_OutType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Out\n")
SLANG_RAW("{};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(InOutType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_InOutType
)
SLANG_RAW(")\n")
SLANG_RAW("struct InOut\n")
SLANG_RAW("{};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(RefType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_RefType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Ref\n")
SLANG_RAW("{};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(ConstRefType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_ConstRefType
)
SLANG_RAW(")\n")
SLANG_RAW("struct ConstRef\n")
SLANG_RAW("{};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(OptionalType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_OptionalType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Optional\n")
SLANG_RAW("{\n")
SLANG_RAW("    property bool hasValue\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_OptionalHasValue
)
SLANG_RAW(")\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    property T value\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_GetOptionalValue
)
SLANG_RAW(")\n")
SLANG_RAW("        get;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(kConversionCost_ValToOptional
)
SLANG_RAW(")\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeOptionalValue
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T val);\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool operator==(Optional<T> val, __none_t noneVal)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return !val.hasValue;\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool operator!=(Optional<T> val, __none_t noneVal)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return val.hasValue;\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool operator==(__none_t noneVal, Optional<T> val)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return !val.hasValue;\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool operator!=(__none_t noneVal, Optional<T> val)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return val.hasValue;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__magic_type(NativeRefType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_NativePtrType
)
SLANG_RAW(")\n")
SLANG_RAW("struct NativeRef\n")
SLANG_RAW("{\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_GetNativePtr
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T val);\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_ManagedPtrAttach
)
SLANG_RAW(")\n")
SLANG_RAW("void __managed_ptr_attach(__ref T val, NativeRef<T> nativeVal);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("T __attachToNativeRef(NativeRef<T> nativeVal)\n")
SLANG_RAW("{\n")
SLANG_RAW("    T result;\n")
SLANG_RAW("    __managed_ptr_attach(result, nativeVal);\n")
SLANG_RAW("    return result;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__magic_type(StringType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_StringType
)
SLANG_RAW(")\n")
SLANG_RAW("struct String\n")
SLANG_RAW("{\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeString
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(int val);\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeString
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(uint val);\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeString
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(int64_t val);\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeString
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(uint64_t val);\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeString
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(float val);\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeString
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(double val);\n")
SLANG_RAW("\n")
SLANG_RAW("    __target_intrinsic(cpp)\n")
SLANG_RAW("    int64_t getLength();\n")
SLANG_RAW("\n")
SLANG_RAW("    property int length\n")
SLANG_RAW("    {\n")
SLANG_RAW("        get { return (int)getLength(); }\n")
SLANG_RAW("    }\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("typedef String string;\n")
SLANG_RAW("\n")
SLANG_RAW("__magic_type(NativeStringType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_NativeStringType
)
SLANG_RAW(")\n")
SLANG_RAW("struct NativeString\n")
SLANG_RAW("{\n")
SLANG_RAW("    __target_intrinsic(cpp, \"int(strlen($0))\")\n")
SLANG_RAW("    int getLength();\n")
SLANG_RAW("\n")
SLANG_RAW("    __target_intrinsic(cpp, \"(void*)((const char*)($0))\")\n")
SLANG_RAW("    Ptr<void> getBuffer();\n")
SLANG_RAW("\n")
SLANG_RAW("    property int length { [__unsafeForceInlineEarly] get{return getLength();} }\n")
SLANG_RAW("\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_getNativeStr
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(String value);\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("extension Ptr<void>\n")
SLANG_RAW("{\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(kConversionCost_PtrToVoidPtr
)
SLANG_RAW(")\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __init(NativeString nativeStr) { this = nativeStr.getBuffer(); }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __intrinsic_op(0)\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(kConversionCost_PtrToVoidPtr
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(Ptr<T> ptr);\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T>\n")
SLANG_RAW("    __intrinsic_op(0)\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(kConversionCost_PtrToVoidPtr
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(NativeRef<T> ptr);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__magic_type(DynamicType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_DynamicType
)
SLANG_RAW(")\n")
SLANG_RAW("struct __Dynamic\n")
SLANG_RAW("{};\n")
SLANG_RAW("\n")
SLANG_RAW("extension half : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const half maxValue = half(65504);\n")
SLANG_RAW("    static const half minValue = half(-65504);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension float : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const float maxValue = 340282346638528859811704183484516925440.0f;\n")
SLANG_RAW("    static const float minValue = -340282346638528859811704183484516925440.0f;\n")
SLANG_RAW("\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension double : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const double maxValue = bit_cast<double>(0x7fefffffffffffffULL);\n")
SLANG_RAW("    static const double minValue = bit_cast<double>(0xffefffffffffffffULL);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension int : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const int maxValue = 2147483647;\n")
SLANG_RAW("    static const int minValue = -2147483648;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension uint : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const uint maxValue = 4294967295;\n")
SLANG_RAW("    static const uint minValue = 0;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension int8_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const int8_t maxValue = 127;\n")
SLANG_RAW("    static const int8_t minValue = -128;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension uint8_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const uint8_t maxValue = 255;\n")
SLANG_RAW("    static const uint8_t minValue = 0;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension uint16_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const uint16_t maxValue = 65535;\n")
SLANG_RAW("    static const uint16_t minValue = 0;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("extension int16_t : IRangedValue\n")
SLANG_RAW("{\n")
SLANG_RAW("    static const int16_t maxValue = 32767;\n")
SLANG_RAW("    static const int16_t minValue = -32768;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("    /// An `N` component vector with elements of type `T`.\n")
SLANG_RAW("__generic<T = float, let N : int = 4>\n")
SLANG_RAW("__magic_type(VectorExpressionType)\n")
SLANG_RAW("struct vector : IArray<T>\n")
SLANG_RAW("{\n")
SLANG_RAW("        /// The element type of the vector\n")
SLANG_RAW("    typedef T Element;\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("        /// Initialize a vector where all elements have the same scalar `value`.\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(kConversionCost_ScalarToVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVectorFromScalar
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T value);\n")
SLANG_RAW("\n")
SLANG_RAW("    /// Initialize a vector from a value of the same type\n")
SLANG_RAW("    // TODO: we should revise semantic checking so this kind of \"identity\" conversion is not required\n")
SLANG_RAW("    __intrinsic_op(0)\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    __init(vector<T,N> value);\n")
SLANG_RAW("\n")
SLANG_RAW("    [ForceInline]\n")
SLANG_RAW("    int getCount() { return N; }\n")
SLANG_RAW("\n")
SLANG_RAW("    __subscript(int index) -> T { __intrinsic_op(")
SLANG_SPLICE(kIROp_GetElement
)
SLANG_RAW(") get; }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("const int kRowMajorMatrixLayout = ")
SLANG_SPLICE(SLANG_MATRIX_LAYOUT_ROW_MAJOR
)
SLANG_RAW(";\n")
SLANG_RAW("const int kColumnMajorMatrixLayout = ")
SLANG_SPLICE(SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
)
SLANG_RAW(";\n")
SLANG_RAW("\n")
SLANG_RAW("    /// A matrix with `R` rows and `C` columns, with elements of type `T`.\n")
SLANG_RAW("__generic<T = float, let R : int = 4, let C : int = 4, let L : int = ")
SLANG_SPLICE(SLANG_MATRIX_LAYOUT_MODE_UNKNOWN
)
SLANG_RAW(">\n")
SLANG_RAW("__magic_type(MatrixExpressionType)\n")
SLANG_RAW("struct matrix : IArray<vector<T,C>>\n")
SLANG_RAW("{\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeMatrixFromScalar
)
SLANG_RAW(")\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    __init(T val);\n")
SLANG_RAW("\n")
SLANG_RAW("    /// Initialize a vector from a value of the same type\n")
SLANG_RAW("    // TODO: we should revise semantic checking so this kind of \"identity\" conversion is not required\n")
SLANG_RAW("    __intrinsic_op(0)\n")
SLANG_RAW("    [TreatAsDifferentiable]\n")
SLANG_RAW("    __init(This value);\n")
SLANG_RAW("\n")
SLANG_RAW("    [ForceInline]\n")
SLANG_RAW("    int getCount() { return R; }\n")
SLANG_RAW("\n")
SLANG_RAW("    __subscript(int index) -> vector<T,C> { __intrinsic_op(")
SLANG_SPLICE(kIROp_GetElement
)
SLANG_RAW(") get; }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Eql
)
SLANG_RAW(")\n")
SLANG_RAW("vector<bool, N> __vectorEql<T, let N:int>(vector<T, N> left, vector<T,N> right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T:__BuiltinFloatingPointType, let N : int>\n")
SLANG_RAW("extension vector<T,N> : IFloat\n")
SLANG_RAW("{\n")
SLANG_RAW("    [__unsafeForceInlineEarly] bool lessThan(This other) { return this[0] < other[0]; }\n")
SLANG_RAW("    [__unsafeForceInlineEarly] bool lessThanOrEquals(This other) { return this[0] <= other[0]; }\n")
SLANG_RAW("    [__unsafeForceInlineEarly] bool equals(This other) { return all(__vectorEql(this, other)); }\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_Add
)
SLANG_RAW(") This add(This other);\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_Sub
)
SLANG_RAW(") This sub(This other);\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(") This mul(This other);\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_Div
)
SLANG_RAW(") This div(This other);\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_FRem
)
SLANG_RAW(") This mod(This other);\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_Neg
)
SLANG_RAW(") This neg();\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(") This scale<T1:__BuiltinFloatingPointType>(T1 s);\n")
SLANG_RAW("    [__unsafeForceInlineEarly] float toFloat() { return __realCast<float>(this[0]); }\n")
SLANG_RAW("\n")
SLANG_RAW("    [OverloadRank(-1)]\n")
SLANG_RAW("    [__unsafeForceInlineEarly] __init(int v) { return vector<T,N>(T(v)); }\n")
SLANG_RAW("    [OverloadRank(-1)]\n")
SLANG_RAW("    [__unsafeForceInlineEarly] __init(float v) { return vector<T,N>(T(v)); }\n")
SLANG_RAW("\n")
SLANG_RAW("    // IDifferentiable\n")
SLANG_RAW("\n")
SLANG_RAW("    typedef vector<T, N> Differential;\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    [BackwardDifferentiable]\n")
SLANG_RAW("    static Differential dzero()\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return Differential(__slang_noop_cast<T>(T.dzero()));\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    [BackwardDifferentiable]\n")
SLANG_RAW("    static Differential dadd(Differential a, Differential b)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return a + b;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<U : __BuiltinRealType>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    [BackwardDifferentiable]\n")
SLANG_RAW("    static Differential dmul(U a, Differential b)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return __realCast<T, U>(a) * b;\n")
SLANG_RAW("    }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T:__BuiltinFloatingPointType, let N : int, let M : int, let L : int>\n")
SLANG_RAW("extension matrix<T,N,M,L> : IFloat\n")
SLANG_RAW("{\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] bool lessThan(This other) { return this < other; }\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] bool lessThanOrEquals(This other) { return this <= other; }\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] bool equals(This other) { return all(this == other); }\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_Add
)
SLANG_RAW(") This add(This other);\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_Sub
)
SLANG_RAW(") This sub(This other);\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(")This mul(This other);\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_Div
)
SLANG_RAW(") This div(This other);\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_FRem
)
SLANG_RAW(") This mod(This other);\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_Neg
)
SLANG_RAW(") This neg();\n")
SLANG_RAW("    [TreatAsDifferentiable] __intrinsic_op(")
SLANG_SPLICE(kIROp_Mul
)
SLANG_RAW(") This scale<T1:__BuiltinFloatingPointType>(T1 s);\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] This scale<T1:__BuiltinFloatingPointType>(T1 s);\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] float toFloat() { return __realCast<float>(this[0][0]); }\n")
SLANG_RAW("\n")
SLANG_RAW("    [OverloadRank(-1)]\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] __init(int v) { return matrix<T,N,M>(T(v)); }\n")
SLANG_RAW("    [OverloadRank(-1)]\n")
SLANG_RAW("    [TreatAsDifferentiable][__unsafeForceInlineEarly] __init(float v) { return matrix<T,N,M>(T(v)); }\n")
SLANG_RAW("\n")
SLANG_RAW("    // IDifferentiable.\n")
SLANG_RAW("    typedef matrix<T, N,M,L> Differential;\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    [BackwardDifferentiable]\n")
SLANG_RAW("    static Differential dzero()\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return matrix<T, N,M,L>(__slang_noop_cast<T>(T.dzero()));\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    [BackwardDifferentiable]\n")
SLANG_RAW("    static Differential dadd(Differential a, Differential b)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return a + b;\n")
SLANG_RAW("    }\n")
SLANG_RAW("    \n")
SLANG_RAW("    __generic<U : __BuiltinRealType>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    [BackwardDifferentiable]\n")
SLANG_RAW("    static Differential dmul(U a, Differential b)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        return __realCast<T, U>(a) * b;\n")
SLANG_RAW("    }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")

static const struct {
    char const* name;
    char const* glslPrefix;
} kTypes[] =
{
    {"half",        "f16"},
    {"float",       ""},
    {"double",      "d"},

    {"float16_t",   "f16"},
    {"float32_t",   "f32"},
    {"float64_t",   "f64"},

    {"int8_t",      "i8"},
    {"int16_t",     "i16"},
    {"int32_t",     "i32"},
    {"int",         "i"},
    {"int64_t",     "i64"},

    {"uint8_t",     "u8"},
    {"uint16_t",    "u16"},
    {"uint32_t",    "u32"},
    {"uint",        "u"},
    {"uint64_t",    "u64"},

    {"bool",        "b"},
};

static const int kTypeCount = sizeof(kTypes) / sizeof(kTypes[0]);

for (int tt = 0; tt < kTypeCount; ++tt)
{
    // Declare HLSL vector types
    for (int ii = 1; ii <= 4; ++ii)
    {
        sb << "typedef vector<" << kTypes[tt].name << "," << ii << "> " << kTypes[tt].name << ii << ";\n";
    }

    // Declare HLSL matrix types
    for (int rr = 2; rr <= 4; ++rr)
    for (int cc = 2; cc <= 4; ++cc)
    {
        sb << "typedef matrix<" << kTypes[tt].name << "," << rr << "," << cc << "> " << kTypes[tt].name << rr << "x" << cc << ";\n";
    }
}

// Declare additional built-in generic types
SLANG_RAW("#line 1236 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("//@ public:\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_ConstantBufferType
)
SLANG_RAW(")\n")
SLANG_RAW("__magic_type(ConstantBufferType)\n")
SLANG_RAW("struct ConstantBuffer {}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_TextureBufferType
)
SLANG_RAW(")\n")
SLANG_RAW("__magic_type(TextureBufferType)\n")
SLANG_RAW("struct TextureBuffer {}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_ParameterBlockType
)
SLANG_RAW(")\n")
SLANG_RAW("__magic_type(ParameterBlockType)\n")
SLANG_RAW("struct ParameterBlock {}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, let MAX_VERTS : uint>\n")
SLANG_RAW("__magic_type(VerticesType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_VerticesType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Vertices\n")
SLANG_RAW("{\n")
SLANG_RAW("    __subscript(uint index) -> T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        // TODO: Ellie make sure these remains write only\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_GetElementPtr
)
SLANG_RAW(")\n")
SLANG_RAW("        ref;\n")
SLANG_RAW("    }\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, let MAX_PRIMITIVES : uint>\n")
SLANG_RAW("__magic_type(IndicesType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_IndicesType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Indices\n")
SLANG_RAW("{\n")
SLANG_RAW("    __subscript(uint index) -> T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        // TODO: Ellie: It's illegal to not write out the whole primitive at once, should we use set over ref?\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_GetElementPtr
)
SLANG_RAW(")\n")
SLANG_RAW("        ref;\n")
SLANG_RAW("    }\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, let MAX_PRIMITIVES : uint>\n")
SLANG_RAW("__magic_type(PrimitivesType)\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_PrimitivesType
)
SLANG_RAW(")\n")
SLANG_RAW("struct Primitives\n")
SLANG_RAW("{\n")
SLANG_RAW("    __subscript(uint index) -> T\n")
SLANG_RAW("    {\n")
SLANG_RAW("        __intrinsic_op(")
SLANG_SPLICE(kIROp_GetElementPtr
)
SLANG_RAW(")\n")
SLANG_RAW("        ref;\n")
SLANG_RAW("    }\n")
SLANG_RAW("};\n")
SLANG_RAW("\n")
SLANG_RAW("//@ hidden:\n")
SLANG_RAW("\n")
SLANG_RAW("// Need to add constructors to the types above\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T> __extension vector<T, 2>\n")
SLANG_RAW("{\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, T y);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T> __extension vector<T, 3>\n")
SLANG_RAW("{\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, T y, T z);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(vector<T,2> xy, T z);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, vector<T,2> yz);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T> __extension vector<T, 4>\n")
SLANG_RAW("{\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, T y, T z, T w);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(vector<T,2> xy, T z, T w);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, vector<T,2> yz, T w);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, T y, vector<T,2> zw);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(vector<T,2> xy, vector<T,2> zw);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(vector<T,3> xyz, T w);\n")
SLANG_RAW("\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(kIROp_MakeVector
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(T x, vector<T,3> yzw);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")


// The above extensions are generic in the *type* of the vector,
// but explicit in the *size*. We will now declare an extension
// for each builtin type that is generic in the size.
//
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    if(kBaseTypes[tt].tag == BaseType::Void) continue;

    sb << "__generic<let N : int> __extension vector<"
        << kBaseTypes[tt].name << ",N>\n{\n";

    for (int ff = 0; ff < kBaseTypeCount; ++ff)
    {
        if(kBaseTypes[ff].tag == BaseType::Void) continue;


        if( tt != ff )
        {
            auto cost = getBaseTypeConversionCost(
                kBaseTypes[tt],
                kBaseTypes[ff]);
            auto op = getBaseTypeConversionOp(
                kBaseTypes[tt],
                kBaseTypes[ff]);

			// Implicit conversion from a vector of the same
			// size, but different element type.
            sb << "    __implicit_conversion(" << cost << ")\n";
            sb << "    __intrinsic_op(" << int(op) << ")\n";
            sb << "    __init(vector<" << kBaseTypes[ff].name << ",N> value);\n";

			// Constructor to make a vector from a scalar of another type.
            if (cost != kConversionCost_Impossible)
            {
                cost += kConversionCost_ScalarToVector;
                sb << "    __implicit_conversion(" << cost << ")\n";
                sb << "    [__unsafeForceInlineEarly]\n";
                sb << "    __init(" << kBaseTypes[ff].name << " value) { this = vector<" << kBaseTypes[tt].name << ",N>( " << kBaseTypes[tt].name << "(value)); }\n";
            }
        }
    }
    sb << "}\n";
}

for( int R = 2; R <= 4; ++R )
for( int C = 2; C <= 4; ++C )
{
    sb << "__generic<T, let L:int> __extension matrix<T, " << R << "," << C << ", L>\n{\n";

    // initialize from R*C scalars
    sb << "__intrinsic_op(" << int(kIROp_MakeMatrix) << ") __init(";
    for( int ii = 0; ii < R; ++ii )
    for( int jj = 0; jj < C; ++jj )
    {
        if ((ii+jj) != 0) sb << ", ";
        sb << "T m" << ii << jj;
    }
    sb << ");\n";

    // Initialize from R C-vectors
    sb << "__intrinsic_op(" << int(kIROp_MakeMatrix) << ") __init(";
    for (int ii = 0; ii < R; ++ii)
    {
        if(ii != 0) sb << ", ";
        sb << "vector<T," << C << "> row" << ii;
    }
    sb << ");\n";

    // initialize from a matrix of larger size
    for(int rr = R; rr <= 4; ++rr)
    for( int cc = C; cc <= 4; ++cc )
    {
        if(rr == R && cc == C) continue;
        sb << "__intrinsic_op(" << int(kIROp_MatrixReshape) << ") __init(matrix<T," << rr << "," << cc << ", L> value);\n";
    }
    sb << "}\n";
}

for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    if(kBaseTypes[tt].tag == BaseType::Void) continue;
    auto toType = kBaseTypes[tt].name;
SLANG_RAW("#line 1429 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<let R : int, let C : int, let L : int> extension matrix<")
SLANG_SPLICE(toType
)
SLANG_RAW(",R,C,L>\n")
SLANG_RAW("{\n")

    for (int ff = 0; ff < kBaseTypeCount; ++ff)
    {
        if(kBaseTypes[ff].tag == BaseType::Void) continue;
        if( tt == ff ) continue;

        auto cost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ff]);
        auto fromType = kBaseTypes[ff].name;
        auto op = getBaseTypeConversionOp(
                kBaseTypes[tt],
                kBaseTypes[ff]);
SLANG_RAW("#line 1446 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("    __implicit_conversion(")
SLANG_SPLICE(cost
)
SLANG_RAW(")\n")
SLANG_RAW("    __intrinsic_op(")
SLANG_SPLICE(op
)
SLANG_RAW(")\n")
SLANG_RAW("    __init(matrix<")
SLANG_SPLICE(fromType
)
SLANG_RAW(",R,C,L> value);\n")

    }
SLANG_RAW("#line 1452 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("}\n")

}
SLANG_RAW("#line 1456 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("__intrinsic_op(0)\n")
SLANG_RAW("T __slang_noop_cast(U u);\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("//@ public:\n")
SLANG_RAW("\n")
SLANG_RAW("    /// Sampling state for filtered texture fetches.\n")
SLANG_RAW("__magic_type(SamplerStateType, ")
SLANG_SPLICE(int(SamplerStateFlavor::SamplerState)
)
SLANG_RAW(")\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_SamplerStateType
)
SLANG_RAW(")\n")
SLANG_RAW("struct SamplerState\n")
SLANG_RAW("{\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("    /// Sampling state for filtered texture fetches that include a comparison operation before filtering.\n")
SLANG_RAW("__magic_type(SamplerStateType, ")
SLANG_SPLICE(int(SamplerStateFlavor::SamplerComparisonState)
)
SLANG_RAW(")\n")
SLANG_RAW("__intrinsic_type(")
SLANG_SPLICE(kIROp_SamplerComparisonStateType
)
SLANG_RAW(")\n")
SLANG_RAW("struct SamplerComparisonState\n")
SLANG_RAW("{\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("//@ hidden:\n")
SLANG_RAW("\n")


for (auto op : intrinsicUnaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* resultType = type.name;
        if (op.flags & BOOL_RESULT) resultType = "bool";

        // scalar version
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << type.name << " value);\n";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " __" << op.funcName << "(" << type.name << " value);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << "vector<" << type.name << ",N> value);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" <<  "matrix<" << type.name << ",N,M> value);\n";
    }

    // Synthesize generic versions
    if(op.interface)
    {
        char const* resultType = "T";
        if (op.flags & BOOL_RESULT) resultType = "bool";
        
        // scalar version
        sb << "__generic<T : " << op.interface << ">\n";
        sb << "[OverloadRank(10)]";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << "T value);\n";

        // vector version
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<T,N> value);\n";

        // matrix version
        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<T,N,M> value);\n";
    }
}

SLANG_RAW("#line 1529 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(0)\n")
SLANG_RAW("__prefix Ref<T> operator*(Ptr<T> value);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(0)\n")
SLANG_RAW("__prefix Ptr<T> operator&(__ref T value);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetOffsetPtr
)
SLANG_RAW(")\n")
SLANG_RAW("Ptr<T> operator+(Ptr<T> value, int64_t offset);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("Ptr<T> operator-(Ptr<T> value, int64_t offset)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __getOffsetPtr(value, -offset);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T : IArithmetic>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("__prefix T operator+(T value)\n")
SLANG_RAW("{ return value; }\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T : __BuiltinArithmeticType, let N : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("__prefix vector<T,N> operator+(vector<T,N> value)\n")
SLANG_RAW("{ return value; }\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T : __BuiltinArithmeticType, let R : int, let C : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("__prefix matrix<T,R,C> operator+(matrix<T,R,C> value)\n")
SLANG_RAW("{ return value; }\n")
SLANG_RAW("\n")


static const struct IncDecOpInfo
{
    char const* name;
    char const* binOp;
} kIncDecOps[] =
{
    { "++", "+" },
    { "--", "-" },
};
static const struct IncDecOpFixity
{
    char const* qual;
    char const* bodyPrefix;
    char const* returnVal;
} kIncDecFixities[] =
{
    { "__prefix", "", "value" },
    { "__postfix", " let result = value;", "result" },
};
for(auto op : kIncDecOps)
for(auto fixity : kIncDecFixities)
{
SLANG_RAW("#line 1589 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_SPLICE(fixity.qual
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : __BuiltinArithmeticType>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("T operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("(in out T value)\n")
SLANG_RAW("{")
SLANG_SPLICE(fixity.bodyPrefix
)
SLANG_RAW(" value = value ")
SLANG_SPLICE(op.binOp
)
SLANG_RAW(" T(1); return ")
SLANG_SPLICE(fixity.returnVal
)
SLANG_RAW("; }\n")
SLANG_RAW("\n")
SLANG_SPLICE(fixity.qual
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : __BuiltinArithmeticType, let N : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("vector<T,N> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("(in out vector<T,N> value)\n")
SLANG_RAW("{")
SLANG_SPLICE(fixity.bodyPrefix
)
SLANG_RAW(" value = value ")
SLANG_SPLICE(op.binOp
)
SLANG_RAW(" T(1); return ")
SLANG_SPLICE(fixity.returnVal
)
SLANG_RAW("; }\n")
SLANG_RAW("\n")
SLANG_SPLICE(fixity.qual
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : __BuiltinArithmeticType, let R : int, let C : int, let L : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("matrix<T,R,C> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("(in out matrix<T,R,C,L> value)\n")
SLANG_RAW("{")
SLANG_SPLICE(fixity.bodyPrefix
)
SLANG_RAW(" value = value ")
SLANG_SPLICE(op.binOp
)
SLANG_RAW(" T(1); return ")
SLANG_SPLICE(fixity.returnVal
)
SLANG_RAW("; }\n")
SLANG_RAW("\n")
SLANG_SPLICE(fixity.qual
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("Ptr<T> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("(in out Ptr<T> value)\n")
SLANG_RAW("{")
SLANG_SPLICE(fixity.bodyPrefix
)
SLANG_RAW(" value = value ")
SLANG_SPLICE(op.binOp
)
SLANG_RAW(" 1; return ")
SLANG_SPLICE(fixity.returnVal
)
SLANG_RAW("; }\n")
SLANG_RAW("\n")

}

for (auto op : intrinsicBinaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* leftType = type.name;
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & BOOL_RESULT) resultType = "bool";

        // TODO: We should handle a `SHIFT` flag on the op
        // by changing `rightType` to `int` in order to
        // account for the fact that the shift amount should
        // always have a fixed type independent of the LHS.
        //
        // (It is unclear why this change hadn't been made
        // already, so it is possible that such a change
        // breaks overload resolution or other parts of
        // the compiler)

        // scalar version
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftType << " left, " << rightType << " right);\n";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " __" << op.funcName << "(" << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";

        // We currently synthesize addiitonal overloads
        // for the case where one or the other operand
        // is a scalar. This choice serves a few purposes:
        //
        // 1. It avoids introducing scalar-to-vector or
        // scalar-to-matrix promotions before the operator,
        // which might allow some back ends to produce
        // more optimal code.
        //
        // 2. It avoids concerns about making overload resolution
        // and the inference rules for `N` and `M` able to
        // handle the mixed vector/scalar or matrix/scalar case.
        //
        // 3. Having explicit overloads for the matrix/scalar cases
        // here means that we do *not* need to support a general
        // implicit conversion from scalars to matrices, unless
        // we decide we want to.
        //
        // Note: Case (2) of the motivation shouldn't really apply
        // any more, because we end up having to support similar
        // inteference for built-in binary math functions where
        // vectors and scalars might be combined (and where defining
        // additional overloads to cover all the combinations doesn't
        // seem practical or desirable).
        //
        // TODO: We should consider whether dropping these extra
        // overloads is possible and worth it. The optimization
        // concern (1) could possibly be addressed in specific
        // back-ends. The issue (3) about not wanting to support
        // implicit scalar-to-matrix conversion may be moot if
        // we end up needing to support mixed scalar/matrix input
        // for builtin in non-operator functions anyway.

        // scalar-vector and scalar-matrix
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftType << " left, vector<" << rightType << ",N> right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftType << " left, matrix<" << rightType << ",N,M> right);\n";

        // vector-scalar and matrix-scalar
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }

    // Synthesize generic versions
    if(op.interface)
    {
        char const* leftType = "T";
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & BOOL_RESULT) resultType = "bool";
        // TODO: handle `SHIFT`

        // scalar version
        sb << "__generic<T : " << op.interface << ">\n";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version
        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";

        // scalar-vector and scalar-matrix
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftType << " left, vector<" << rightType << ",N> right);\n";

        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" <<  leftType << " left, matrix<" << rightType << ",N,M> right);\n";

        // vector-scalar and matrix-scalar
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }
}

// We will declare the shift operations entirely as generics
// rather than try to handle all the pairings of left-hand
// and right-hand side types.
//
static const struct ShiftOpInfo
{
    char const* name;
    char const* funcName;
    int op;
} kShiftOps[] =
{
    { "<<", "shl", kIROp_Lsh },
    { ">>", "shr", kIROp_Rsh },
};
for(auto info : kShiftOps) {
SLANG_RAW("#line 1761 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("L operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(L left, R right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("L __")
SLANG_SPLICE(info.funcName
)
SLANG_RAW("(L left, R right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("L operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("=(in out L left, R right)\n")
SLANG_RAW("{\n")
SLANG_RAW("    left = left ")
SLANG_SPLICE(info.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("    return left;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("vector<L,N> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(vector<L,N> left, vector<R,N> right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("vector<L,N> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("=(in out vector<L,N> left, vector<R,N> right)\n")
SLANG_RAW("{\n")
SLANG_RAW("    left = left ")
SLANG_SPLICE(info.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("    return left;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("matrix<L,N,M> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(matrix<L,N,M> left, matrix<R,N,M> right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int, let Layout : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("matrix<L, N, M> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("=(in out matrix<L, N, M, Layout> left, matrix<R, N, M> right)\n")
SLANG_RAW("{\n")
SLANG_RAW("    left = left ")
SLANG_SPLICE(info.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("    return left;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("vector<L,N> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(L left, vector<R,N> right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("matrix<L,N,M> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(L left, matrix<R,N,M> right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("vector<L,N> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(vector<L,N> left, R right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("vector<L, N> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("=(in out vector<L, N> left, R right)\n")
SLANG_RAW("{\n")
SLANG_RAW("    left = left ")
SLANG_SPLICE(info.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("    return left;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(info.op
)
SLANG_RAW(")\n")
SLANG_RAW("matrix<L,N,M> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("(matrix<L,N,M> left, R right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int, let Layout : int>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("matrix<L,N,M> operator")
SLANG_SPLICE(info.name
)
SLANG_RAW("=(in out matrix<L,N,M, Layout> left, R right)\n")
SLANG_RAW("{\n")
SLANG_RAW("    left = left ")
SLANG_SPLICE(info.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("    return left;\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")

}

    static const struct CompoundBinaryOpInfo
    {
        char const* name;
        char const* interface;
    } kCompoundBinaryOps[] =
    {
        { "+",  "__BuiltinArithmeticType" },
        { "-",  "__BuiltinArithmeticType" },
        { "*",  "__BuiltinArithmeticType" },
        { "/",  "__BuiltinArithmeticType" },
        { "%",  "__BuiltinIntegerType" },
        { "%",  "__BuiltinFloatingPointType" },
        { "&",  "__BuiltinLogicalType" },
        { "|",  "__BuiltinLogicalType" },
        { "^",  "__BuiltinLogicalType" },
    };
    for( auto op : kCompoundBinaryOps )
    {
    
SLANG_RAW("#line 1856 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T : ")
SLANG_SPLICE(op.interface
)
SLANG_RAW(">\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    T operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("=(in out T left, T right)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        left = left ")
SLANG_SPLICE(op.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("        return left;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T : ")
SLANG_SPLICE(op.interface
)
SLANG_RAW(", let N : int>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    vector<T,N> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("=(in out vector<T,N> left, vector<T,N> right)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        left = left ")
SLANG_SPLICE(op.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("        return left;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T : ")
SLANG_SPLICE(op.interface
)
SLANG_RAW(", let N : int>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    vector<T,N> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("=(in out vector<T,N> left, T right)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        left = left ")
SLANG_SPLICE(op.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("        return left;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T : ")
SLANG_SPLICE(op.interface
)
SLANG_RAW(", let R : int, let C : int, let Layout : int>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    matrix<T,R,C> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("=(in out matrix<T,R,C,Layout> left, matrix<T,R,C> right)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        left = left ")
SLANG_SPLICE(op.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("        return left;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")
SLANG_RAW("    __generic<T : ")
SLANG_SPLICE(op.interface
)
SLANG_RAW(", let R : int, let C : int, let Layout : int>\n")
SLANG_RAW("    [__unsafeForceInlineEarly]\n")
SLANG_RAW("    matrix<T,R,C> operator")
SLANG_SPLICE(op.name
)
SLANG_RAW("=(in out matrix<T,R,C, Layout> left, T right)\n")
SLANG_RAW("    {\n")
SLANG_RAW("        left = left ")
SLANG_SPLICE(op.name
)
SLANG_RAW(" right;\n")
SLANG_RAW("        return left;\n")
SLANG_RAW("    }\n")
SLANG_RAW("\n")

    }

SLANG_RAW("#line 1901 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("//@ public:\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("// Bit cast\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_BitCast
)
SLANG_RAW(")\n")
SLANG_RAW("T bit_cast(U value);\n")
SLANG_RAW("\n")
SLANG_RAW("// Create Existential object\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_CreateExistentialObject
)
SLANG_RAW(")\n")
SLANG_RAW("T createDynamicObject(uint typeId, U value);\n")
SLANG_RAW("\n")
SLANG_RAW("// Reinterpret\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Reinterpret
)
SLANG_RAW(")\n")
SLANG_RAW("T reinterpret(U value);\n")
SLANG_RAW("\n")
SLANG_RAW("// Use an otherwise unused value\n")
SLANG_RAW("//\n")
SLANG_RAW("// This can be used to silence the warning about returning before initializing\n")
SLANG_RAW("// an out paramter.\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__readNone]\n")
SLANG_RAW("[ForceInline]\n")
SLANG_RAW("void unused(inout T){}\n")
SLANG_RAW("\n")
SLANG_RAW("// Specialized function\n")
SLANG_RAW("\n")
SLANG_RAW("/// Given a string returns an integer hash of that string.\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetStringHash
)
SLANG_RAW(")\n")
SLANG_RAW("int getStringHash(String string);\n")
SLANG_RAW("\n")
SLANG_RAW("/// Use will produce a syntax error in downstream compiler\n")
SLANG_RAW("/// Useful for testing diagnostics around compilation errors of downstream compiler\n")
SLANG_RAW("/// It 'returns' an int so can be used in expressions without the front end complaining.\n")
SLANG_RAW("__target_intrinsic(hlsl, \" @ \")\n")
SLANG_RAW("__target_intrinsic(glsl, \" @ \")\n")
SLANG_RAW("__target_intrinsic(cuda, \" @ \")\n")
SLANG_RAW("__target_intrinsic(cpp, \" @ \")\n")
SLANG_RAW("int __SyntaxError();\n")
SLANG_RAW("\n")
SLANG_RAW("/// For downstream compilers that allow sizeof/alignof/offsetof\n")
SLANG_RAW("/// Can't be called in the C/C++ style. Need to use __size_of<some_type>() as opposed to sizeof(some_type).\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__target_intrinsic(cuda, \"sizeof($G0)\")\n")
SLANG_RAW("__target_intrinsic(cpp, \"sizeof($G0)\")\n")
SLANG_RAW("[__readNone]\n")
SLANG_RAW("int __sizeOf();\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__target_intrinsic(cuda, \"sizeof($T0)\")\n")
SLANG_RAW("__target_intrinsic(cpp, \"sizeof($T0)\")\n")
SLANG_RAW("[__readNone]\n")
SLANG_RAW("int __sizeOf(T v);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__target_intrinsic(cuda, \"SLANG_ALIGN_OF($G0)\")\n")
SLANG_RAW("__target_intrinsic(cpp, \"SLANG_ALIGN_OF($G0)\")\n")
SLANG_RAW("[__readNone]\n")
SLANG_RAW("int __alignOf();\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__target_intrinsic(cuda, \"SLANG_ALIGN_OF($T0)\")\n")
SLANG_RAW("__target_intrinsic(cpp, \"SLANG_ALIGN_OF($T0)\")\n")
SLANG_RAW("[__readNone]\n")
SLANG_RAW("int __alignOf(T v);\n")
SLANG_RAW("\n")
SLANG_RAW("// It would be nice to have offsetof equivalent, but it's not clear how that would work in terms of the Slang language.\n")
SLANG_RAW("// Here we allow calculating the offset of a field in bytes from an *instance* of the type.\n")
SLANG_RAW("__generic<T,F>\n")
SLANG_RAW("__target_intrinsic(cuda, \"int(((char*)&($1)) - ((char*)&($0)))\")\n")
SLANG_RAW("__target_intrinsic(cpp, \"int(((char*)&($1)) - ((char*)&($0))\")\n")
SLANG_RAW("[__readNone]\n")
SLANG_RAW("int __offsetOf(in T t, in F field);\n")
SLANG_RAW("\n")
SLANG_RAW("/// Mark beginning of \"interlocked\" operations in a fragment shader.\n")
SLANG_RAW("__glsl_extension(GL_ARB_fragment_shader_interlock)\n")
SLANG_RAW("__glsl_version(420)\n")
SLANG_RAW("void beginInvocationInterlock()\n")
SLANG_RAW("{\n")
SLANG_RAW("    __target_switch\n")
SLANG_RAW("    {\n")
SLANG_RAW("    case glsl:\n")
SLANG_RAW("        __intrinsic_asm \"beginInvocationInterlockARB\";\n")
SLANG_RAW("    case spirv:\n")
SLANG_RAW("        spirv_asm {\n")
SLANG_RAW("            OpCapability FragmentShaderPixelInterlockEXT;\n")
SLANG_RAW("            OpExtension \"SPV_EXT_fragment_shader_interlock\";\n")
SLANG_RAW("            OpExecutionMode __entryPoint PixelInterlockOrderedEXT;\n")
SLANG_RAW("            OpBeginInvocationInterlockEXT;\n")
SLANG_RAW("        };\n")
SLANG_RAW("    }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("/// Mark end of \"interlocked\" operations in a fragment shader.\n")
SLANG_RAW("__glsl_extension(GL_ARB_fragment_shader_interlock)\n")
SLANG_RAW("__glsl_version(420)\n")
SLANG_RAW("void endInvocationInterlock()\n")
SLANG_RAW("{\n")
SLANG_RAW("    __target_switch\n")
SLANG_RAW("    {\n")
SLANG_RAW("    case glsl:\n")
SLANG_RAW("        __intrinsic_asm \"endInvocationInterlockARB\";\n")
SLANG_RAW("    case spirv:\n")
SLANG_RAW("        spirv_asm {\n")
SLANG_RAW("            OpCapability FragmentShaderPixelInterlockEXT;\n")
SLANG_RAW("            OpExtension \"SPV_EXT_fragment_shader_interlock\";\n")
SLANG_RAW("            OpExecutionMode __entryPoint PixelInterlockOrderedEXT;\n")
SLANG_RAW("            OpEndInvocationInterlockEXT;\n")
SLANG_RAW("        };\n")
SLANG_RAW("    }\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("// Operators to apply to `enum` types\n")
SLANG_RAW("\n")
SLANG_RAW("//@ hidden:\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<E : __EnumType>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Eql
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator==(E left, E right);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<E : __EnumType>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_Neq
)
SLANG_RAW(")\n")
SLANG_RAW("bool operator!=(E left, E right);\n")
SLANG_RAW("\n")
SLANG_RAW("//@ public:\n")
SLANG_RAW("\n")
SLANG_RAW("// public interfaces for generic arithmetic types.\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T : IComparable>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("bool operator<(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.lessThan(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : IComparable>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("bool operator>(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v1.lessThan(v0);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : IComparable>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("bool operator ==(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.equals(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : IComparable>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("bool operator >=(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v1.lessThan(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : IComparable>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("bool operator <=(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.lessThanOrEquals(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : IComparable>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("bool operator !=(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return !v0.equals(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")

const char* arithmeticInterfaces[] = {"IArithmetic", "IFloat"};
const char* attribs[] = {"", "[TreatAsDifferentiable]"};
for (Index i = 0; i < 2; i++) {
    const auto interfaceName = arithmeticInterfaces[i];
    const auto attrib = attribs[i];
    Index overloadRank = i - 3;
SLANG_RAW("#line 2086 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ")
SLANG_SPLICE(interfaceName
)
SLANG_RAW(">\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(")
SLANG_SPLICE(overloadRank
)
SLANG_RAW(")]\n")
SLANG_RAW("T operator +(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.add(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ")
SLANG_SPLICE(interfaceName
)
SLANG_RAW(">\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(")
SLANG_SPLICE(overloadRank
)
SLANG_RAW(")]\n")
SLANG_RAW("T operator -(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.sub(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ")
SLANG_SPLICE(interfaceName
)
SLANG_RAW(">\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(")
SLANG_SPLICE(overloadRank
)
SLANG_RAW(")]\n")
SLANG_RAW("T operator *(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.mul(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ")
SLANG_SPLICE(interfaceName
)
SLANG_RAW(">\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(")
SLANG_SPLICE(overloadRank
)
SLANG_RAW(")]\n")
SLANG_RAW("T operator /(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.div(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ")
SLANG_SPLICE(interfaceName
)
SLANG_RAW(">\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(")
SLANG_SPLICE(overloadRank
)
SLANG_RAW(")]\n")
SLANG_RAW("T operator %(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.mod(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_SPLICE(attrib
)
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ")
SLANG_SPLICE(interfaceName
)
SLANG_RAW(">\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(")
SLANG_SPLICE(overloadRank
)
SLANG_RAW(")]\n")
SLANG_RAW("__prefix T operator -(T v0)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.neg();\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")

    } // foreach ["IArithmetic", "IFloat"]
SLANG_RAW("#line 2143 \"core.meta.slang\"")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("T operator &(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.bitAnd(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("T operator &&(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.and(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("T operator |(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.bitOr(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("T operator ||(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.or(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("T operator ^(T v0, T v1)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.bitXor(v1);\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("__prefix T operator ~(T v0)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.bitNot();\n")
SLANG_RAW("}\n")
SLANG_RAW("__generic<T : ILogical>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("[OverloadRank(-10)]\n")
SLANG_RAW("__prefix T operator !(T v0)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return v0.not();\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("// IR level type traits.\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_undefined
)
SLANG_RAW(")\n")
SLANG_RAW("T __declVal();\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_DefaultConstruct
)
SLANG_RAW(")\n")
SLANG_RAW("T __default();\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_TypeEquals
)
SLANG_RAW(")\n")
SLANG_RAW("bool __type_equals_impl(T t, U u);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __type_equals(T t, U u)\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __type_equals_impl(__declVal<T>(), __declVal<U>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T, U>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __type_equals()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __type_equals_impl(__declVal<T>(), __declVal<U>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsBool
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isBool_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isBool()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isBool_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsInt
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isInt_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isInt()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isInt_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsFloat
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isFloat_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsHalf
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isHalf_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isFloat()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isFloat_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isHalf()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isHalf_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsUnsignedInt
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isUnsignedInt_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isUnsignedInt()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isUnsignedInt_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsSignedInt
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isSignedInt_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isSignedInt()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isSignedInt_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_IsVector
)
SLANG_RAW(")\n")
SLANG_RAW("bool __isVector_impl(T t);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("bool __isVector()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __isVector_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_GetNaturalStride
)
SLANG_RAW(")\n")
SLANG_RAW("int __naturalStrideOf_impl(T v);\n")
SLANG_RAW("\n")
SLANG_RAW("__generic<T>\n")
SLANG_RAW("[__unsafeForceInlineEarly]\n")
SLANG_RAW("int __naturalStrideOf()\n")
SLANG_RAW("{\n")
SLANG_RAW("    return __naturalStrideOf_impl(__declVal<T>());\n")
SLANG_RAW("}\n")
SLANG_RAW("\n")
SLANG_RAW("__intrinsic_op(")
SLANG_SPLICE(kIROp_TreatAsDynamicUniform
)
SLANG_RAW(")\n")
SLANG_RAW("T asDynamicUniform<T>(T v);\n")
SLANG_RAW("\n")
SLANG_RAW("// Binding Attributes\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [vk_binding(binding: int, set: int = 0)]\t\t\t: GLSLBindingAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [gl_binding(binding: int, set: int = 0)]\t\t\t: GLSLBindingAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [vk_shader_record]\t\t\t                        : ShaderRecordAttribute;\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [shader_record]\t\t\t                        : ShaderRecordAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [vk_push_constant]\t\t\t\t\t\t\t\t\t: PushConstantAttribute;\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [push_constant]\t\t\t\t\t\t\t\t\t: PushConstantAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [vk_location(locaiton : int)] : GLSLLocationAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [vk_index(index : int)] : GLSLIndexAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [vk_spirv_instruction(op : int, set : String = \"\")]     : SPIRVInstructionOpAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [spv_target_env_1_3] : SPIRVTargetEnv13Attribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [disable_array_flattening] : DisableArrayFlatteningAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Statement Attributes\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(LoopStmt)\n")
SLANG_RAW("attribute_syntax [unroll(count: int = 0)]   : UnrollAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(LoopStmt)\n")
SLANG_RAW("attribute_syntax [ForceUnroll(count: int = 0)]   : ForceUnrollAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(LoopStmt)\n")
SLANG_RAW("attribute_syntax [loop]                 : LoopAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(LoopStmt)\n")
SLANG_RAW("attribute_syntax [fastopt]              : FastOptAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(LoopStmt)\n")
SLANG_RAW("attribute_syntax [allow_uav_condition]  : AllowUAVConditionAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(LoopStmt)\n")
SLANG_RAW("attribute_syntax [MaxIters(count)]      : MaxItersAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(IfStmt)\n")
SLANG_RAW("attribute_syntax [flatten]              : FlattenAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(IfStmt)\n")
SLANG_RAW("__attributeTarget(SwitchStmt)\n")
SLANG_RAW("attribute_syntax [branch]               : BranchAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(SwitchStmt)\n")
SLANG_RAW("attribute_syntax [forcecase]            : ForceCaseAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(SwitchStmt)\n")
SLANG_RAW("attribute_syntax [call]                 : CallAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Entry-point Attributes\n")
SLANG_RAW("\n")
SLANG_RAW("// All Stages\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [shader(stage)]    : EntryPointAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Hull Shader\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [maxtessfactor(factor: float)]     : MaxTessFactorAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [outputcontrolpoints(count: int)]  : OutputControlPointsAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [outputtopology(topology)]         : OutputTopologyAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [partitioning(mode)]               : PartitioningAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [patchconstantfunc(name)]          : PatchConstantFuncAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Hull/Domain Shader\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [domain(domain)]   : DomainAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Geometry Shader\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [maxvertexcount(count: int)]   : MaxVertexCountAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [instance(count: int)]         : InstanceAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Fragment (\"Pixel\") Shader\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [earlydepthstencil]    : EarlyDepthStencilAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Compute Shader\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [numthreads(x: int, y: int = 1, z: int = 1)]   : NumThreadsAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("//\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [__vulkanRayPayload(location : int = -1)] : VulkanRayPayloadAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [__vulkanCallablePayload(location : int = -1)] : VulkanCallablePayloadAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [__vulkanHitObjectAttributes(location : int = -1)] : VulkanHitObjectAttributesAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [__vulkanHitAttributes] : VulkanHitAttributesAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [mutating] : MutatingAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(SetterDecl)\n")
SLANG_RAW("attribute_syntax [nonmutating] : NonmutatingAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [constref] : ConstRefAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("    /// Indicates that a function computes its result as a function of its arguments without loading/storing any memory or other state.\n")
SLANG_RAW("    ///\n")
SLANG_RAW("    /// This is equivalent to the LLVM `readnone` function attribute.\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [__readNone] : ReadNoneAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("enum _AttributeTargets\n")
SLANG_RAW("{\n")
SLANG_RAW("    Struct = ")
SLANG_SPLICE( (int) UserDefinedAttributeTargets::Struct
)
SLANG_RAW(",\n")
SLANG_RAW("    Var = ")
SLANG_SPLICE( (int) UserDefinedAttributeTargets::Var
)
SLANG_RAW(",\n")
SLANG_RAW("    Function = ")
SLANG_SPLICE( (int) UserDefinedAttributeTargets::Function
)
SLANG_RAW(",\n")
SLANG_RAW("};\n")
SLANG_RAW("__attributeTarget(StructDecl)\n")
SLANG_RAW("attribute_syntax [__AttributeUsage(target : _AttributeTargets)] : AttributeUsageAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [format(format : String)] : FormatAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(VarDeclBase)\n")
SLANG_RAW("attribute_syntax [vk_image_format(format : String)] : FormatAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(Decl)\n")
SLANG_RAW("attribute_syntax [allow(diagnostic: String)] : AllowAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(Decl)\n")
SLANG_RAW("attribute_syntax[require(capability)] : RequireCapabilityAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Linking\n")
SLANG_RAW("__attributeTarget(Decl)\n")
SLANG_RAW("attribute_syntax [__extern] : ExternAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [__unsafeForceInlineEarly] : UnsafeForceInlineEarlyAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [ForceInline] : ForceInlineAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [OverloadRank] : OverloadRankAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [DllImport(modulePath: String)] : DllImportAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [DllExport] : DllExportAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [TorchEntryPoint] : TorchEntryPointAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [CudaDeviceExport] : CudaDeviceExportAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [CudaHost] : CudaHostAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [CudaKernel] : CudaKernelAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [CUDADeviceExport] : CudaDeviceExportAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [CUDAHost] : CudaHostAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax [CUDAKernel] : CudaKernelAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FuncDecl)\n")
SLANG_RAW("attribute_syntax[AutoPyBindCUDA] : AutoPyBindCudaAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(AggTypeDecl)\n")
SLANG_RAW("attribute_syntax [PyExport(name: String)] : PyExportAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(InterfaceDecl)\n")
SLANG_RAW("attribute_syntax [COM(guid: String)] : ComInterfaceAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("// Inheritance Control\n")
SLANG_RAW("__attributeTarget(AggTypeDecl)\n")
SLANG_RAW("attribute_syntax [sealed] : SealedAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(AggTypeDecl)\n")
SLANG_RAW("attribute_syntax [open] : OpenAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(InterfaceDecl)\n")
SLANG_RAW("attribute_syntax [anyValueSize(size:int)] : AnyValueSizeAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(InterfaceDecl)\n")
SLANG_RAW("attribute_syntax [Specialize] : SpecializeAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [builtin] : BuiltinAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [__requiresNVAPI] : RequiresNVAPIAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [__AlwaysFoldIntoUseSiteAttribute] : AlwaysFoldIntoUseSiteAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [noinline] : NoInlineAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(StructDecl)\n")
SLANG_RAW("attribute_syntax [payload] : PayloadAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [deprecated(message: String)] : DeprecatedAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [PreferRecompute] : PreferRecomputeAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [PreferCheckpoint] : PreferCheckpointAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(DeclBase)\n")
SLANG_RAW("attribute_syntax [KnownBuiltin(name : String)] : KnownBuiltinAttribute;\n")
SLANG_RAW("\n")
SLANG_RAW("__attributeTarget(FunctionDeclBase)\n")
SLANG_RAW("attribute_syntax [NonUniformReturn] : NonDynamicUniformAttribute;\n")
