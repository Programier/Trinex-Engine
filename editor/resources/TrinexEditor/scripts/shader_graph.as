namespace Engine::VisualMaterialGraph
{
	class CommonInput : Node
	{
		CommonInput(RHIShaderParameterType type)
		{
			new_output("Out", type);
		}

		string expr() const { return ""; }

		Expression compile(OutputPin@ pin, Compiler@ compiler) override 
		{ 
			return Expression(pin.type(), expr());
		}
	};

	class Constant : Node
	{
		Constant(RHIShaderParameterType type)
		{
			new_output("Out", type, type);
		}

		Expression compile(OutputPin@ pin, Compiler@ compiler) override { return compiler.compile_default(pin); }
		
		void create(RHIShaderParameterType type)
		{
			new_output("Out", RHIShaderParameterType::Float, RHIShaderParameterType::Float);
		}
	};

	class Unary : Node
	{
		InputPin@ m_in0 = new_input("In", in0_type(), in0_default_type());
		OutputPin@ m_out0 = new_output("Out", out0_type());

		RHIShaderParameterType in0_type() const { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in0_default_type() const { return RHIShaderParameterType::Float; }
		RHIShaderParameterType out0_type() const { return RHIShaderParameterType::META_Numeric; }
		
		string expr() const { return ""; }
		RHIShaderParameterType resolve_args(Expression& in0) const { return in0.type; }

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			RHIShaderParameterType result_type = resolve_args(in0);
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
		
		RHIShaderParameterType in0_type() const { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in0_default_type() const { return RHIShaderParameterType::Float; }
		RHIShaderParameterType in1_type() const { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in1_default_type() const { return RHIShaderParameterType::Float; }
		RHIShaderParameterType out0_type() const { return RHIShaderParameterType::META_Numeric; }
		
		string expr() const { return ""; }
		
		RHIShaderParameterType resolve_args(Expression& in0, Expression& in1) const 
		{
			RHIShaderParameterType type = Expression::static_resolve(in0.type, in1.type);
			in0 = in0.convert(type);
			in1 = in1.convert(type);
			return type;
		}
		
		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			Expression in1 = compiler.compile(m_in1);
			RHIShaderParameterType result_type = resolve_args(in0, in1);		
			return Expression(result_type, expr().format(in0.value, in1.value));
		}
	};

	class Ternary : Node
	{
		InputPin@ m_in0 = new_input(in0_name(), in0_type(), in0_default_type());
		InputPin@ m_in1 = new_input(in1_name(), in1_type(), in1_default_type());
		InputPin@ m_in2 = new_input(in2_name(), in2_type(), in2_default_type());
		OutputPin@ m_out0 = new_output("Out", RHIShaderParameterType::META_Numeric);

		string in0_name() const { return "A"; }
		string in1_name() const { return "B"; }
		string in2_name() const { return "C"; }
		
		RHIShaderParameterType in0_type() const { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in0_default_type() const { return RHIShaderParameterType::Float; }
		RHIShaderParameterType in1_type() const { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in1_default_type() const { return RHIShaderParameterType::Float; }
		RHIShaderParameterType in2_type() const { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in2_default_type() const { return RHIShaderParameterType::Float; }
		RHIShaderParameterType out0_type() const { return RHIShaderParameterType::META_Numeric; }
		
		string expr() const { return ""; }
		
		RHIShaderParameterType resolve_args(Expression& in0, Expression& in1, Expression& in2) const 
		{
			RHIShaderParameterType type = Expression::static_resolve(in0.type, in1.type, in2.type);
			
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

			RHIShaderParameterType result_type = resolve_args(in0, in1, in2);
			return Expression(result_type, expr().format(in0.value, in1.value, in2.value));
		}
	};

	class FloatUnary : Unary
	{
		RHIShaderParameterType resolve_args(Expression& in0) const override 
		{ 
			in0 = in0.to_floating();
			return in0.type; 
		}
	};

	class FloatBinary : Binary
	{
		RHIShaderParameterType resolve_args(Expression& in0, Expression& in1) const override 
		{ 
			RHIShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			in0 = in0.convert(type); 
			in1 = in1.convert(type); 
			return type;
		}
	};

	class Float3Binary : Binary
	{
		RHIShaderParameterType in0_type() const override { return RHIShaderParameterType::Float3; }
		RHIShaderParameterType in0_default_type() const override { return RHIShaderParameterType::Float3; }
		RHIShaderParameterType in1_type() const override { return RHIShaderParameterType::Float3; }
		RHIShaderParameterType in1_default_type() const override { return RHIShaderParameterType::Float3; }
	};

	class FloatTernary : Ternary
	{
		RHIShaderParameterType resolve_args(Expression& in0, Expression& in1, Expression& in2) const override 
		{ 
			RHIShaderParameterType type = Expression::static_resolve(
				Expression::static_make_float(in0.type), 
				Expression::static_make_float(in1.type),
				Expression::static_make_float(in2.type)
			);
			
			in0 = in0.convert(type); 
			in1 = in1.convert(type); 
			in2 = in2.convert(type);
			
			return type; 
		}
	};

	/////////////////////////////// CONSTANTS ///////////////////////////////

	[node_group("Constants")] class ConstantBool  : Constant { ConstantBool()  { super(RHIShaderParameterType::Bool);  } };
	[node_group("Constants")] class ConstantBool2 : Constant { ConstantBool2() { super(RHIShaderParameterType::Bool2); } };
	[node_group("Constants")] class ConstantBool3 : Constant { ConstantBool3() { super(RHIShaderParameterType::Bool3); } };
	[node_group("Constants")] class ConstantBool4 : Constant { ConstantBool4() { super(RHIShaderParameterType::Bool4); } };

	[node_group("Constants")] class ConstantInt  : Constant { ConstantInt()  { super(RHIShaderParameterType::Int);  } };
	[node_group("Constants")] class ConstantInt2 : Constant { ConstantInt2() { super(RHIShaderParameterType::Int2); } };
	[node_group("Constants")] class ConstantInt3 : Constant { ConstantInt3() { super(RHIShaderParameterType::Int3); } };
	[node_group("Constants")] class ConstantInt4 : Constant { ConstantInt4() { super(RHIShaderParameterType::Int4); } };

	[node_group("Constants")] class ConstantUInt  : Constant { ConstantUInt()  { super(RHIShaderParameterType::UInt);  } };
	[node_group("Constants")] class ConstantUInt2 : Constant { ConstantUInt2() { super(RHIShaderParameterType::UInt2); } };
	[node_group("Constants")] class ConstantUInt3 : Constant { ConstantUInt3() { super(RHIShaderParameterType::UInt3); } };
	[node_group("Constants")] class ConstantUInt4 : Constant { ConstantUInt4() { super(RHIShaderParameterType::UInt4); } };

	[node_group("Constants")] class ConstantFloat  : Constant { ConstantFloat()  { super(RHIShaderParameterType::Float);  } };
	[node_group("Constants")] class ConstantFloat2 : Constant { ConstantFloat2() { super(RHIShaderParameterType::Float2); } };
	[node_group("Constants")] class ConstantFloat3 : Constant { ConstantFloat3() { super(RHIShaderParameterType::Float3); } };
	[node_group("Constants")] class ConstantFloat4 : Constant { ConstantFloat4() { super(RHIShaderParameterType::Float4); } };

	/////////////////////////////// INPUTS ///////////////////////////////

	[node_group("Inputs")] class Projection : CommonInput 
	{ 
		Projection() { super(RHIShaderParameterType::Float4x4); } 
		string expr() const override { return "scene_view.projection"; }	
	};

	[node_group("Inputs")] class View : CommonInput 
	{ 
		View() { super(RHIShaderParameterType::Float4x4); } 
		string expr() const override { return "scene_view.view"; }	
	};

	[node_group("Inputs")] class ProjView : CommonInput 
	{ 
		ProjView() { super(RHIShaderParameterType::Float4x4); } 
		string expr() const override { return "scene_view.projview"; }	
	};

	[node_group("Inputs")] class InvProjView : CommonInput 
	{ 
		InvProjView() { super(RHIShaderParameterType::Float4x4); } 
		string expr() const override { return "scene_view.inv_projview"; }	
	};

	[node_group("Inputs")] class ViewportPos : CommonInput 
	{ 
		ViewportPos() { super(RHIShaderParameterType::Float2); } 
		string expr() const override { return "scene_view.viewport.pos"; }
	};

	[node_group("Inputs")] class ViewportSize : CommonInput 
	{ 
		ViewportSize() { super(RHIShaderParameterType::Float2); } 
		string expr() const override { return "scene_view.viewport.size"; }
	};

	[node_group("Inputs")] class CameraLocation : CommonInput 
	{ 
		CameraLocation() { super(RHIShaderParameterType::Float3); } 
		string expr() const override { return "scene_view.camera.location"; }
	};

	[node_group("Inputs")] class CameraForward : CommonInput 
	{ 
		CameraForward() { super(RHIShaderParameterType::Float3); } 
		string expr() const override { return "scene_view.camera.forward"; }
	};

	[node_group("Inputs")] class CameraRight : CommonInput 
	{ 
		CameraRight() { super(RHIShaderParameterType::Float3); } 
		string expr() const override { return "scene_view.camera.rigth"; }
	};

	[node_group("Inputs")] class CameraUp : CommonInput 
	{ 
		CameraUp() { super(RHIShaderParameterType::Float3); } 
		string expr() const override { return "scene_view.camera.up"; }
	};

	[node_group("Inputs")] class CameraFov : CommonInput 
	{ 
		CameraFov() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.camera.fov"; }
	};

	[node_group("Inputs")] class CameraOrthoWidth : CommonInput 
	{ 
		CameraOrthoWidth() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.camera.ortho_width"; }
	};

	[node_group("Inputs")] class CameraOrthoHeight : CommonInput 
	{ 
		CameraOrthoHeight() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.camera.ortho_height"; }
	};

	[node_group("Inputs")] class CameraNear : CommonInput 
	{ 
		CameraNear() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.camera.near"; }
	};

	[node_group("Inputs")] class CameraFar : CommonInput 
	{ 
		CameraFar() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.camera.far"; }
	};

	[node_group("Inputs")] class CameraAspect : CommonInput 
	{ 
		CameraAspect() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.camera.aspect_ratio"; }
	};

	[node_group("Inputs")] class RenderTargetSize : CommonInput 
	{ 
		RenderTargetSize() { super(RHIShaderParameterType::Float2); } 
		string expr() const override { return "scene_view.render_target_size"; }
	};

	[node_group("Inputs")] class DepthRange : Node
	{ 
		DepthRange()
		{
			new_output("Min", RHIShaderParameterType::Float);
			new_output("Max", RHIShaderParameterType::Float);
		} 

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			if(pin.index() == 0)
			{
				return Expression(RHIShaderParameterType::Float, "scene_view.viewport.min_depth");
			}

			return Expression(RHIShaderParameterType::Float, "scene_view.viewport.max_depth");
		}
	};

	[node_group("Inputs")] class Gamma : CommonInput
	{
		Gamma() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.gamma"; }
	};

	[node_group("Inputs")] class Time : CommonInput
	{
		Time() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.time"; }
	};

	[node_group("Inputs")] class DeltaTime : CommonInput
	{
		DeltaTime() { super(RHIShaderParameterType::Float); } 
		string expr() const override { return "scene_view.delta_time"; }
	};

	[node_group("Inputs")] class ScreenCoordinates : Node
	{
		ScreenCoordinates() 
		{
			new_output("Viewport UV", RHIShaderParameterType::Float2); 
			new_output("Fragment Coord", RHIShaderParameterType::Float2);
		}
		
		Expression compile(OutputPin@ pin, Compiler@ compiler) override 
		{ 
			// if(!compiler.is_fragment_stage())
			// 	return Expression();

			// if(pin.index() == 0)
			// {
			// 	return Expression(RHIShaderParameterType::Float2, "(input.sv_position.xy / scene_view.viewport.size)");
			// }
			// else
			// {
			// 	return Expression(RHIShaderParameterType::Float2, "input.sv_position.xy");
			// }

			return Expression();
		}
	};

	[node_group("Inputs")] class UV0 : CommonInput
	{
		UV0() { super(RHIShaderParameterType::Float2); }
		string expr() const override { return "input.vertex.uv0"; }
	};

	[node_group("Inputs")] class UV1 : CommonInput
	{
		UV1() { super(RHIShaderParameterType::Float2); }
		string expr() const override { return "input.vertex.uv1"; }
	};

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
		RHIShaderParameterType in0_type() const override { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in0_default_type() const override { return RHIShaderParameterType::Float3; }
		RHIShaderParameterType in1_type() const override { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in1_default_type() const override { return RHIShaderParameterType::Float3; }
		string expr() const override { return "reflect(%0, %1)"; } 
		
		RHIShaderParameterType resolve_args(Expression& in0, Expression& in1) const override 
		{
			RHIShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			type = Expression::static_vector_clamp(type, 1, 3);
			
			in0 = in0.convert(type);
			in1 = in1.convert(type);
			return type;
		}
	};

	[node_group("Math")]
	class Refract : Ternary
	{ 
		RHIShaderParameterType in0_type() const override { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in0_default_type() const override { return RHIShaderParameterType::Float3; }
		RHIShaderParameterType in1_type() const override { return RHIShaderParameterType::META_Numeric; }
		RHIShaderParameterType in1_default_type() const override { return RHIShaderParameterType::Float3; }
		RHIShaderParameterType in2_type() const override { return RHIShaderParameterType::Float; }
		RHIShaderParameterType in2_default_type() const override { return RHIShaderParameterType::Float; }
		
		string expr() const override { return "refract(%0, %1, %2)"; } 
		
		RHIShaderParameterType resolve_args(Expression& in0, Expression& in1, Expression& in2) const override 
		{
			RHIShaderParameterType type = Expression::static_resolve(Expression::static_make_float(in0.type), Expression::static_make_float(in1.type));
			
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

		InputPin@ in0 = new_input("In", RHIShaderParameterType::META_Numeric);
		OutputPin@ out0 = new_output("Out", RHIShaderParameterType::META_Numeric);

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

		void render()
		{
			ImGui::Text("X"); ImGui::SameLine(); ImGui::Checkbox("##X", x);
			ImGui::Text("Y"); ImGui::SameLine(); ImGui::Checkbox("##Y", y);
			ImGui::Text("Z"); ImGui::SameLine(); ImGui::Checkbox("##Z", z);
			ImGui::Text("W"); ImGui::SameLine(); ImGui::Checkbox("##W", w);
		}
	};

	[node_group("Common")] class MakeVector2 : Node
	{
		InputPin@ m_in0 = new_input("A", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		InputPin@ m_in1 = new_input("B", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		OutputPin@ m_out0 = new_output("Out", RHIShaderParameterType::META_Vector);

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			Expression in1 = compiler.compile(m_in1);

			RHIShaderParameterType scalar0 = Expression::static_make_scalar(in0.type);
			RHIShaderParameterType scalar1 = Expression::static_make_scalar(in1.type);
			RHIShaderParameterType component_type = Expression::static_resolve(scalar0, scalar1);
			RHIShaderParameterType vector_type = Expression::static_make_vector(component_type, 2);

			string type_name = Expression::static_typename_of(vector_type);

			in0 = in0.convert(component_type);
			in1 = in1.convert(component_type);

			return Expression(vector_type, "%0(%1, %2)".format(type_name, in0.value, in1.value));
		}
	};

	[node_group("Common")] class MakeVector3 : Node
	{
		InputPin@ m_in0 = new_input("A", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		InputPin@ m_in1 = new_input("B", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		InputPin@ m_in2 = new_input("C", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		OutputPin@ m_out0 = new_output("Out", RHIShaderParameterType::META_Vector);

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			Expression in1 = compiler.compile(m_in1);
			Expression in2 = compiler.compile(m_in2);

			RHIShaderParameterType scalar0 = Expression::static_make_scalar(in0.type);
			RHIShaderParameterType scalar1 = Expression::static_make_scalar(in1.type);
			RHIShaderParameterType scalar2 = Expression::static_make_scalar(in2.type);
			
			RHIShaderParameterType component_type = Expression::static_resolve(scalar0, scalar1, scalar2);
			RHIShaderParameterType vector_type = Expression::static_make_vector(component_type, 3);
			string type_name = Expression::static_typename_of(vector_type);

			in0 = in0.convert(component_type);
			in1 = in1.convert(component_type);
			in2 = in2.convert(component_type);

			return Expression(vector_type, "%0(%1, %2, %3)".format(type_name, in0.value, in1.value, in2.value));
		}
	};

	[node_group("Common")] class MakeVector4 : Node
	{
		InputPin@ m_in0 = new_input("A", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		InputPin@ m_in1 = new_input("B", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		InputPin@ m_in2 = new_input("C", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		InputPin@ m_in3 = new_input("D", RHIShaderParameterType::META_Scalar, RHIShaderParameterType::Float);
		OutputPin@ m_out0 = new_output("Out", RHIShaderParameterType::META_Vector);

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			Expression in1 = compiler.compile(m_in1);
			Expression in2 = compiler.compile(m_in2);
			Expression in3 = compiler.compile(m_in3);

			RHIShaderParameterType scalar0 = Expression::static_make_scalar(in0.type);
			RHIShaderParameterType scalar1 = Expression::static_make_scalar(in1.type);
			RHIShaderParameterType scalar2 = Expression::static_make_scalar(in2.type);
			RHIShaderParameterType scalar3 = Expression::static_make_scalar(in3.type);
			
			RHIShaderParameterType component_type = Expression::static_resolve(scalar0, scalar1, scalar2, scalar3);
			RHIShaderParameterType vector_type = Expression::static_make_vector(component_type, 4);
			string type_name = Expression::static_typename_of(vector_type);

			in0 = in0.convert(component_type);
			in1 = in1.convert(component_type);
			in2 = in2.convert(component_type);
			in3 = in3.convert(component_type);

			return Expression(vector_type, "%0(%1, %2, %3, %4)".format(type_name, in0.value, in1.value, in2.value, in3.value));
		}
	};

	/////////////////////////////// CONDITIONS ///////////////////////////////

	[node_group("Conditions")]
	class If : Node
	{
		InputPin@ m_in0 = new_input("A", RHIShaderParameterType::META_Numeric, RHIShaderParameterType::Float);
		InputPin@ m_in1 = new_input("B", RHIShaderParameterType::META_Numeric, RHIShaderParameterType::Float);
		InputPin@ m_in2 = new_input("A > B", RHIShaderParameterType::META_Any);
		InputPin@ m_in3 = new_input("A == B", RHIShaderParameterType::META_Any);
		InputPin@ m_in4 = new_input("A < B", RHIShaderParameterType::META_Any);
		OutputPin@ m_out0 = new_output("Out", RHIShaderParameterType::META_Any);

		Expression compile2(Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0).vector_length();
			Expression in1 = compiler.compile(m_in1).vector_length();

			Expression in2 = compiler.compile(m_in2);
			Expression in4 = compiler.compile(m_in4);

			RHIShaderParameterType type = Expression::static_resolve(in2.type, in4.type);
			in2 = in2.convert(type);
			in4 = in4.convert(type);

			return Expression(type, "(%0 > %1 ? %2 : %3)".format(in0.value, in1.value, in2.value, in4.value));
		}

		Expression compile3(Compiler@ compiler)
		{
			Expression in0 = compiler.make_variable(compiler.compile(m_in0).vector_length());
			Expression in1 = compiler.make_variable(compiler.compile(m_in1).vector_length());

			Expression in2 = compiler.compile(m_in2);
			Expression in3 = compiler.compile(m_in3);
			Expression in4 = compiler.compile(m_in4);

			RHIShaderParameterType type = Expression::static_resolve(in2.type, in3.type, in4.type);

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
		InputPin@ m_in0 = new_input("Condition", RHIShaderParameterType::Bool, RHIShaderParameterType::Bool);
		InputPin@ m_in1 = new_input("True", RHIShaderParameterType::META_Any);
		InputPin@ m_in2 = new_input("False", RHIShaderParameterType::META_Any);
		OutputPin@ m_out0 = new_output("Out", RHIShaderParameterType::META_Any);

		Expression compile(OutputPin@ pin, Compiler@ compiler)
		{
			Expression in0 = compiler.compile(m_in0);
			
			Expression in1 = compiler.compile(m_in1);
			Expression in2 = compiler.compile(m_in2);

			const RHIShaderParameterType type = Expression::static_resolve(in1.type, in2.type);
			in1 = in1.convert(type);
			in2 = in2.convert(type);

			return Expression(type, "(%0 ? %1 : %2)".format(in0.value, in1.value, in2.value));
		}
	};

	/////////////////////////////// TEXTURES ///////////////////////////////
}