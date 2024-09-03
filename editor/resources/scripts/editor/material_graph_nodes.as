namespace Engine
{
	namespace VisualMaterialGraph
	{
		class SignaturedNode : Node
		{
			Expression make_expression(OutputPin@ pin, Ptr<NodeSignature::Signature> signature, const array<Expression>& args)
			{
				return Expression();
			}

			Expression compile(OutputPin@ pin, CompilerState@ state)
			{
				Ptr<const NodeSignature> node_signature = Ptr<const NodeSignature>(signature());

			 	int index = node_signature.get().find_signature_index(this);

			 	if(index == -1)
			 		return Expression();

			 	Ptr<NodeSignature::Signature> signature = Ptr<NodeSignature::Signature>(node_signature.get().signature(index));

			 	array<Expression> arguments;
			 	int args_count = signature.get().inputs_count();
			 	arguments.resize(args_count);

			 	for(int i = 0; i < args_count; ++i)
			 	{
			 		Expression expr = state.pin_source(input_pin(i));
			 		expr = state.expression_cast(expr, signature.get().input(i));

			 		if(!expr.is_valid())
			 			return Expression();

			 		arguments[i] = expr;
			 	}

			 	return make_expression(pin, signature, arguments);
			}
		};


		NodeSignature power_node_signature;

		[group("Engine::VisualMaterialGraphGroups::Math")]
		class Power : SignaturedNode 
		{	
			Power()
			{
				add_pin(FloatInputPin(this, "Base"));
				add_pin(FloatInputPin(this, "Exp", 2.f));
				add_pin(FloatOutputPinND(this, "Out"));
			}

			Expression make_expression(OutputPin@ pin, Ptr<NodeSignature::Signature> signature, const array<Expression>& args)
			{
				return Expression("pow(%0, %1)".format(args[0].code, args[1].code), signature.get().output(0));
			}

			const NodeSignature& signature() const
			{
				return power_node_signature;
			}
		};

		[initializer]
		void initializer()
		{
			power_node_signature.add_signature({PinType::Int, PinType::Int}, {PinType::Int});
			power_node_signature.add_signature({PinType::UInt, PinType::UInt}, {PinType::UInt});
			power_node_signature.add_signature({PinType::Float, PinType::Float}, {PinType::Float});

			power_node_signature.add_signature({PinType::IVec2, PinType::IVec2}, {PinType::IVec2});
			power_node_signature.add_signature({PinType::UVec2, PinType::UVec2}, {PinType::Vec2});
			power_node_signature.add_signature({PinType::Vec2, PinType::Vec2}, {PinType::Vec2});

			power_node_signature.add_signature({PinType::IVec3, PinType::IVec3}, {PinType::IVec3});
			power_node_signature.add_signature({PinType::UVec3, PinType::UVec3}, {PinType::UVec3});
			power_node_signature.add_signature({PinType::Vec3, PinType::Vec3}, {PinType::Vec3});
			power_node_signature.add_signature({PinType::Color3, PinType::Color3}, {PinType::Color3});

			power_node_signature.add_signature({PinType::IVec4, PinType::IVec4}, {PinType::IVec4});
			power_node_signature.add_signature({PinType::UVec4, PinType::UVec4}, {PinType::UVec4});
			power_node_signature.add_signature({PinType::Vec4, PinType::Vec4}, {PinType::Vec4});
			power_node_signature.add_signature({PinType::Color4, PinType::Color4}, {PinType::Color4});
		}
	}
}