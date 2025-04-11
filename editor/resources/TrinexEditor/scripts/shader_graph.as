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

	class Unary : Node
	{
		InputPin@ m_in0 = new_input("In", in0_type(), in0_default_type());
		OutputPin@ m_out0 = new_output("Out", out0_type());

		ShaderParameterType in0_type() const { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in0_default_type() const { return ShaderParameterType::Float; }
		ShaderParameterType out0_type() const { return ShaderParameterType::META_Numeric; }
		
		string expr() const { return ""; }
		ShaderParameterType resolve_args(Expression& in0) const { return in0.type; }

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			ShaderParameterType result_type = resolve_args(in0);
			return Expression(result_type, expr().format(in0.value));
		}
	};

	class Binary : Node
	{
		InputPin@ m_in0 = new_input(in0_name(), in0_type(), in0_default_type());
		InputPin@ m_in1 = new_input(in1_name(), in1_type(), in1_default_type());
		OutputPin@ m_out0 = new_output("Out", out0_type());

		string in0_name() const { return "A"; }
		string in1_name() const { return "B"; }
		
		ShaderParameterType in0_type() const { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in0_default_type() const { return ShaderParameterType::Float; }
		ShaderParameterType in1_type() const { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in1_default_type() const { return ShaderParameterType::Float; }
		ShaderParameterType out0_type() const { return ShaderParameterType::META_Numeric; }
		
		string expr() const { return ""; }
		
		ShaderParameterType resolve_args(Expression& in0, Expression& in1) const 
		{
			ShaderParameterType type = Expression::static_resolve(in0.type, in1.type);
			in0 = in0.convert(type);
			in1 = in1.convert(type);
			return type;
		}
		
		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			Expression in1 = compiler.compile(m_in1);
			ShaderParameterType result_type = resolve_args(in0, in1);		
			return Expression(result_type, expr().format(in0.value, in1.value));
		}
	};

	class Ternary : Node
	{
		InputPin@ m_in0 = new_input(in0_name(), in0_type(), in0_default_type());
		InputPin@ m_in1 = new_input(in1_name(), in1_type(), in1_default_type());
		InputPin@ m_in2 = new_input(in2_name(), in2_type(), in2_default_type());
		OutputPin@ m_out0 = new_output("Out", ShaderParameterType::META_Numeric);

		string in0_name() const { return "A"; }
		string in1_name() const { return "B"; }
		string in2_name() const { return "C"; }
		
		ShaderParameterType in0_type() const { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in0_default_type() const { return ShaderParameterType::Float; }
		ShaderParameterType in1_type() const { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in1_default_type() const { return ShaderParameterType::Float; }
		ShaderParameterType in2_type() const { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in2_default_type() const { return ShaderParameterType::Float; }
		ShaderParameterType out0_type() const { return ShaderParameterType::META_Numeric; }
		
		string expr() const { return ""; }
		
		ShaderParameterType resolve_args(Expression& in0, Expression& in1, Expression& in2) const 
		{
			ShaderParameterType type = Expression::static_resolve(in0.type, in1.type);
			type = Expression::static_resolve(type, in2.type);
			
			in0 = in0.convert(type);
			in1 = in1.convert(type);
			in2 = in2.convert(type);
			return type;
		}

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			Expression in1 = compiler.compile(m_in1);
			Expression in2 = compiler.compile(m_in2);

			ShaderParameterType result_type = resolve_args(in0, in1, in2);
			return Expression(result_type, expr().format(in0.value, in1.value, in2.value));
		}
	};

	class FloatUnary : Unary
	{
		ShaderParameterType resolve_args(Expression& in0) const override 
		{ 
			in0 = in0.to_floating();
			return in0.type; 
		}
	};

	class FloatBinary : Binary
	{
		ShaderParameterType resolve_args(Expression& in0, Expression& in1) const override 
		{ 
			ShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			in0 = in0.convert(type); 
			in1 = in1.convert(type); 
			return type;
		}
	};

	class Float3Binary : Binary
	{
		ShaderParameterType in0_type() const override { return ShaderParameterType::Float3; }
		ShaderParameterType in0_default_type() const override { return ShaderParameterType::Float3; }
		ShaderParameterType in1_type() const override { return ShaderParameterType::Float3; }
		ShaderParameterType in1_default_type() const override { return ShaderParameterType::Float3; }
	};

	class FloatTernary : Ternary
	{
		ShaderParameterType resolve_args(Expression& in0, Expression& in1, Expression& in2) const override 
		{ 
			ShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			type = Expression::static_resolve(type, Expression::static_make_float(in2.type));

			in0 = in0.convert(type); 
			in1 = in1.convert(type); 
			in2 = in2.convert(type);
			
			return type; 
		}
	};

	/////////////////////////////// CONSTANTS ///////////////////////////////

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


	/////////////////////////////// MATH ///////////////////////////////

	[node_group("Math")] class Add : Binary { string expr() const override { return "(%0 + %1)"; } };
	[node_group("Math")] class Sub : Binary { string expr() const override { return "(%0 - %1)"; } };
	[node_group("Math")] class Mul : Binary { string expr() const override { return "(%0 * %1)"; } };
	[node_group("Math")] class Div : Binary { string expr() const override { return "(%0 / %1)"; } };
	
	[node_group("Math")] class Min : Binary { string expr() const override { return "min(%0, %1)"; } };
	[node_group("Math")] class Max : Binary { string expr() const override { return "min(%0, %1)"; } };
	[node_group("Math")] class Clamp : Ternary { string expr() const override { return "clamp(%0, %1, %2)"; } };
	
	[node_group("Math")] class Abs : FloatUnary { string expr() const override { return "abs(%0)"; } };
	[node_group("Math")] class Floor : FloatUnary { string expr() const override { return "floor(%0)"; } };
	[node_group("Math")] class Ceil : FloatUnary { string expr() const override { return "ceil(%0)"; } };
	[node_group("Math")] class Frac : FloatUnary { string expr() const override { return "frac(%0)"; } };
	[node_group("Math")] class Sin : FloatUnary { string expr() const override { return "sin(%0)"; } };
	[node_group("Math")] class Cos : FloatUnary { string expr() const override { return "cos(%0)"; } };
	[node_group("Math")] class Tan : FloatUnary { string expr() const override { return "tan(%0)"; } };
	[node_group("Math")] class Normalize : FloatUnary { string expr() const override { return "normalize(%0)"; } };
	[node_group("Math")] class Exp : FloatUnary { string expr() const override { return "exp(%0)"; } };
	[node_group("Math")] class Exp2 : FloatUnary { string expr() const override { return "exp2(%0)"; } };
	[node_group("Math")] class Sqrt : FloatUnary { string expr() const override { return "sqrt(%0)"; } };
	[node_group("Math")] class InverseSqrt : FloatUnary { string expr() const override { return "rsqrt(%0)"; } };
	[node_group("Math")] class Saturate : FloatUnary { string expr() const override { return "saturate(%0)"; } };

	[node_group("Math")] class Power : FloatBinary { string expr() const override { return "pow(%0, %1)"; } };
	[node_group("Math")] class FMod : FloatBinary { string expr() const override { return "fmod(%0, %1)"; } };
	[node_group("Math")] class Dot : Float3Binary { string expr() const override { return "dot(%0, %1)"; } };
	[node_group("Math")] class Cross : Float3Binary { string expr() const override { return "cross(%0, %1)"; } };

	[node_group("Math")] 
	class Reflect : Binary
	{ 
		ShaderParameterType in0_type() const override { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in0_default_type() const override { return ShaderParameterType::Float3; }
		ShaderParameterType in1_type() const override { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in1_default_type() const override { return ShaderParameterType::Float3; }
		string expr() const override { return "reflect(%0, %1)"; } 
		
		ShaderParameterType resolve_args(Expression& in0, Expression& in1) const override 
		{
			ShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			type = Expression::static_vector_clamp(type, 1, 3);
			
			in0 = in0.convert(type);
			in1 = in1.convert(type);
			return type;
		}
	};

	[node_group("Math")]
	class Refract : Ternary
	{ 
		ShaderParameterType in0_type() const override { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in0_default_type() const override { return ShaderParameterType::Float3; }
		ShaderParameterType in1_type() const override { return ShaderParameterType::META_Numeric; }
		ShaderParameterType in1_default_type() const override { return ShaderParameterType::Float3; }
		ShaderParameterType in2_type() const override { return ShaderParameterType::Float; }
		ShaderParameterType in2_default_type() const override { return ShaderParameterType::Float; }
		
		string expr() const override { return "refract(%0, %1, %2)"; } 
		
		ShaderParameterType resolve_args(Expression& in0, Expression& in1, Expression& in2) const override 
		{
			ShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			
			in0 = in0.convert(type);
			in1 = in1.convert(type);
			return type;
		}
	};

	[node_group("Math")] class Lerp : FloatTernary { string expr() const override { return "lerp(%0, %1, %2)"; } };
	[node_group("Math")] class Step : FloatTernary { string expr() const override { return "step(%0, %1, %2)"; } };
	[node_group("Math")] class SmoothStep : FloatTernary { string expr() const override { return "smoothstep(%0, %1, %2)"; } };

	/////////////////////////////// COMMON ///////////////////////////////

	[node_group("Common")]
	class ComponentMask : Node
	{
		[property] bool x = true;
		[property] bool y = true;
		[property] bool z = true;
		[property] bool w = true;

		InputPin@ in0 = new_input("In", ShaderParameterType::META_Numeric);
		OutputPin@ out0 = new_output("Out", ShaderParameterType::META_Numeric);

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			uint input_len = 0;
			string swizzle;

			if (x) { swizzle += "x"; input_len = 1; }
			if (y) { swizzle += "y"; input_len = 2; }
			if (z) { swizzle += "z"; input_len = 3; }
			if (w) { swizzle += "w"; input_len = 4; }

			Expression expression = compiler.compile(in0);
			expression = expression.convert(Expression::static_make_vector(expression.type, input_len));

			if(swizzle.length() != input_len)
			{
				expression.value += ".";
				expression.value += swizzle;

				expression.type = Expression::static_make_vector(expression.type, swizzle.length());
			}

			return expression;
		}
	};

	/////////////////////////////// CONDITIONS ///////////////////////////////

	[node_group("Conditions")]
	class If : Node
	{
		InputPin@ m_in0 = new_input("A", ShaderParameterType::META_Numeric, ShaderParameterType::Float);
		InputPin@ m_in1 = new_input("B", ShaderParameterType::META_Numeric, ShaderParameterType::Float);
		InputPin@ m_in2 = new_input("A > B", ShaderParameterType::META_Any);
		InputPin@ m_in3 = new_input("A == B", ShaderParameterType::META_Any);
		InputPin@ m_in4 = new_input("A < B", ShaderParameterType::META_Any);
		OutputPin@ m_out0 = new_output("Out", ShaderParameterType::META_Any);

		Expression compile2(Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0).vector_length();
			Expression in1 = compiler.compile(m_in1).vector_length();

			Expression in2 = compiler.compile(m_in2);
			Expression in4 = compiler.compile(m_in4);

			ShaderParameterType type = Expression::static_resolve(in2.type, in4.type);
			in2 = in2.convert(type);
			in4 = in4.convert(type);

			return Expression(type, "(%0 > %1 ? %2 : %3)".format(in0.value, in1.value, in2.value, in4.value));
		}

		Expression compile3(Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0).vector_length();
			Expression in1 = compiler.compile(m_in1).vector_length();

			Expression in2 = compiler.compile(m_in2);
			Expression in3 = compiler.compile(m_in3);
			Expression in4 = compiler.compile(m_in4);

			ShaderParameterType type = Expression::static_resolve(in2.type, in3.type);
			type = Expression::static_resolve(type, in4.type);

			in2 = in2.convert(type);
			in3 = in3.convert(type);
			in4 = in4.convert(type);

			string expr = "(%0 > %1 ? %2 : %0 < %1 ? %4 : %3)";
			return Expression(type, expr.format(in0.value, in1.value, in2.value, in3.value, in4.value));
		}

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			return m_in3.links_count() == 0 ? compile2(compiler) : compile3(compiler);
		}
	};


	[node_group("Conditions")]
	class Branch : Node
	{
		InputPin@ m_in0 = new_input("Condition", ShaderParameterType::Bool, ShaderParameterType::Bool);
		InputPin@ m_in1 = new_input("True", ShaderParameterType::META_Any);
		InputPin@ m_in2 = new_input("False", ShaderParameterType::META_Any);
		OutputPin@ m_out0 = new_output("Out", ShaderParameterType::META_Any);

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			
			Expression in1 = compiler.compile(m_in1);
			Expression in2 = compiler.compile(m_in2);

			const ShaderParameterType type = Expression::static_resolve(in1.type, in2.type);
			in1 = in1.convert(type);
			in2 = in2.convert(type);

			return Expression(type, "(%0 ? %1 : %2)".format(in0.value, in1.value, in2.value));
		}
	};
}