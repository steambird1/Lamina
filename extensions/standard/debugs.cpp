// Debug support, should be removed in release versions
#include "lamina.hpp"

Value __hash_symbolic(const std::vector<Value>& args) {
	auto hd = SymbolicExpr::HashData(args[0].as_symbolic());
	return Value(std::vector<Value>({Value(hd.k), Value(hd.ksqrt), Value(BigInt(hd.hash)), Value(hd.hash_obj), Value(hd.to_single_hash())}));
}

namespace Lamina {
	LAMINA_FUNC("__hash_symbolic", __hash_symbolic, 1);
}