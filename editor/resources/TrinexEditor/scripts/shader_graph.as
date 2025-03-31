namespace Engine::VisualMaterialGraph
{
	class Constant : Node
	{
		Constant(ShaderParameterType type)
		{
			new_output("Out", type, type);
		}

		Expression compile(OutputPin@ pin, Compiler@ compiler) override { return compiler.compile_default(pin); }
		
		void create(ShaderParameterType type)
		{
			new_output("Out", ShaderParameterType::Float, ShaderParameterType::Float);
		}
	};

	class Arithmetic : Node
	{
		InputPin@ a = new_input("A", ShaderParameterType::META_Numeric, ShaderParameterType::Float);
		InputPin@ b = new_input("B", ShaderParameterType::META_Numeric, ShaderParameterType::Float);
		OutputPin@ output = new_output("Out", ShaderParameterType::META_Numeric);

		string operator() const { return ""; }

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression a_expr = compiler.compile(a);
			Expression b_expr = compiler.compile(b);

			ShaderParameterType result_type = Expression::static_resolve(a_expr.type, b_expr.type);
			
			a_expr = a_expr.convert(result_type);
			b_expr = b_expr.convert(result_type);
			
			return Expression(result_type, operator().format(a_expr.value, b_expr.value));
		}
	};

	[node_group("Constants")] class ConstantBool  : Constant { ConstantBool()  { super(ShaderParameterType::Bool);  } };
	[node_group("Constants")] class ConstantBool2 : Constant { ConstantBool2() { super(ShaderParameterType::Bool2); } };
	[node_group("Constants")] class ConstantBool3 : Constant { ConstantBool3() { super(ShaderParameterType::Bool3); } };
	[node_group("Constants")] class ConstantBool4 : Constant { ConstantBool4() { super(ShaderParameterType::Bool4); } };

	[node_group("Constants")] class ConstantInt  : Constant { ConstantInt()  { super(ShaderParameterType::Int);  } };
	[node_group("Constants")] class ConstantInt2 : Constant { ConstantInt2() { super(ShaderParameterType::Int2); } };
	[node_group("Constants")] class ConstantInt3 : Constant { ConstantInt3() { super(ShaderParameterType::Int3); } };
	[node_group("Constants")] class ConstantInt4 : Constant { ConstantInt4() { super(ShaderParameterType::Int4); } };

	[node_group("Constants")] class ConstantUInt  : Constant { ConstantUInt()  { super(ShaderParameterType::UInt);  } };
	[node_group("Constants")] class ConstantUInt2 : Constant { ConstantUInt2() { super(ShaderParameterType::UInt2); } };
	[node_group("Constants")] class ConstantUInt3 : Constant { ConstantUInt3() { super(ShaderParameterType::UInt3); } };
	[node_group("Constants")] class ConstantUInt4 : Constant { ConstantUInt4() { super(ShaderParameterType::UInt4); } };

	[node_group("Constants")] class ConstantFloat  : Constant { ConstantFloat()  { super(ShaderParameterType::Float);  } };
	[node_group("Constants")] class ConstantFloat2 : Constant { ConstantFloat2() { super(ShaderParameterType::Float2); } };
	[node_group("Constants")] class ConstantFloat3 : Constant { ConstantFloat3() { super(ShaderParameterType::Float3); } };
	[node_group("Constants")] class ConstantFloat4 : Constant { ConstantFloat4() { super(ShaderParameterType::Float4); } };

	[node_group("Math")] class Add : Arithmetic { string operator() const { return "(%0 + %1)"; } };
	[node_group("Math")] class Sub : Arithmetic { string operator() const { return "(%0 - %1)"; } };
	[node_group("Math")] class Mul : Arithmetic { string operator() const { return "(%0 * %1)"; } };
	[node_group("Math")] class Div : Arithmetic { string operator() const { return "(%0 / %1)"; } };
}