namespace Engine
{
	namespace VisualMaterialGraph
	{
		NodeSignature power_node_signature;

		[group("Engine::VisualMaterialGraphGroups::Math")]
		class Pow : SignaturedNode
		{	
			Pow()
			{
				add_pin(FloatInputPin(this, "Base"));
				add_pin(FloatInputPin(this, "Exp", 2.f));
				add_pin(FloatOutputPinND(this, "Out"));
			}

			Expression make_expression(OutputPin@ pin, const NodeSignature::Signature& signature, const Vector<Expression>& args)
			{
				return Expression("pow(%0, %1)".format(args[0].code, args[1].code), signature.output(0));
			}

			const NodeSignature& signature() const
			{
				return power_node_signature;
			}
		};

		[initializer]
		void initializer()
		{
			power_node_signature.add_signature({PinType::Float, PinType::Float}, {PinType::Float});
			power_node_signature.add_signature({PinType::Vec2, PinType::Vec2}, {PinType::Vec2});
			power_node_signature.add_signature({PinType::Vec3, PinType::Vec3}, {PinType::Vec3});
			power_node_signature.add_signature({PinType::Color3, PinType::Color3}, {PinType::Color3});
			power_node_signature.add_signature({PinType::Vec4, PinType::Vec4}, {PinType::Vec4});
			power_node_signature.add_signature({PinType::Color4, PinType::Color4}, {PinType::Color4});
		}
	}
}