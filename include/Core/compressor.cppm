module;
#include <Core/definitions.hpp>

export module Trinex.Core.Compressor;

import Trinex.Core.Types;
import Trinex.Core.ETL.Array;

namespace Trinex::Compressor
{
	trinex_api void compress(const Array<u8>& src, Array<u8>& dst);
	trinex_api void decompress(const Array<u8>& src, Array<u8>& dst);
} // namespace Trinex::Compressor
