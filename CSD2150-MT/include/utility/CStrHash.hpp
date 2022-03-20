/*!*****************************************************************************
 * @file    CStrHash.hpp
 * @author  Owen Huang Wensong	(w.huang@digipen.edu)
 * @date    27 FEB 2022
 * @brief   This file contains the implementations for string hashing.
 *          Compile Time Known Constant Expression versions are the main reason 
 *          for this custom implementation.
 *          This is modified based on my previous implementation authored on 
 *          04 OCT 2021, which is based on the STL's implementation at the time.
 * 
 *          This implementation requires at least C++17 for 
 *          the constexpr constants.
 *
 * Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef HEADER_GUARD_CSTRING_HASH
#define HEADER_GUARD_CSTRING_HASH

#include <cstring>			// for strlen
#include <string_view>	// for convenience

// FNV-1a constants, from STL, but added here to prevent changes in STL from 
// affecting the compile time known constant versions of the hash
namespace my_Fnv1a_NS
{
#if defined(_WIN64)
	constexpr size_t _FNV_offset_basis{ 14695981039346656037ULL };
	constexpr size_t _FNV_prime{ 1099511628211ULL };
#else // defined(_WIN64)
	constexpr size_t _FNV_offset_basis{ 2166136261U };
	constexpr size_t _FNV_prime{ 16777619U };
#endif // defined(_WIN64)

	// not true recursion, this is evaluated at compile time
	static constexpr size_t my_Fnv1a_CTConst(size_t _val, const char* const _First, const size_t _count)
	{
		if (!_count)return _val;
		_val ^= static_cast<size_t>(*_First);
		_val *= my_Fnv1a_NS::_FNV_prime;
		return my_Fnv1a_CTConst(_val, _First + 1, _count - 1);
	}
}

// *****************************************************************************
// *********************************** COMPILE TIME KNOWN CONSTEXPR VERSION ****

inline constexpr size_t operator""_literalHash(const char* _cstr, size_t _len)
{
	return my_Fnv1a_NS::my_Fnv1a_CTConst(my_Fnv1a_NS::_FNV_offset_basis, _cstr, _len);
}

template <size_t N>
inline constexpr size_t literalHash(const char(&_cstr)[N])
{
	static_assert(N > 0, "String literal must have a length greater than 0");
	return my_Fnv1a_NS::my_Fnv1a_CTConst(my_Fnv1a_NS::_FNV_offset_basis, _cstr, N - 1);
}

// *****************************************************************************
// ******************************************************* RUNTIME VERSIONS ****

inline size_t cstrHash(const char* _cstr, size_t _len = 0) noexcept
{
	if (!_len)_len = (_cstr == nullptr ? 0 : strlen(_cstr));
	size_t retval{ my_Fnv1a_NS::_FNV_offset_basis };
	for (size_t i{ 0 }; i < _len; ++i)
	{
		retval ^= static_cast<size_t>(_cstr[i]);
		retval *= my_Fnv1a_NS::_FNV_prime;
	}
	return retval;
}

inline size_t strHash(std::string_view const& inStrView) noexcept
{
	return cstrHash(inStrView.data(), inStrView.length());
}

// *****************************************************************************

#endif//HEADER_GUARD_CSTRING_HASH
