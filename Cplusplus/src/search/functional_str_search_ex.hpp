#pragma once

#include <functional>
#pragma pack(push, _CRT_PACKING)
#pragma warning(push, _STL_WARNING_LEVEL)
#pragma warning(disable : _STL_DISABLED_WARNINGS)
_STL_DISABLE_CLANG_WARNINGS
#pragma push_macro("new")
#undef new

_STD_BEGIN

// KMP (Knuth-Morris-Pratt) searcher implementation
template <class _RanItPat, class _Pred_eq>
void _Build_kmp_failure_table(_Iter_diff_t<_RanItPat>* const _Failure, const _RanItPat _Pat_first,
    const _Iter_diff_t<_RanItPat> _Pat_size, _Pred_eq& _Eq) {
    // Builds KMP failure function table from a pattern [_Pat_first, _Pat_first + _Pat_size)
    using _Diff = _Iter_diff_t<_RanItPat>;

    if (_Pat_size == 0) {
        return;
    }

    _Failure[0] = 0;
    _Diff _Len = 0;

    for (_Diff _Idx = 1; _Idx < _Pat_size; ++_Idx) {
        while (_Len > 0 && !_Eq(_Pat_first[_Idx], _Pat_first[_Len])) {
            _Len = _Failure[_Len - 1];
        }

        if (_Eq(_Pat_first[_Idx], _Pat_first[_Len])) {
            ++_Len;
        }

        _Failure[_Idx] = _Len;
    }
}

template <class _RanItHaystack, class _RanItPat, class _Pred_eq>
pair<_RanItHaystack, _RanItHaystack> _Kmp_search(
    const _Iter_diff_t<_RanItPat>* _Failure, const _RanItPat _Pat_first, const _Iter_diff_t<_RanItPat> _Pat_size,
    _RanItHaystack _First, _RanItHaystack _Last, _Pred_eq& _Eq) {
    // KMP string search algorithm
    static_assert(is_same_v<_Iter_value_t<_RanItPat>, _Iter_value_t<_RanItHaystack>>,
        "kmp_searcher requires matching iterator value types");
    using _Diff = _Iter_diff_t<_RanItPat>;

    _STD _Adl_verify_range(_First, _Last);
    auto _UFirst = _STD _Get_unwrapped(_First);
    const auto _ULast = _STD _Get_unwrapped(_Last);

    if (_Pat_size == 0) {
        return { _First, _First };
    }

    const auto _UPat_first = _STD _Get_unwrapped_n(_Pat_first, _Pat_size);
    _Diff _Pat_idx = 0;

    while (_UFirst != _ULast) {
        if (_Eq(*_UFirst, _UPat_first[_Pat_idx])) {
            ++_Pat_idx;
            ++_UFirst;

            if (_Pat_idx == _Pat_size) {
                // Found match
                const auto _Match_start = _UFirst - _Pat_size;
                _STD _Seek_wrapped(_Last, _UFirst);
                _STD _Seek_wrapped(_First, _Match_start);
                return { _First, _Last };
            }
        }
        else {
            if (_Pat_idx != 0) {
                _Pat_idx = _Failure[_Pat_idx - 1];
            }
            else {
                ++_UFirst;
            }
        }
    }

    _STD _Seek_wrapped(_Last, _ULast);
    _STD _Seek_wrapped(_First, _ULast);
    return { _First, _Last };
}

_EXPORT_STD template <class _RanItPat, class _Pred_eq = equal_to<>>
class kmp_searcher {
public:
    kmp_searcher(const _RanItPat _First, const _RanItPat _Last, _Pred_eq _Eq = _Pred_eq())
        : _Pat_first(_First), _Pat_size(_STD _Get_unwrapped(_Last) - _STD _Get_unwrapped(_First)), _Eq(_STD move(_Eq)) {
        // preprocess a pattern for use with the KMP string search algorithm
        _STD _Adl_verify_range(_First, _Last);

        if (_Pat_size > 0) {
            _Failure_table = _STD make_unique<_Iter_diff_t<_RanItPat>[]>(static_cast<size_t>(_Pat_size));
            _STD _Build_kmp_failure_table(_Failure_table.get(), _STD _Get_unwrapped(_First), _Pat_size, _Eq);
        }
    }

    template <class _RanItHaystack>
    _NODISCARD pair<_RanItHaystack, _RanItHaystack> operator()(
        const _RanItHaystack _First, const _RanItHaystack _Last) const {
        // search for the preprocessed pattern in [_First, _Last)
        if (_Pat_size == 0) {
            return { _First, _First };
        }

        return _STD _Kmp_search(_Failure_table.get(), _Pat_first, _Pat_size, _First, _Last, _Eq);
    }

private:
    _RanItPat _Pat_first;
    _Iter_diff_t<_RanItPat> _Pat_size;
    _Pred_eq _Eq;
    unique_ptr<_Iter_diff_t<_RanItPat>[]> _Failure_table;
};

// Rabin-Karp searcher implementation
template <class _Value_t>
struct _Rabin_karp_hash {
    using _Hash_t = size_t;
    static constexpr _Hash_t _Base = 257;
    static constexpr _Hash_t _Mod = 1000000007; // Large prime number

    static _Hash_t _To_unsigned_byte(_Value_t _Char) {
        return static_cast<_Hash_t>(static_cast<unsigned char>(_Char));
    }

    static _Hash_t _Compute_hash(const _Value_t* _Data, size_t _Len) {
        _Hash_t _Hash = 0;
        for (size_t _Idx = 0; _Idx < _Len; ++_Idx) {
            _Hash = (_Hash * _Base + _To_unsigned_byte(_Data[_Idx])) % _Mod;
        }

        return _Hash;
    }

    static _Hash_t _Roll_hash(_Hash_t _Old_hash, _Value_t _Old_char, _Value_t _New_char, _Hash_t _Highest_power) {
        // Remove old character and add new character
        _Hash_t _New_hash = (_Old_hash + _Mod - (_To_unsigned_byte(_Old_char) * _Highest_power) % _Mod) % _Mod;
        _New_hash = (_New_hash * _Base + _To_unsigned_byte(_New_char)) % _Mod;
        return _New_hash;
    }

    static _Hash_t _Roll_hash(_Hash_t _Old_hash, const _Value_t* _Old_chars,
        const _Value_t* _New_chars, size_t _Char_len, _Hash_t _Highest_power) {
        // Remove the contribution of the first character in the previous window
        // Note: _Old_chars points to the start of the character sequence to remove
        _Hash_t _Char_value = _Compute_hash(_Old_chars, _Char_len);
        _Hash_t _New_hash = (_Old_hash + _Mod - (_Char_value * _Highest_power) % _Mod) % _Mod;

        // Add the contribution of the new character
        // Note: _New_chars points to the start of the character sequence to add
        return _Compute_hash(_New_chars, _Char_len);
    }

    static _Hash_t _Fast_power(_Hash_t base, _Hash_t exp, _Hash_t mod) {
        _Hash_t result = 1;
        base %= mod;

        while (exp > 0) {
            if (exp & 1) {
                result = (result * base) % mod;
            }
            base = (base * base) % mod;
            exp >>= 1;
        }

        return result;
    }

    static _Hash_t _Compute_highest_power(size_t _Pattern_length) {
        if (_Pattern_length == 0) return 1;
        return _Fast_power(_Base, _Pattern_length - 1, _Mod);
    }
    
};

template <class _RanItHaystack, class _RanItPat, class _Pred_eq>
pair<_RanItHaystack, _RanItHaystack> _Rabin_karp_search(
    const _RanItPat _Pat_first, const _Iter_diff_t<_RanItPat> _Pat_size,
    _RanItHaystack _First, _RanItHaystack _Last, _Pred_eq& _Eq) {
    // Rabin-Karp string search algorithm
    static_assert(is_same_v<_Iter_value_t<_RanItPat>, _Iter_value_t<_RanItHaystack>>,
        "rabin_karp_searcher requires matching iterator value types");
    using _Value_t = _Iter_value_t<_RanItPat>;
    using _Hash = _Rabin_karp_hash<_Value_t>;
    using _Diff = _Iter_diff_t<_RanItPat>;

    _STD _Adl_verify_range(_First, _Last);
    auto _UFirst = _STD _Get_unwrapped(_First);
    const auto _ULast = _STD _Get_unwrapped(_Last);

    if (_Pat_size == 0) {
        return { _First, _First };
    }

    const auto _UPat_first = _STD _Get_unwrapped_n(_Pat_first, _Pat_size);
    const auto _Haystack_size = _ULast - _UFirst;

    if (_Haystack_size < _Pat_size) {
        _STD _Seek_wrapped(_Last, _ULast);
        _STD _Seek_wrapped(_First, _ULast);
        return { _First, _Last };
    }

    // Compute pattern hash
    const auto _Pattern_hash = _Hash::_Compute_hash(_UPat_first, static_cast<size_t>(_Pat_size));

    // Compute power for rolling hash
    const auto _Highest_power = _Hash::_Compute_highest_power(static_cast<size_t>(_Pat_size));

    // Compute initial window hash
    auto _Text_hash = _Hash::_Compute_hash(_UFirst, static_cast<size_t>(_Pat_size));

    // Check first window
    if (_Text_hash == _Pattern_hash) {
        if (_STD equal(_UPat_first, _UPat_first + _Pat_size, _UFirst, _STD _Pass_fn(_Eq))) {
            _STD _Seek_wrapped(_Last, _UFirst + _Pat_size);
            _STD _Seek_wrapped(_First, _UFirst);
            return { _First, _Last };
        }
    }

    // Roll the hash and check remaining windows
    for (auto _Text_start = _UFirst + 1; _Text_start <= _ULast - _Pat_size; ++_Text_start) {
        _Text_hash = _Hash::_Roll_hash(_Text_hash, *(_Text_start - 1), *(_Text_start + _Pat_size - 1), _Highest_power);

        if (_Text_hash == _Pattern_hash) {
            if (_STD equal(_UPat_first, _UPat_first + _Pat_size, _Text_start, _STD _Pass_fn(_Eq))) {
                _STD _Seek_wrapped(_Last, _Text_start + _Pat_size);
                _STD _Seek_wrapped(_First, _Text_start);
                return { _First, _Last };
            }
        }
    }

    _STD _Seek_wrapped(_Last, _ULast);
    _STD _Seek_wrapped(_First, _ULast);
    return { _First, _Last };
}

_EXPORT_STD template <class _RanItPat, class _Pred_eq = equal_to<>>
class rabin_karp_searcher {
public:
    rabin_karp_searcher(const _RanItPat _First, const _RanItPat _Last, _Pred_eq _Eq = _Pred_eq())
        : _Pat_first(_First), _Pat_size(_STD _Get_unwrapped(_Last) - _STD _Get_unwrapped(_First)), _Eq(_STD move(_Eq)) {
        // preprocess a pattern for use with the Rabin-Karp string search algorithm
        _STD _Adl_verify_range(_First, _Last);
    }

    template <class _RanItHaystack>
    _NODISCARD pair<_RanItHaystack, _RanItHaystack> operator()(
        const _RanItHaystack _First, const _RanItHaystack _Last) const {
        // search for the preprocessed pattern in [_First, _Last)
        if (_Pat_size == 0) {
            return { _First, _First };
        }

        return _STD _Rabin_karp_search(_Pat_first, _Pat_size, _First, _Last, _Eq);
    }

private:
    _RanItPat _Pat_first;
    _Iter_diff_t<_RanItPat> _Pat_size;
    _Pred_eq _Eq;
};

// Sunday searcher implementation
template <class _Value_t>
struct _Sunday_bad_char_table {
    static constexpr size_t _Table_size = 256; // For char/unsigned char

    template <class _RanItPat>
    static void _Build_table(_Iter_diff_t<_RanItPat>* _Table, const _RanItPat _Pat_first,
        const _Iter_diff_t<_RanItPat> _Pat_size) {
        // Initialize all entries to pattern length + 1
        const auto _Default_shift = _Pat_size + 1;
        for (size_t _Idx = 0; _Idx < _Table_size; ++_Idx) {
            _Table[_Idx] = _Default_shift;
        }

        // Fill table with actual shift values
        for (_Iter_diff_t<_RanItPat> _Idx = 0; _Idx < _Pat_size; ++_Idx) {
            const auto _Char_val = static_cast<unsigned char>(_Pat_first[_Idx]);
            _Table[_Char_val] = _Pat_size - _Idx;
        }
    }
};

template <class _RanItHaystack, class _RanItPat, class _Pred_eq>
pair<_RanItHaystack, _RanItHaystack> _Sunday_search(
    const _RanItPat _Pat_first, const _Iter_diff_t<_RanItPat> _Pat_size,
    _RanItHaystack _First, _RanItHaystack _Last, _Pred_eq& _Eq) {
    // Sunday string search algorithm
    static_assert(is_same_v<_Iter_value_t<_RanItPat>, _Iter_value_t<_RanItHaystack>>,
        "sunday_searcher requires matching iterator value types");
    using _Value_t = _Iter_value_t<_RanItPat>;
    using _Diff = _Iter_diff_t<_RanItPat>;

    _STD _Adl_verify_range(_First, _Last);
    auto _UFirst = _STD _Get_unwrapped(_First);
    const auto _ULast = _STD _Get_unwrapped(_Last);

    if (_Pat_size == 0) {
        return { _First, _First };
    }

    const auto _UPat_first = _STD _Get_unwrapped_n(_Pat_first, _Pat_size);
    const auto _Haystack_size = _ULast - _UFirst;

    if (_Haystack_size < _Pat_size) {
        _STD _Seek_wrapped(_Last, _ULast);
        _STD _Seek_wrapped(_First, _ULast);
        return { _First, _Last };
    }

    // Build bad character table
    _Diff _Bad_char_table[_Sunday_bad_char_table<_Value_t>::_Table_size];
    _Sunday_bad_char_table<_Value_t>::_Build_table(_Bad_char_table, _UPat_first, _Pat_size);

    auto _Text_pos = _UFirst;

    while (_Text_pos <= _ULast - _Pat_size) {
        // Compare pattern with current window
        _Diff _Match_pos = 0;
        while (_Match_pos < _Pat_size && _STD _Pass_fn(_Eq)(_UPat_first[_Match_pos], _Text_pos[_Match_pos])) {
            ++_Match_pos;
        }

        if (_Match_pos == _Pat_size) {
            // Found a match
            _STD _Seek_wrapped(_Last, _Text_pos + _Pat_size);
            _STD _Seek_wrapped(_First, _Text_pos);
            return { _First, _Last };
        }

        // Calculate shift using Sunday algorithm
        const auto _Next_pos = _Text_pos + _Pat_size;
        if (_Next_pos < _ULast) {
            const auto _Next_char = static_cast<unsigned char>(*_Next_pos);
            _Text_pos += _Bad_char_table[_Next_char];
        }
        else {
            break; // No more characters to examine
        }
    }

    _STD _Seek_wrapped(_Last, _ULast);
    _STD _Seek_wrapped(_First, _ULast);
    return { _First, _Last };
}

_EXPORT_STD template <class _RanItPat, class _Pred_eq = equal_to<>>
class sunday_searcher {
public:
    sunday_searcher(const _RanItPat _First, const _RanItPat _Last, _Pred_eq _Eq = _Pred_eq())
        : _Pat_first(_First), _Pat_size(_STD _Get_unwrapped(_Last) - _STD _Get_unwrapped(_First)), _Eq(_STD move(_Eq)) {
        // preprocess a pattern for use with the Sunday string search algorithm
        _STD _Adl_verify_range(_First, _Last);
    }

    template <class _RanItHaystack>
    _NODISCARD pair<_RanItHaystack, _RanItHaystack> operator()(
        const _RanItHaystack _First, const _RanItHaystack _Last) const {
        // search for the preprocessed pattern in [_First, _Last)
        if (_Pat_size == 0) {
            return { _First, _First };
        }

        return _STD _Sunday_search(_Pat_first, _Pat_size, _First, _Last, _Eq);
    }

private:
    _RanItPat _Pat_first;
    _Iter_diff_t<_RanItPat> _Pat_size;
    _Pred_eq _Eq;
};


_STD_END

#pragma pop_macro("new")
_STL_RESTORE_CLANG_WARNINGS
#pragma warning(pop)
#pragma pack(pop)