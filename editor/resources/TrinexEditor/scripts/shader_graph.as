namespace Engine::VisualMaterialGraph
{
	[node_group("Math")]
	class Add : Node
	{
		InputPin@ a = new_input("A", ShaderParameterType::META_Numeric);
		InputPin@ b = new_input("B", ShaderParameterType::META_Numeric);
		OutputPin@ output = new_output("Out", ShaderParameterType::META_Numeric);

		Expression compile(OutputPin@ pin, Compiler& compiler)
		{
			Expression a_expr = compiler.compile(a);
			Expression b_expr = compiler.compile(b);

			ShaderParameterType result_type = Expression::static_resolve(a_expr.type, b_expr.type);
			
			a_expr = a_expr.convert(result_type);
			b_expr = b_expr.convert(result_type);
			
			return Expression(result_type, "(%1 + %2)".format(a_expr.value, b_expr.value));
		}
	};
}