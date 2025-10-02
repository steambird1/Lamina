#ifndef RANGE_HPP
#define RANGE_HPP
#include "lamina.hpp"
#include "lstruct.hpp"
#include "cas.hpp"
#include <vector>

/**
 Range is a kind of set. Must be at least SymbolicExpr to use range
	(or no comparison is available).
*/

#define RANGE_TEST 0
#define PRE_FLAGS 0

#define PRE_FLAGS_PRE_SORT 1

namespace LaminaRange {
	
#if RANGE_TEST & 2
	using Value = int;
#endif 
	
#if RANGE_TEST
	using RangeValue = int;
#else
	using RangeValue = ::SymbolicExpr;
#endif
	
	// Return if a > b.
	bool is_greater(const RangeValue& a, const RangeValue& b) {
#if RANGE_TEST
		return a > b;
#else
		return a.to_double() > b.to_double();
#endif
	}
	
	// Return if a < b.
	bool is_less(const RangeValue& a, const RangeValue& b) {
#if RANGE_TEST
		return a < b;
#else
		return a.to_double() < b.to_double();
#endif
	}
	
	// Return if a == b.
	bool is_equal(const RangeValue& a, const RangeValue& b) {
#if RANGE_TEST
		return a == b;
#else
		return a.to_double() == b.to_double();
#endif
	}
	
	RangeValue from_lamina(const Value& src) {
#if RANGE_TEST & 4
		return src.as_number();
#else
		return src;
#endif
	}
	
	Value to_lamina(const RangeValue& src) {
#if RANGE_TEST & 4
		return Value(src);
#else
		return src;
#endif
	}
	
	RangeValue mini(const RangeValue& a, const RangeValue& b) {
		if (is_less(a, b)) return a;
		else return b;
	}
	
	RangeValue maxi(const RangeValue& a, const RangeValue& b) {
		if (is_greater(a, b)) return a;
		else return b;
	}
	
	struct Range {
		
		struct BasicRange {
			
			BasicRange() {}
			BasicRange(RangeValue l, RangeValue r, bool l_incl = true, bool r_incl = true) 
				: l(l), r(r), l_incl(l_incl), r_incl(r_incl) {}
			
			RangeValue l, r;
			bool l_incl = false, r_incl = false;	// Empty set by default for better operation
			char flag = 0;							// Flag = 1: merge failure
			
			// For sorter only
			bool operator < (const BasicRange &other) const {
				return is_less(this->l, other.l);
			}
			
			bool is_empty() const {
				if (is_equal(l, r)) {
					return (!l_incl) || (!r_incl);
				} else return is_greater(l, r);
			}
			
			bool in_range(const RangeValue& val) const {
				if (l_incl && is_equal(val, l)) return true;
				if (r_incl && is_equal(val, r)) return true;
				if (is_greater(val, l) && is_less(val, r)) return true;
				return false;
			}
			
			BasicRange intersect(const BasicRange &other) const {
				BasicRange result;
				if (is_greater(l, other.l)) {
					result.l = l;
					result.l_incl = l_incl;
				} else if (is_equal(l, other.l)) {
					result.l = l;
					result.l_incl = l_incl && other.l_incl;
				} else {
					result.l = other.l;
					result.l_incl = other.l_incl;
				}
				if (is_less(r, other.r)) {
					result.r = r;
					result.r_incl = r_incl;
				} else if (is_equal(r, other.r)) {
					result.r = r;
					result.r_incl = r_incl && other.r_incl;
				} else {
					result.r = other.r;
					result.r_incl = other.r_incl;
				}
				return result;
			}
			
			bool can_merge(const BasicRange &other) const {
				BasicRange result;
				if (is_greater(l, other.l)) {
					result.l = l;
					result.l_incl = l_incl;
				} else if (is_equal(l, other.l)) {
					result.l = l;
					result.l_incl = l_incl || other.l_incl;
				} else {
					result.l = other.l;
					result.l_incl = other.l_incl;
				}
				if (is_less(r, other.r)) {
					result.r = r;
					result.r_incl = r_incl;
				} else if (is_equal(r, other.r)) {
					result.r = r;
					result.r_incl = r_incl || other.r_incl;
				} else {
					result.r = other.r;
					result.r_incl = other.r_incl;
				}
				return !result.is_empty();
			}
			
			// Warning: before calling this, must check weak interaction
			BasicRange try_merge(const BasicRange &other) const {
				BasicRange result;
				if (is_greater(l, other.l)) {
					result.l = other.l;
					result.l_incl = other.l_incl;
				} else if (is_equal(l, other.l)) {
					result.l = l;
					result.l_incl = l_incl || other.l_incl;
				} else {
					result.l = l;
					result.l_incl = l_incl;
				}
				if (is_less(r, other.r)) {
					result.r = other.r;
					result.r_incl = other.r_incl;
				} else if (is_equal(r, other.r)) {
					result.r = r;
					result.r_incl = r_incl || other.r_incl;
				} else {
					result.r = r;
					result.r_incl = r_incl;
				}
				return result;
			}
			
			std::string to_string() const {
				std::string lclose, rclose;
				if (l_incl) lclose = "[";
				else lclose = "(";
				if (r_incl) rclose = "]";
				else rclose = ")";
#if RANGE_TEST
				return lclose + std::to_string(l) + "," + std::to_string(r) + rclose;
#else
				return lclose + l.to_string() + "," + r.to_string() + rclose;
#endif
			}
			
		};
		
		Range() {
			
		}

#if !(RANGE_TEST & 2)
		explicit Range(Value lamina_value) {
			if (!lamina_value.is_lstruct()) return;
			int sz = getaddr_raw(lamina_value, "size").as_number();
			for (int i = 1; i <= sz; i++) {
				Value l = getaddr_raw(lamina_value, "l_" + to_string(i));
				Value r = getaddr_raw(lamina_value, "r_" + to_string(i));
				Value l_incl = getaddr_raw(lamina_value, "l_inc_" + to_string(i));
				Value r_incl = getaddr_raw(lamina_value, "r_inc_" + to_string(i));
				
				segments.push_back(BasicRange(from_lamina(l), from_lamina(r), from_lamina(l_incl), from_lamina(r_incl)));
			}
			if (PRE_FLAGS & PRE_FLAGS_PRE_SORT) sort(segments.begin(), segments.end());
		}
#endif
	
		Value lamina() const {
#if RANGE_TEST & 2
			return Value(0);
#else
			auto ls = std::make_shared<lStruct>();
			ls->insert("size", segments.size());
			int it = 1;
			for (const auto i : segments) {
				ls->insert("l_" + to_string(it), to_lamina(i.l));
				ls->insert("r_" + to_string(it), to_lamina(i.r));
				ls->insert("l_inc_" + to_string(it), to_lamina(i.l_incl));
				ls->insert("r_inc_" + to_string(it), to_lamina(i.r_incl));
				it++;
			}
#endif	
		}
		
		bool in_range(const RangeValue& val) const {
			for (const auto i : segments) {
				if (is_greater(val, i.r)) continue;
				if (i.in_range(val)) return true;
				if (is_less(val, i.l)) break;
			}
			return false;
		}
		
		std::string to_string() const {
			if (!segments.size()) return std::string("<empty range>");
			std::string result;
			for (const auto i : segments) {
				result += i.to_string();
				result.push_back('u');	// Union symbol
			}
			result.pop_back();
			return result;
		}
		
		std::vector<BasicRange> segments;
	};
	
	Range intersect(const Range &a, const Range &b) {
		int p1 = 0, p2 = 0;
		Range result;
		while (p1 < a.segments.size() && p2 < b.segments.size()) {
			while (p1 < a.segments.size() && p2 < b.segments.size()
				&& is_less(b.segments[p2].r, a.segments[p1].l)) {
					if (p1 < a.segments.size() && p2 < b.segments.size()
						&& (is_less(a.segments[p1].r, b.segments[p2].r)
						|| (is_equal(a.segments[p1].r, b.segments[p2].r) && (!a.segments[p1].r_incl) && (b.segments[p2].r_incl))))
						p1++;
					else
						p2++;
				}
			if (!(p1 < a.segments.size() && p2 < b.segments.size())) break;
			Range::BasicRange pre_result = a.segments[p1].intersect(b.segments[p2]);
			if (!pre_result.is_empty()) result.segments.push_back(pre_result);
			// Jump to next one to evaluate intersection:
			if (p1 < a.segments.size() && p2 < b.segments.size()
				&& (is_less(a.segments[p1].r, b.segments[p2].r)
				|| (is_equal(a.segments[p1].r, b.segments[p2].r) && (!a.segments[p1].r_incl) && (b.segments[p2].r_incl))))
				p1++;
			else
				p2++;
		}
		return result;
	}
	
	Range join(const Range &a, const Range &b) {
		if (!a.segments.size()) return b;
		int p1 = 0, p2 = 0;
		Range result;
		Range::BasicRange current_seg = a.segments[0];
		while (p1 < a.segments.size() && p2 < b.segments.size()) {
			bool flag = true;
			while (flag) {
				flag = false;
				while (p1 < a.segments.size() && current_seg.can_merge(a.segments[p1])) {
					current_seg = current_seg.try_merge(a.segments[p1]);
					flag = true;
					p1++;
				}
				while (p2 < b.segments.size() && current_seg.can_merge(b.segments[p2])) {
					current_seg = current_seg.try_merge(b.segments[p2]);
					flag = true;
					p2++;
				}
			}
			result.segments.push_back(current_seg);
			current_seg = Range::BasicRange();
			if (p1 < a.segments.size()) {
				current_seg = a.segments[p1];
				p1++;
			}
			else if (p2 < b.segments.size()) {
				current_seg = b.segments[p2];
				p2++;
			}
		}
		if (!current_seg.is_empty()) result.segments.push_back(current_seg);
		return result;
	}
	
	// TODO: Reserve all conditions if comparision is not available.
	// No extension/inheritance is given because they are actually different.
	struct CASRange {
		// To be implemented ...
	};
	
	// TODO: Intersection and Merging of CAS Ranges.
	
	// Lamina-compatible interfaces:
	// (Infinity values are given here because inf are only used here)
	
	/**
	 * function: inf()
	*/
	Value lamina_inf(const std::vector<Value> &args) {
		return Value(1.0/0.0);
	}
	
	/**
	 * function: neginf()
	 */
	Value lamina_neginf(const std::vector<Value> &args) {
		return Value(-1.0/0.0);
	}
	
	/**
	 * function: range(l, r)
	 * generating [l, r] by default
	 */
	Value lamina_range(const std::vector<Value> &args) {
		Range tmp;
		tmp.segments.push_back(Range::BasicRange(mini(args[0], args[1]), maxi(args[0], args[1])));
		return tmp.lamina();
	}
	
	/**
	 * function: rangex(l, r, has_l, has_r)
	 * (TODO: Probably we need range literals for a math-processing language!)
	 */
	Value lamina_rangex(const std::vector<Value> &args) {
		Range tmp;
		tmp.segments.push_back(Range::BasicRange(mini(args[0], args[1]), maxi(args[0], args[1]), args[2].as_bool(), args[3].as_bool()));
		return tmp.lamina();
	}
	
	Value lamina_intersect(const std::vector<Value> &args) {
		if (!args.size()) return Range();
		Range result = Range(args[0]);
		for (size_t i = 1; i < args.size(); i++) result = intersect(result, Range(args[i]));
		return result.lamina();
	}
	
	Value lamina_join(const std::vector<Value> &args) {
		if (!args.size()) return Range();
		Range result = Range(args[0]);
		for (size_t i = 1; i < args.size(); i++) result = join(result, Range(args[i]));
		return result.lamina();
	}
	
	/**
	 * function: in_range(range, value)
	 */
	Value lamina_range_test(const std::vector<Value> &args) {
		return Value(Range(args[0]).in_range(args[1]));
	}
	
}

namespace Lamina {
	LAMINA_FUNC("inf", lamina_inf, 0);
	LAMINA_FUNC("neginf", lamina_neginf, 0);
	LAMINA_FUNC("range", lamina_range, 2);
	LAMINA_FUNC("rangex", lamina_rangex, 4);
	LAMINA_FUNC_WIT_ANY_ARGS("intersect", lamina_intersect);
	LAMINA_FUNC_WIT_ANY_ARGS("join", lamina_join);
	LAMINA_FUNC("in_range", lamina_range_test, 2);
}

#endif