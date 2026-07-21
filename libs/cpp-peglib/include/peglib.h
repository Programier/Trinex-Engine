//
//  peglib.h
//
//  Copyright (c) 2022 Yuji Hirose. All rights reserved.
//  MIT License
//

#pragma once

#define CPPPEGLIB_VERSION "1.15.0"
#define CPPPEGLIB_VERSION_NUM "0x010f00"

/*
 * Configuration
 */

#ifndef CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT
#define CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT 32
#endif

#include <any>
#include <bitset>
#include <cassert>
#if __has_include(<charconv>)
#include <charconv>
#endif
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#if !defined(__cplusplus) || __cplusplus < 201703L
#error "Requires complete C++17 support"
#endif

#if defined(_WIN32) && defined(PEGLIB_SHARED)
#if defined(PEGLIB_EXPORTS)
#define PEGLIB_API __declspec(dllexport)
#else
#define PEGLIB_API __declspec(dllimport)
#endif
#else
#define PEGLIB_API
#endif

namespace peg {

struct GrammarBlob;
namespace detail {
struct GrammarBlobImpl;
}

/*-----------------------------------------------------------------------------
 *  scope_exit
 *---------------------------------------------------------------------------*/

// This is based on
// "http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189".

template <typename EF> struct scope_exit {
  explicit scope_exit(EF &&f)
      : exit_function(std::move(f)), execute_on_destruction{true} {}

  scope_exit(scope_exit &&rhs)
      : exit_function(std::move(rhs.exit_function)),
        execute_on_destruction{rhs.execute_on_destruction} {
    rhs.release();
  }

  ~scope_exit() {
    if (execute_on_destruction) { this->exit_function(); }
  }

  void release() { this->execute_on_destruction = false; }

private:
  scope_exit(const scope_exit &) = delete;
  void operator=(const scope_exit &) = delete;
  scope_exit &operator=(scope_exit &&) = delete;

  EF exit_function;
  bool execute_on_destruction;
};

/*-----------------------------------------------------------------------------
 *  UTF8 functions
 *---------------------------------------------------------------------------*/

PEGLIB_API size_t codepoint_length(const char *s8, size_t l);

PEGLIB_API size_t codepoint_count(const char *s8, size_t l);

PEGLIB_API size_t encode_codepoint(char32_t cp, char *buff);

PEGLIB_API std::string encode_codepoint(char32_t cp);

PEGLIB_API bool decode_codepoint(const char *s8, size_t l, size_t &bytes,
                             char32_t &cp);

PEGLIB_API size_t decode_codepoint(const char *s8, size_t l, char32_t &cp);

PEGLIB_API char32_t decode_codepoint(const char *s8, size_t l);

PEGLIB_API std::u32string decode(const char *s8, size_t l);

template <typename T> const char *u8(const T *s) {
  return reinterpret_cast<const char *>(s);
}

/*-----------------------------------------------------------------------------
 *  escape_characters
 *---------------------------------------------------------------------------*/

PEGLIB_API std::string escape_characters(const char *s, size_t n);

PEGLIB_API std::string escape_characters(std::string_view sv);

/*-----------------------------------------------------------------------------
 *  resolve_escape_sequence
 *---------------------------------------------------------------------------*/

PEGLIB_API bool is_hex(char c, int &v);

PEGLIB_API bool is_digit(char c, int &v);

PEGLIB_API std::pair<int, size_t> parse_hex_number(const char *s, size_t n,
                                               size_t i);

PEGLIB_API std::pair<int, size_t> parse_octal_number(const char *s, size_t n,
                                                 size_t i);

PEGLIB_API std::string resolve_escape_sequence(const char *s, size_t n);

/*
 * Predefined character classes (ASCII semantics)
 */
PEGLIB_API const std::vector<std::pair<char32_t, char32_t>> *
predefined_character_class(std::string_view name);

// Ranges must be sorted and non-overlapping.
PEGLIB_API std::vector<std::pair<char32_t, char32_t>> complement_character_ranges(
    const std::vector<std::pair<char32_t, char32_t>> &ranges);

/*-----------------------------------------------------------------------------
 *  token_to_number_ - This function should be removed eventually
 *---------------------------------------------------------------------------*/

template <typename T> T token_to_number_(std::string_view sv) {
  T n = 0;
#if __has_include(<charconv>)
  if constexpr (!std::is_floating_point<T>::value) {
    std::from_chars(sv.data(), sv.data() + sv.size(), n);
#else
  if constexpr (false) {
#endif
  } else {
    auto s = std::string(sv);
    std::istringstream ss(s);
    ss >> n;
  }
  return n;
}

PEGLIB_API std::string to_lower(std::string s);

/*-----------------------------------------------------------------------------
 *  Trie
 *---------------------------------------------------------------------------*/

class PEGLIB_API Trie {
public:
  Trie(const std::vector<std::string> &items, bool ignore_case);

  size_t match(const char *text, size_t text_len, size_t &id) const;

  size_t size() const;
  size_t items_count() const;

  friend struct ComputeFirstSet;
  friend struct GrammarBlob;
  friend struct detail::GrammarBlobImpl;

private:
  struct Info {
    bool done;
    bool match;
    size_t id;
  };

  // TODO: Use unordered_map when heterogeneous lookup is supported in C++20
  // std::unordered_map<std::string, Info> dic_;
  std::map<std::string, Info, std::less<>> dic_;

  bool ignore_case_;
  size_t items_count_;
  size_t max_len_ = 0;
};

/*-----------------------------------------------------------------------------
 *  PEG
 *---------------------------------------------------------------------------*/

/*
 * Line information utility function
 */
PEGLIB_API std::pair<size_t, size_t> line_info(const char *start, const char *cur);

/*
 * String tag
 */
inline constexpr unsigned int str2tag_core(const char *s, size_t l,
                                           unsigned int h) {
  return (l == 0) ? h
                  : str2tag_core(s + 1, l - 1,
                                 (h * 33) ^ static_cast<unsigned char>(*s));
}

inline constexpr unsigned int str2tag(std::string_view sv) {
  return str2tag_core(sv.data(), sv.size(), 0);
}

namespace udl {

inline constexpr unsigned int operator""_(const char *s, size_t l) {
  return str2tag_core(s, l, 0);
}

} // namespace udl

/*
 * Semantic values
 */
class Context;

struct PEGLIB_API SemanticValues : protected std::vector<std::any> {
  SemanticValues() = default;
  SemanticValues(Context *c);

  // Input text
  const char *path = nullptr;
  const char *ss = nullptr;

  // Matched string
  std::string_view sv() const;

  // Definition name
  const std::string &name() const;

  std::vector<unsigned int> tags;

  // Line number and column at which the matched string is
  std::pair<size_t, size_t> line_info() const;

  // Choice count
  size_t choice_count() const;

  // Choice number (0 based index)
  size_t choice() const;

  // Tokens
  std::vector<std::string_view> tokens;

  std::string_view token(size_t id = 0) const;

  // Token conversion
  std::string token_to_string(size_t id = 0) const;

  template <typename T> T token_to_number() const {
    return token_to_number_<T>(token());
  }

  // Transform the semantic value vector to another vector
  template <typename T>
  std::vector<T> transform(size_t beg = 0,
                           size_t end = static_cast<size_t>(-1)) const {
    std::vector<T> r;
    end = end < size() ? end : size();
    for (size_t i = beg; i < end; i++) {
      r.emplace_back(std::any_cast<T>((*this)[i]));
    }
    return r;
  }

  using std::vector<std::any>::iterator;
  using std::vector<std::any>::const_iterator;
  using std::vector<std::any>::size;
  using std::vector<std::any>::empty;
  using std::vector<std::any>::assign;
  using std::vector<std::any>::begin;
  using std::vector<std::any>::end;
  using std::vector<std::any>::rbegin;
  using std::vector<std::any>::rend;
  using std::vector<std::any>::operator[];
  using std::vector<std::any>::at;
  using std::vector<std::any>::resize;
  using std::vector<std::any>::front;
  using std::vector<std::any>::back;
  using std::vector<std::any>::push_back;
  using std::vector<std::any>::pop_back;
  using std::vector<std::any>::insert;
  using std::vector<std::any>::erase;
  using std::vector<std::any>::clear;
  using std::vector<std::any>::swap;
  using std::vector<std::any>::emplace;
  using std::vector<std::any>::emplace_back;

private:
  friend class Context;
  friend class Dictionary;
  friend class Sequence;
  friend class PrioritizedChoice;
  friend class Repetition;
  friend class Holder;
  friend class PrecedenceClimbing;

  Context *c_ = nullptr;
  std::string_view sv_;
  size_t choice_count_ = 0;
  size_t choice_ = 0;
  std::string name_;
};

/*
 * Semantic action
 */
template <typename F, typename... Args> std::any call(F fn, Args &&...args) {
  using R = decltype(fn(std::forward<Args>(args)...));
  if constexpr (std::is_void<R>::value) {
    fn(std::forward<Args>(args)...);
    return std::any();
  } else if constexpr (std::is_same<typename std::remove_cv<R>::type,
                                    std::any>::value) {
    return fn(std::forward<Args>(args)...);
  } else {
    return std::any(fn(std::forward<Args>(args)...));
  }
}

template <typename T>
struct argument_count : argument_count<decltype(&T::operator())> {};
template <typename R, typename... Args>
struct argument_count<R (*)(Args...)>
    : std::integral_constant<unsigned, sizeof...(Args)> {};
template <typename R, typename C, typename... Args>
struct argument_count<R (C::*)(Args...)>
    : std::integral_constant<unsigned, sizeof...(Args)> {};
template <typename R, typename C, typename... Args>
struct argument_count<R (C::*)(Args...) const>
    : std::integral_constant<unsigned, sizeof...(Args)> {};

class Action {
public:
  Action() = default;
  Action(Action &&rhs) = default;
  template <typename F> Action(F fn) : fn_(make_adaptor(fn)) {}
  template <typename F> void operator=(F fn) { fn_ = make_adaptor(fn); }
  Action &operator=(const Action &rhs) = default;

  PEGLIB_API operator bool() const;

  PEGLIB_API std::any operator()(SemanticValues &vs, std::any &dt,
                                 const std::any &predicate_data) const;

private:
  using Fty = std::function<std::any(SemanticValues &vs, std::any &dt,
                                     const std::any &predicate_data)>;

  template <typename F> Fty make_adaptor(F fn) {
    if constexpr (argument_count<F>::value == 1) {
      return [fn](auto &vs, auto & /*dt*/, const auto & /*predicate_data*/) {
        return call(fn, vs);
      };
    } else if constexpr (argument_count<F>::value == 2) {
      return [fn](auto &vs, auto &dt, const auto & /*predicate_data*/) {
        return call(fn, vs, dt);
      };
    } else {
      return [fn](auto &vs, auto &dt, const auto &predicate_data) {
        return call(fn, vs, dt, predicate_data);
      };
    }
  }

  Fty fn_;
};

class Predicate {
public:
  Predicate() = default;
  Predicate(Predicate &&rhs) = default;
  template <typename F> Predicate(F fn) : fn_(make_adaptor(fn)) {}
  template <typename F> void operator=(F fn) { fn_ = make_adaptor(fn); }
  Predicate &operator=(const Predicate &rhs) = default;

  PEGLIB_API operator bool() const;

  PEGLIB_API bool operator()(const SemanticValues &vs, const std::any &dt,
                             std::string &msg, std::any &predicate_data) const;

private:
  using Fty = std::function<bool(const SemanticValues &vs, const std::any &dt,
                                 std::string &msg, std::any &predicate_data)>;

  template <typename F> Fty make_adaptor(F fn) {
    if constexpr (argument_count<F>::value == 3) {
      return [fn](const auto &vs, const auto &dt, auto &msg,
                  auto & /*predicate_data*/) { return fn(vs, dt, msg); };
    } else {
      return [fn](const auto &vs, const auto &dt, auto &msg,
                  auto &predicate_data) {
        return fn(vs, dt, msg, predicate_data);
      };
    }
  }

  Fty fn_;
};

/*
 * Parse result helper
 */
PEGLIB_API bool success(size_t len);

PEGLIB_API bool fail(size_t len);

/*
 * Log
 */
using Log = std::function<void(size_t line, size_t col, const std::string &msg,
                               const std::string &rule)>;

/*
 * ErrorReport - structured error information passed to an ErrorReporter.
 * Unlike Log, nothing is flattened into a display string, so applications
 * can map errors to their own error types, localize messages, or feed
 * diagnostics to IDEs.
 */
struct ErrorReport {
  size_t line = 0;              // 1-based
  size_t col = 1;               // 1-based
  size_t position = 0;          // byte offset in the input
  std::string unexpected_token; // heuristic token at the error position
  std::vector<std::string> expected_literals;
  std::vector<std::string> expected_rules; // rules starting with '_' excluded
  std::string message; // custom error_message if any (placeholders resolved)
  std::string label;   // rule name or recovery label the error belongs to
};

using ErrorReporter = std::function<void(const ErrorReport &report)>;

/*
 * ErrorInfo
 */
class Definition;

struct PEGLIB_API ErrorInfo {
  const char *error_pos = nullptr;
  std::vector<std::pair<const char *, const Definition *>> expected_tokens;
  const char *message_pos = nullptr;
  std::string message;
  std::string label;
  const char *last_output_pos = nullptr;
  bool keep_previous_token = false;

  void clear();

  void add(const char *error_literal, const Definition *error_rule);

  void output_log(const Log &log, const char *s, size_t n);
  void output_log(const Log &log, const ErrorReporter &reporter, const char *s,
                  size_t n);

private:
  int cast_char(char c) const;

  std::string heuristic_error_token(const char *s, size_t n,
                                    const char *pos) const;

  std::string replace_all(std::string str, const std::string &from,
                          const std::string &to) const;
};

/*
 * Context
 */
class Ope;

using TracerEnter = std::function<void(
    const Ope &name, const char *s, size_t n, const SemanticValues &vs,
    const Context &c, const std::any &dt, std::any &trace_data)>;

using TracerLeave = std::function<void(
    const Ope &ope, const char *s, size_t n, const SemanticValues &vs,
    const Context &c, const std::any &dt, size_t, std::any &trace_data)>;

using TracerStartOrEnd = std::function<void(std::any &trace_data)>;

// Packrat memoization table: open-addressing hash map keyed by the fused
// (position * rule count + rule id) index. The insert-heavy access pattern
// makes node-based containers a bottleneck, so keys and lengths live in one
// flat array of 16-byte POD slots probed linearly; semantic values go into a
// parallel array that is never allocated when no cached result carries a
// value. Erased slots become tombstones (erase only happens during
// left-recursion cache invalidation).
class PEGLIB_API PackratCache {
public:
  explicit PackratCache(size_t expected_entries);

  bool find(size_t key, size_t &len, std::any &val) const;

  void insert_or_assign(size_t key, size_t len, const std::any &val);

  void erase(size_t key);

private:
  static constexpr size_t kEmpty = static_cast<size_t>(-1);
  static constexpr size_t kTombstone = static_cast<size_t>(-2);

  struct Slot {
    size_t key = kEmpty;
    size_t len = 0;
  };

  static size_t mix(size_t key);

  void grow();

  size_t initial_capacity_ = 1024;
  std::vector<Slot> slots_;
  std::vector<std::any> vals_;
  size_t used_ = 0; // occupied + tombstone slots
};

class PEGLIB_API Context {
public:
  const char *path;
  const char *s;
  const size_t l;

  ErrorInfo error_info;
  bool recovered = false;

  std::vector<std::unique_ptr<SemanticValues>> value_stack;
  size_t value_stack_size = 0;

  std::vector<Definition *> rule_stack;
  std::vector<std::vector<std::shared_ptr<Ope>>> args_stack;

  size_t in_token_boundary_count = 0;

  std::shared_ptr<Ope> whitespaceOpe;
  bool in_whitespace = false;

  std::shared_ptr<Ope> wordOpe;

  std::vector<std::pair<std::string_view, std::string>> capture_entries;

  std::vector<bool> cut_stack;

  const size_t def_count;
  const bool enablePackratParsing;
  const std::vector<int32_t> *packrat_index; // def_id -> cache slot or -1
  size_t packrat_cached_count;               // number of memoized rules
  std::vector<bool> cache_registered;
  std::vector<bool> cache_success;
  // Innermost active start position per rule; re-entry guard for rules that
  // are not memoized (replaces the per-position bitvector for them).
  std::vector<const char *> active_pos;

  PackratCache cache_values;

  // Left recursion support
  struct LRMemo {
    size_t len = static_cast<size_t>(-1);
    std::any val;
  };
  std::map<std::pair<const Definition *, const char *>, LRMemo> lr_memo;

  // Rules whose lr_memo was hit during the current parse scope.
  // Used to track LR cycle membership.
  std::set<const Definition *> lr_refs_hit;

  // Rules currently in their seeding/growing phase at a given position.
  // Protected from having their lr_memo erased by inner growers.
  std::set<std::pair<const Definition *, const char *>> lr_active_seeds;

  // Map a def_id to its slot in the cache tables, or -1 for guard-only
  // rules (not memoized).
  int32_t cache_slot(size_t def_id) const;

  void clear_packrat_cache(const char *pos, size_t def_id);

  void write_packrat_cache(const char *pos, size_t def_id, size_t len,
                           const std::any &val);

  TracerEnter tracer_enter;
  TracerLeave tracer_leave;
  const bool has_tracer;
  std::any trace_data;
  const bool verbose_trace;

  // Byte-wise tolower frozen at parse start, so case-insensitive matching
  // avoids a locale-sensitive libc call per input byte.
  unsigned char tolower_table[256];

  Log log;
  ErrorReporter error_reporter;

  Context(const char *path, const char *s, size_t l, size_t def_count,
          std::shared_ptr<Ope> whitespaceOpe, std::shared_ptr<Ope> wordOpe,
          bool enablePackratParsing, TracerEnter tracer_enter,
          TracerLeave tracer_leave, std::any trace_data, bool verbose_trace,
          Log log, ErrorReporter error_reporter = nullptr,
          const std::vector<int32_t> *packrat_index = nullptr,
          size_t packrat_cached_count = 0);

  ~Context();

  Context(const Context &) = delete;
  Context(Context &&) = delete;
  Context operator=(const Context &) = delete;

  // Per-rule packrat stats (populated when packrat_stats is non-null)
  struct PackratStats {
    size_t hits = 0;
    size_t misses = 0;
  };
  std::vector<PackratStats> *packrat_stats = nullptr;

  template <typename T>
  void packrat(const char *a_s, size_t def_id, size_t &len, std::any &val,
               T fn) {
    if (!enablePackratParsing) {
      fn(val);
      return;
    }

    auto slot = cache_slot(def_id);
    if (slot < 0) {
      // Guard-only rule: no memoization. Recursion at the same position is
      // caught by the per-rule active-position guard.
      if (active_pos[def_id] == a_s) {
        if (packrat_stats && def_id < packrat_stats->size()) {
          (*packrat_stats)[def_id].hits++;
        }
        len = static_cast<size_t>(-1);
        return;
      }
      if (packrat_stats && def_id < packrat_stats->size()) {
        (*packrat_stats)[def_id].misses++;
      }
      auto save = active_pos[def_id];
      active_pos[def_id] = a_s;
      fn(val);
      active_pos[def_id] = save;
      return;
    }

    auto col = a_s - s;
    auto idx = packrat_cached_count * static_cast<size_t>(col) +
               static_cast<size_t>(slot);

    if (cache_registered[idx]) {
      if (packrat_stats && def_id < packrat_stats->size()) {
        (*packrat_stats)[def_id].hits++;
      }
      if (cache_success[idx]) {
        if (!cache_values.find(idx, len, val)) {
          len = 0;
          val.reset();
        }
        return;
      } else {
        len = static_cast<size_t>(-1);
        return;
      }
    } else {
      // Pre-register as failure (re-entry guard + failure memoization)
      cache_registered[idx] = true;
      cache_success[idx] = false;

      if (packrat_stats && def_id < packrat_stats->size()) {
        (*packrat_stats)[def_id].misses++;
      }

      fn(val);

      if (success(len)) { write_packrat_cache(a_s, def_id, len, val); }
      return;
    }
  }

  // Semantic values
  SemanticValues &push_semantic_values_scope();

  void pop_semantic_values_scope();

  // Arguments
  void push_args(std::vector<std::shared_ptr<Ope>> &&args);

  void pop_args();

  const std::vector<std::shared_ptr<Ope>> &top_args() const;

  // Snapshot/Rollback
  struct Snapshot {
    size_t sv_size;
    size_t sv_tags_size;
    size_t sv_tokens_size;
    std::string_view sv_sv;
    size_t choice_count;
    size_t choice;
    size_t capture_size;
  };

  Snapshot snapshot(const SemanticValues &vs) const;

  void rollback(SemanticValues &vs, const Snapshot &snap);

  // Skip trailing whitespace with trace suppression.
  // Returns whitespace length, or -1 on failure.
  // No-op (returns 0) if inside a token boundary or no whitespaceOpe.
  size_t skip_whitespace(const char *a_s, size_t n, SemanticValues &vs,
                         std::any &dt);

  // Error
  void set_error_pos(const char *a_s, const char *literal = nullptr);

  // Trace
  void trace_enter(const Ope &ope, const char *a_s, size_t n,
                   const SemanticValues &vs, std::any &dt);
  void trace_leave(const Ope &ope, const char *a_s, size_t n,
                   const SemanticValues &vs, std::any &dt, size_t len);
  bool is_traceable(const Ope &ope) const;

  // Line info
  std::pair<size_t, size_t> line_info(const char *cur) const;

  size_t next_trace_id = 0;
  std::vector<size_t> trace_ids;
  bool ignore_trace_state = false;
  mutable std::once_flag source_line_index_init_;
  mutable std::vector<size_t> source_line_index;
};

/*
 * Parser operators
 */
class PEGLIB_API Ope {
public:
  struct PEGLIB_API Visitor;

  virtual ~Ope() = default;
  size_t parse(const char *s, size_t n, SemanticValues &vs, Context &c,
               std::any &dt) const;
  virtual size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                            Context &c, std::any &dt) const = 0;
  virtual void accept(Visitor &v) = 0;

  bool is_token_boundary = false;
  bool is_choice_like = false;
};

struct KeywordGuardData;


class PEGLIB_API Sequence : public Ope {
public:
  template <typename... Args>
  Sequence(const Args &...args)
      : opes_{static_cast<std::shared_ptr<Ope>>(args)...} {}
  Sequence(const std::vector<std::shared_ptr<Ope>> &opes);
  Sequence(std::vector<std::shared_ptr<Ope>> &&opes);
  ~Sequence();

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::vector<std::shared_ptr<Ope>> opes_;

private:
  friend struct SetupFirstSets;
  std::unique_ptr<KeywordGuardData> kw_guard_;

  // Returns parse result, or nullopt to fall through to normal path
  std::optional<size_t> parse_keyword_guarded(const char *s, size_t n,
                                              SemanticValues &vs, Context &c,
                                              std::any &dt) const;
};

struct PEGLIB_API FirstSet {
  // First-Set: set of possible first bytes for an expression.
  // Used by PrioritizedChoice to skip alternatives that cannot match.
  std::bitset<256> chars;    // byte values that can appear as the first byte
  bool can_be_empty = false; // true if the expression can match empty string
  bool any_char = false;     // true if any character can appear (cannot filter)
  const char *first_literal = nullptr; // first literal for error reporting
  const Definition *first_rule =
      nullptr; // first token rule for error reporting

  void merge(const FirstSet &other);
};

class PEGLIB_API PrioritizedChoice : public Ope {
public:
  template <typename... Args>
  PrioritizedChoice(bool for_label, const Args &...args)
      : opes_{static_cast<std::shared_ptr<Ope>>(args)...},
        for_label_(for_label) {
    is_choice_like = true;
  }
  PrioritizedChoice(const std::vector<std::shared_ptr<Ope>> &opes);
  PrioritizedChoice(std::vector<std::shared_ptr<Ope>> &&opes);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  size_t size() const;

  std::vector<std::shared_ptr<Ope>> opes_;
  bool for_label_ = false;
  std::vector<FirstSet> first_sets_;
};

class PEGLIB_API Repetition : public Ope {
public:
  Repetition(const std::shared_ptr<Ope> &ope, size_t min, size_t max);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  bool is_zom() const;

  static std::shared_ptr<Repetition> zom(const std::shared_ptr<Ope> &ope);

  static std::shared_ptr<Repetition> oom(const std::shared_ptr<Ope> &ope);

  static std::shared_ptr<Repetition> opt(const std::shared_ptr<Ope> &ope);

  std::shared_ptr<Ope> ope_;
  size_t min_;
  size_t max_;
  const std::bitset<256> *span_bitset_ =
      nullptr; // non-owning, set by SetupFirstSets
};

class PEGLIB_API AndPredicate : public Ope {
public:
  AndPredicate(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class PEGLIB_API NotPredicate : public Ope {
public:
  NotPredicate(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class PEGLIB_API Dictionary : public Ope, public std::enable_shared_from_this<Dictionary> {
public:
  Dictionary(const std::vector<std::string> &v, bool ignore_case);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  Trie trie_;
};

class PEGLIB_API LiteralString : public Ope,
                      public std::enable_shared_from_this<LiteralString> {
public:
  LiteralString(std::string &&s, bool ignore_case);

  LiteralString(const std::string &s, bool ignore_case);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::string lit_;
  bool ignore_case_;
  std::string lower_lit_; // pre-computed for ignore_case
  mutable std::once_flag init_is_word_;
  mutable bool is_word_;
};

class PEGLIB_API CharacterClass : public Ope,
                       public std::enable_shared_from_this<CharacterClass> {
public:
  CharacterClass(const std::string &s, bool negated, bool ignore_case);

  CharacterClass(const std::vector<std::pair<char32_t, char32_t>> &ranges,
                 bool negated, bool ignore_case);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context &c, std::any &dt) const override;

  void accept(Visitor &v) override;

  friend struct ComputeFirstSet;
  friend struct GrammarBlob;
  friend struct detail::GrammarBlobImpl;

  bool is_ascii_only() const;
  const std::bitset<256> &ascii_bitset() const;

private:
  bool in_range(const std::pair<char32_t, char32_t> &range, char32_t cp) const;

  void setup_ascii_bitset();

  std::vector<std::pair<char32_t, char32_t>> ranges_;
  bool negated_;
  bool ignore_case_;
  std::bitset<256> ascii_bitset_;
  bool is_ascii_only_ = false;
};

class PEGLIB_API Character : public Ope, public std::enable_shared_from_this<Character> {
public:
  Character(char32_t ch);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context &c, std::any &dt) const override;

  void accept(Visitor &v) override;

  char32_t ch_;
};

class PEGLIB_API AnyCharacter : public Ope,
                     public std::enable_shared_from_this<AnyCharacter> {
public:
  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context &c, std::any &dt) const override;

  void accept(Visitor &v) override;
};

class PEGLIB_API CaptureScope : public Ope {
public:
  CaptureScope(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class PEGLIB_API Capture : public Ope {
public:
  using MatchAction = std::function<void(const char *s, size_t n, Context &c)>;

  Capture(const std::shared_ptr<Ope> &ope, MatchAction ma);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
  MatchAction match_action_;
};

class PEGLIB_API TokenBoundary : public Ope {
public:
  TokenBoundary(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class PEGLIB_API Ignore : public Ope {
public:
  Ignore(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context &c, std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

using Parser = std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                                    std::any &dt)>;

class PEGLIB_API User : public Ope {
public:
  User(Parser fn);
  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context &c, std::any &dt) const override;
  void accept(Visitor &v) override;
  std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                       std::any &dt)>
      fn_;
};

class PEGLIB_API WeakHolder : public Ope {
public:
  WeakHolder(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::weak_ptr<Ope> weak_;
};

class PEGLIB_API Holder : public Ope {
public:
  Holder(Definition *outer);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::any reduce(SemanticValues &vs, std::any &dt,
                  const std::any &predicate_data) const;

  const std::string &name() const;
  const std::string &trace_name() const;

  std::shared_ptr<Ope> ope_;
  Definition *outer_;
  mutable std::once_flag trace_name_init_;
  mutable std::string trace_name_;

  friend class Definition;
};

using Grammar = std::unordered_map<std::string, Definition>;

class PEGLIB_API Reference : public Ope, public std::enable_shared_from_this<Reference> {
public:
  Reference(const Grammar &grammar, const std::string &name, const char *s,
            bool is_macro, const std::vector<std::shared_ptr<Ope>> &args);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> get_core_operator() const;

  const Grammar &grammar_;
  const std::string name_;
  const char *s_;

  const bool is_macro_;
  const std::vector<std::shared_ptr<Ope>> args_;

  Definition *rule_;
  size_t iarg_;
};

class PEGLIB_API Whitespace : public Ope {
public:
  Whitespace(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class PEGLIB_API BackReference : public Ope {
public:
  BackReference(std::string &&name);

  BackReference(const std::string &name);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::string name_;
};

class PEGLIB_API PrecedenceClimbing : public Ope {
public:
  using BinOpeInfo = std::map<std::string_view, std::pair<size_t, char>>;

  PrecedenceClimbing(const std::shared_ptr<Ope> &atom,
                     const std::shared_ptr<Ope> &binop, const BinOpeInfo &info,
                     const Definition &rule);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> atom_;
  std::shared_ptr<Ope> binop_;
  BinOpeInfo info_;
  // Owned backing storage for info_ keys when this node is built by
  // GrammarBlob::deserialize. Grammars parsed from source leave this empty and
  // point info_ keys into the retained grammar text instead.
  std::vector<std::string> info_keys_;
  const Definition &rule_;

private:
  size_t parse_expression(const char *s, size_t n, SemanticValues &vs,
                          Context &c, std::any &dt, size_t min_prec) const;

  Definition &get_reference_for_binop(Context &c) const;
};

class PEGLIB_API Recovery : public Ope {
public:
  Recovery(const std::shared_ptr<Ope> &ope);

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class PEGLIB_API Cut : public Ope, public std::enable_shared_from_this<Cut> {
public:
  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context &c, std::any &dt) const override;

  void accept(Visitor &v) override;
};

/*
 * Factories
 */
template <typename... Args> std::shared_ptr<Ope> seq(Args &&...args) {
  return std::make_shared<Sequence>(static_cast<std::shared_ptr<Ope>>(args)...);
}

template <typename... Args> std::shared_ptr<Ope> cho(Args &&...args) {
  return std::make_shared<PrioritizedChoice>(
      false, static_cast<std::shared_ptr<Ope>>(args)...);
}

template <typename... Args> std::shared_ptr<Ope> cho4label_(Args &&...args) {
  return std::make_shared<PrioritizedChoice>(
      true, static_cast<std::shared_ptr<Ope>>(args)...);
}

PEGLIB_API std::shared_ptr<Ope> zom(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> oom(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> opt(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> rep(const std::shared_ptr<Ope> &ope, size_t min,
                                size_t max);

PEGLIB_API std::shared_ptr<Ope> apd(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> npd(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> dic(const std::vector<std::string> &v,
                                bool ignore_case);

PEGLIB_API std::shared_ptr<Ope> lit(std::string &&s);

PEGLIB_API std::shared_ptr<Ope> liti(std::string &&s);

PEGLIB_API std::shared_ptr<Ope> cls(const std::string &s);

PEGLIB_API std::shared_ptr<Ope>
cls(const std::vector<std::pair<char32_t, char32_t>> &ranges,
    bool ignore_case = false);

PEGLIB_API std::shared_ptr<Ope> ncls(const std::string &s);

PEGLIB_API std::shared_ptr<Ope>
ncls(const std::vector<std::pair<char32_t, char32_t>> &ranges,
     bool ignore_case = false);

PEGLIB_API std::shared_ptr<Ope> chr(char32_t dt);

PEGLIB_API std::shared_ptr<Ope> dot();

PEGLIB_API std::shared_ptr<Ope> csc(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> cap(const std::shared_ptr<Ope> &ope,
                                Capture::MatchAction ma);

PEGLIB_API std::shared_ptr<Ope> tok(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> ign(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope>
usr(std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                         std::any &dt)>
        fn);

PEGLIB_API std::shared_ptr<Ope> ref(const Grammar &grammar, const std::string &name,
                                const char *s, bool is_macro,
                                const std::vector<std::shared_ptr<Ope>> &args);

PEGLIB_API std::shared_ptr<Ope> wsp(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> bkr(std::string &&name);

PEGLIB_API std::shared_ptr<Ope> pre(const std::shared_ptr<Ope> &atom,
                                const std::shared_ptr<Ope> &binop,
                                const PrecedenceClimbing::BinOpeInfo &info,
                                const Definition &rule);

PEGLIB_API std::shared_ptr<Ope> rec(const std::shared_ptr<Ope> &ope);

PEGLIB_API std::shared_ptr<Ope> cut();

/*
 * Visitor
 */
struct PEGLIB_API Ope::Visitor {
  virtual ~Visitor();
  virtual void visit(Sequence &);
  virtual void visit(PrioritizedChoice &);
  virtual void visit(Repetition &);
  virtual void visit(AndPredicate &);
  virtual void visit(NotPredicate &);
  virtual void visit(Dictionary &);
  virtual void visit(LiteralString &);
  virtual void visit(CharacterClass &);
  virtual void visit(Character &);
  virtual void visit(AnyCharacter &);
  virtual void visit(CaptureScope &);
  virtual void visit(Capture &);
  virtual void visit(TokenBoundary &);
  virtual void visit(Ignore &);
  virtual void visit(User &);
  virtual void visit(WeakHolder &);
  virtual void visit(Holder &);
  virtual void visit(Reference &);
  virtual void visit(Whitespace &);
  virtual void visit(BackReference &);
  virtual void visit(PrecedenceClimbing &);
  virtual void visit(Recovery &);
  virtual void visit(Cut &);
};

struct TraversalVisitor;
struct TraceOpeName;
struct AssignIDToDefinition;
struct IsLiteralToken;
struct TokenChecker;
struct FindLiteralToken;
struct DetectLeftRecursion;
struct ComputeCanBeEmpty;
struct HasEmptyElement;
struct DetectInfiniteLoop;
struct ReferenceChecker;
struct LinkReferences;
struct FindReference;
struct ComputeFirstSet;
struct SetupFirstSets;

/*
 * Keywords
 */
PEGLIB_API extern const char *const WHITESPACE_DEFINITION_NAME;
PEGLIB_API extern const char *const WORD_DEFINITION_NAME;
PEGLIB_API extern const char *const RECOVER_DEFINITION_NAME;

/*
 * Definition
 */
class PEGLIB_API Definition {
public:
  struct Result {
    bool ret;
    bool recovered;
    size_t len;
    ErrorInfo error_info;
  };

  Definition();

  Definition(const Definition &rhs);

  Definition(const std::shared_ptr<Ope> &ope);

  operator std::shared_ptr<Ope>();

  Definition &operator<=(const std::shared_ptr<Ope> &ope);

  Result parse(const char *s, size_t n, const char *path = nullptr,
               Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const;

  Result parse(const char *s, const char *path = nullptr, Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const;

  Result parse(const char *s, size_t n, std::any &dt,
               const char *path = nullptr, Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const;

  Result parse(const char *s, std::any &dt, const char *path = nullptr,
               Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const;

  template <typename T>
  Result parse_and_get_value(const char *s, size_t n, T &val,
                             const char *path = nullptr, Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    SemanticValues vs;
    std::any dt;
    auto r = parse_core(s, n, vs, dt, path, log, error_reporter);
    if (r.ret && !vs.empty() && vs.front().has_value()) {
      val = std::any_cast<T>(vs[0]);
    }
    return r;
  }

  template <typename T>
  Result parse_and_get_value(const char *s, T &val, const char *path = nullptr,
                             Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    auto n = std::char_traits<char>::length(s);
    return parse_and_get_value(s, n, val, path, log, error_reporter);
  }

  template <typename T>
  Result parse_and_get_value(const char *s, size_t n, std::any &dt, T &val,
                             const char *path = nullptr, Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    SemanticValues vs;
    auto r = parse_core(s, n, vs, dt, path, log, error_reporter);
    if (r.ret && !vs.empty() && vs.front().has_value()) {
      val = std::any_cast<T>(vs[0]);
    }
    return r;
  }

  template <typename T>
  Result parse_and_get_value(const char *s, std::any &dt, T &val,
                             const char *path = nullptr, Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    auto n = std::char_traits<char>::length(s);
    return parse_and_get_value(s, n, dt, val, path, log, error_reporter);
  }

#if defined(__cpp_lib_char8_t)
  Result parse(const char8_t *s, size_t n, const char *path = nullptr,
               Log log = nullptr) const;

  Result parse(const char8_t *s, const char *path = nullptr,
               Log log = nullptr) const;

  Result parse(const char8_t *s, size_t n, std::any &dt,
               const char *path = nullptr, Log log = nullptr) const;

  Result parse(const char8_t *s, std::any &dt, const char *path = nullptr,
               Log log = nullptr) const;

  template <typename T>
  Result parse_and_get_value(const char8_t *s, size_t n, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), n, val, path,
                               log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), val, path,
                               log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, size_t n, std::any &dt, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), n, dt, val,
                               path, log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, std::any &dt, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), dt, val, path,
                               log);
  }
#endif

  void operator=(Action a);

  template <typename T> Definition &operator,(T fn) {
    operator=(fn);
    return *this;
  }

  Definition &operator~();

  void accept(Ope::Visitor &v);

  std::shared_ptr<Ope> get_core_operator() const;

  bool is_token() const;

  std::string name;
  const char *s_ = nullptr;
  std::pair<size_t, size_t> line_ = {1, 1};

  Predicate predicate;

  size_t id = 0;
  Action action;
  std::function<void(const Context &c, const char *s, size_t n, std::any &dt)>
      enter;
  std::function<void(const Context &c, const char *s, size_t n, size_t matchlen,
                     std::any &value, std::any &dt)>
      leave;
  bool ignoreSemanticValue = false;
  std::shared_ptr<Ope> whitespaceOpe;
  std::shared_ptr<Ope> wordOpe;
  bool enablePackratParsing = false;
  bool is_macro = false;
  std::vector<std::string> params;
  bool disable_action = false;
  bool is_left_recursive = false;
  bool can_be_empty = false;

  TracerEnter tracer_enter;
  TracerLeave tracer_leave;
  bool verbose_trace = false;
  TracerStartOrEnd tracer_start;
  TracerStartOrEnd tracer_end;

  std::string error_message;
  bool no_ast_opt = false;
  bool no_whitespace = false; // Disable %whitespace skipping inside this rule
                              // (like a token boundary, without capturing)
  std::string ast_name; // When non-empty, AST nodes produced by this rule carry
                        // this name/tag instead of the rule's own name

  bool eoi_check = true;

  // Per-rule packrat stats (optional, for profiling)
  mutable bool collect_packrat_stats = false;
  mutable std::vector<Context::PackratStats> packrat_stats_;

private:
  friend class Reference;
  friend class ParserGenerator;

  Definition &operator=(const Definition &rhs);
  Definition &operator=(Definition &&rhs);

  void initialize_definition_ids() const;

  void initialize_packrat_filter() const;

  Result parse_core(const char *s, size_t n, SemanticValues &vs, std::any &dt,
                    const char *path, Log log,
                    ErrorReporter error_reporter = nullptr) const;

  std::shared_ptr<Holder> holder_;
  mutable std::once_flag is_token_init_;
  mutable bool is_token_ = false;
  mutable std::once_flag assign_id_to_definition_init_;
  mutable std::once_flag definition_ids_init_;
  mutable std::unordered_map<void *, size_t> definition_ids_;
  mutable std::once_flag packrat_filter_init_;
  mutable std::vector<int32_t> packrat_index_; // def_id -> cache slot or -1
  mutable size_t packrat_cached_count_ = 0;
};


/*-----------------------------------------------------------------------------
 *  Grammar serialization
 *
 *  Serialize a compiled Grammar (the operator tree) to a byte blob and back,
 *  letting an application skip the meta-parse on startup by embedding a
 *  prebuilt blob. Structure only: semantic callbacks (actions / enter / leave /
 *  predicate, attached by enable_ast() etc.) are NOT serialized and must be
 *  re-applied after deserialize. References resolve by name (no pointer fixup);
 *  first-sets and keyword guards are recomputed on load (O(N)). The
 *  `precedence` instruction is supported (its operator table is structural).
 *  Grammars using the `User` operator or a Capture with a match action are
 *  rejected. The blob is specific to this peglib version's layout.
 *---------------------------------------------------------------------------*/

struct PEGLIB_API GrammarBlob {
  static std::vector<uint8_t> serialize(const Grammar &g,
                                        const std::string &start);
  static std::shared_ptr<Grammar> deserialize(const std::vector<uint8_t> &blob,
                                              std::string &start_out);
};

/*-----------------------------------------------------------------------------
 *  PEG parser generator
 *---------------------------------------------------------------------------*/

using Rules = std::unordered_map<std::string, std::shared_ptr<Ope>>;

/*-----------------------------------------------------------------------------
 *  AST
 *---------------------------------------------------------------------------*/

template <typename Annotation> struct AstBase : public Annotation {
  AstBase(const char *path, size_t line, size_t column, const char *name,
          const std::vector<std::shared_ptr<AstBase>> &nodes,
          size_t position = 0, size_t length = 0, size_t choice_count = 0,
          size_t choice = 0, bool preserve_position = false)
      : path(path ? path : ""), line(line), column(column), name(name),
        position(position), length(length), choice_count(choice_count),
        choice(choice), original_name(name),
        original_choice_count(choice_count), original_choice(choice),
        tag(str2tag(name)), original_tag(tag), is_token(false),
        preserve_position(preserve_position), nodes(nodes) {}

  AstBase(const char *path, size_t line, size_t column, const char *name,
          const std::string_view &token, size_t position = 0, size_t length = 0,
          size_t choice_count = 0, size_t choice = 0,
          bool preserve_position = false)
      : path(path ? path : ""), line(line), column(column), name(name),
        position(position), length(length), choice_count(choice_count),
        choice(choice), original_name(name),
        original_choice_count(choice_count), original_choice(choice),
        tag(str2tag(name)), original_tag(tag), is_token(true),
        preserve_position(preserve_position), token(token) {}

  AstBase(const AstBase &ast, const char *original_name, size_t position = 0,
          size_t length = 0, size_t original_choice_count = 0,
          size_t original_choice = 0)
      : path(ast.path), line(ast.line), column(ast.column), name(ast.name),
        position(position), length(length), choice_count(ast.choice_count),
        choice(ast.choice), original_name(original_name),
        original_choice_count(original_choice_count),
        original_choice(original_choice), tag(ast.tag),
        original_tag(str2tag(original_name)), is_token(ast.is_token),
        preserve_position(ast.preserve_position), token(ast.token),
        nodes(ast.nodes), parent(ast.parent) {}

  const std::string path;
  const size_t line = 1;
  const size_t column = 1;

  const std::string name;
  size_t position;
  size_t length;
  const size_t choice_count;
  const size_t choice;
  const std::string original_name;
  const size_t original_choice_count;
  const size_t original_choice;
  const unsigned int tag;
  const unsigned int original_tag;

  const bool is_token;
  const bool preserve_position;
  const std::string_view token;

  std::vector<std::shared_ptr<AstBase<Annotation>>> nodes;
  std::weak_ptr<AstBase<Annotation>> parent;

  std::string token_to_string() const {
    assert(is_token);
    return std::string(token);
  }

  template <typename T> T token_to_number() const {
    return token_to_number_<T>(token);
  }
};

template <typename T>
void ast_to_s_core(const std::shared_ptr<T> &ptr, std::string &s, int level,
                   std::function<std::string(const T &ast, int level)> fn) {
  const auto &ast = *ptr;
  for (auto i = 0; i < level; i++) {
    s += "  ";
  }
  auto name = ast.original_name;
  if (ast.original_choice_count > 0) {
    name += "/" + std::to_string(ast.original_choice);
  }
  if (ast.name != ast.original_name) { name += "[" + ast.name + "]"; }
  if (ast.is_token) {
    s += "- " + name + " (";
    s += ast.token;
    s += ")\n";
  } else {
    s += "+ " + name + "\n";
  }
  if (fn) { s += fn(ast, level + 1); }
  for (const auto &node : ast.nodes) {
    ast_to_s_core(node, s, level + 1, fn);
  }
}

template <typename T>
std::string
ast_to_s(const std::shared_ptr<T> &ptr,
         std::function<std::string(const T &ast, int level)> fn = nullptr) {
  std::string s;
  ast_to_s_core(ptr, s, 0, fn);
  return s;
}

struct AstOptimizer {
  AstOptimizer(bool mode, const std::vector<std::string> &rules = {})
      : mode_(mode), rules_(rules) {}

  template <typename T>
  std::shared_ptr<T> optimize(std::shared_ptr<T> original,
                              std::shared_ptr<T> parent = nullptr) {
    auto found = false;
    for (const auto &rule : rules_) {
      if (rule == original->name) {
        found = true;
        break;
      }
    }
    auto opt = mode_ ? !found : found;

    if (opt && original->nodes.size() == 1) {
      auto child = optimize(original->nodes[0], parent);
      auto pos =
          child->preserve_position ? child->position : original->position;
      auto len = child->preserve_position ? child->length : original->length;
      auto ast = std::make_shared<T>(*child, original->name.data(), pos, len,
                                     original->choice_count, original->choice);
      for (auto &node : ast->nodes) {
        node->parent = ast;
      }
      return ast;
    }

    auto ast = std::make_shared<T>(*original);
    ast->parent = parent;
    ast->nodes.clear();
    for (const auto &node : original->nodes) {
      auto child = optimize(node, ast);
      ast->nodes.push_back(child);
    }
    return ast;
  }

private:
  const bool mode_;
  const std::vector<std::string> rules_;
};

struct EmptyType {};
using Ast = AstBase<EmptyType>;

template <typename T = Ast> void add_ast_action(Definition &rule) {
  rule.action = [&](const SemanticValues &vs) {
    auto line = vs.line_info();

    // `{ ast_name: X }` overrides the node's name/tag (falls back to the
    // rule's own name when unset).
    const char *node_name =
        rule.ast_name.empty() ? rule.name.data() : rule.ast_name.data();

    if (rule.is_token()) {
      return std::make_shared<T>(
          vs.path, line.first, line.second, node_name, vs.token(),
          std::distance(vs.ss, vs.sv().data()), vs.sv().length(),
          vs.choice_count(), vs.choice(), rule.no_ast_opt);
    }

    auto ast = std::make_shared<T>(vs.path, line.first, line.second, node_name,
                                   vs.transform<std::shared_ptr<T>>(),
                                   std::distance(vs.ss, vs.sv().data()),
                                   vs.sv().length(), vs.choice_count(),
                                   vs.choice(), rule.no_ast_opt);

    for (auto &node : ast->nodes) {
      node->parent = ast;
    }
    return ast;
  };
}

#define PEG_EXPAND(...) __VA_ARGS__
#define PEG_CONCAT(a, b) a##b
#define PEG_CONCAT2(a, b) PEG_CONCAT(a, b)

#define PEG_PICK(                                                              \
    a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, \
    a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, \
    a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, \
    a47, a48, a49, a50, a51, a52, a53, a54, a55, a56, a57, a58, a59, a60, a61, \
    a62, a63, a64, a65, a66, a67, a68, a69, a70, a71, a72, a73, a74, a75, a76, \
    a77, a78, a79, a80, a81, a82, a83, a84, a85, a86, a87, a88, a89, a90, a91, \
    a92, a93, a94, a95, a96, a97, a98, a99, a100, ...)                         \
  a100

#define PEG_COUNT(...)                                                         \
  PEG_EXPAND(PEG_PICK(                                                         \
      __VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87,    \
      86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69,  \
      68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51,  \
      50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,  \
      32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,  \
      14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define PEG_DEF_1(r)                                                           \
  peg::Definition r;                                                           \
  r.name = #r;                                                                 \
  peg::add_ast_action(r);

#define PEG_DEF_2(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_1(__VA_ARGS__))
#define PEG_DEF_3(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_2(__VA_ARGS__))
#define PEG_DEF_4(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_3(__VA_ARGS__))
#define PEG_DEF_5(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_4(__VA_ARGS__))
#define PEG_DEF_6(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_5(__VA_ARGS__))
#define PEG_DEF_7(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_6(__VA_ARGS__))
#define PEG_DEF_8(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_7(__VA_ARGS__))
#define PEG_DEF_9(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_8(__VA_ARGS__))
#define PEG_DEF_10(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_9(__VA_ARGS__))
#define PEG_DEF_11(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_10(__VA_ARGS__))
#define PEG_DEF_12(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_11(__VA_ARGS__))
#define PEG_DEF_13(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_12(__VA_ARGS__))
#define PEG_DEF_14(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_13(__VA_ARGS__))
#define PEG_DEF_15(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_14(__VA_ARGS__))
#define PEG_DEF_16(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_15(__VA_ARGS__))
#define PEG_DEF_17(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_16(__VA_ARGS__))
#define PEG_DEF_18(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_17(__VA_ARGS__))
#define PEG_DEF_19(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_18(__VA_ARGS__))
#define PEG_DEF_20(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_19(__VA_ARGS__))
#define PEG_DEF_21(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_20(__VA_ARGS__))
#define PEG_DEF_22(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_21(__VA_ARGS__))
#define PEG_DEF_23(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_22(__VA_ARGS__))
#define PEG_DEF_24(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_23(__VA_ARGS__))
#define PEG_DEF_25(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_24(__VA_ARGS__))
#define PEG_DEF_26(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_25(__VA_ARGS__))
#define PEG_DEF_27(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_26(__VA_ARGS__))
#define PEG_DEF_28(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_27(__VA_ARGS__))
#define PEG_DEF_29(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_28(__VA_ARGS__))
#define PEG_DEF_30(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_29(__VA_ARGS__))
#define PEG_DEF_31(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_30(__VA_ARGS__))
#define PEG_DEF_32(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_31(__VA_ARGS__))
#define PEG_DEF_33(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_32(__VA_ARGS__))
#define PEG_DEF_34(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_33(__VA_ARGS__))
#define PEG_DEF_35(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_34(__VA_ARGS__))
#define PEG_DEF_36(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_35(__VA_ARGS__))
#define PEG_DEF_37(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_36(__VA_ARGS__))
#define PEG_DEF_38(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_37(__VA_ARGS__))
#define PEG_DEF_39(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_38(__VA_ARGS__))
#define PEG_DEF_40(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_39(__VA_ARGS__))
#define PEG_DEF_41(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_40(__VA_ARGS__))
#define PEG_DEF_42(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_41(__VA_ARGS__))
#define PEG_DEF_43(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_42(__VA_ARGS__))
#define PEG_DEF_44(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_43(__VA_ARGS__))
#define PEG_DEF_45(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_44(__VA_ARGS__))
#define PEG_DEF_46(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_45(__VA_ARGS__))
#define PEG_DEF_47(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_46(__VA_ARGS__))
#define PEG_DEF_48(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_47(__VA_ARGS__))
#define PEG_DEF_49(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_48(__VA_ARGS__))
#define PEG_DEF_50(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_49(__VA_ARGS__))
#define PEG_DEF_51(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_50(__VA_ARGS__))
#define PEG_DEF_52(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_51(__VA_ARGS__))
#define PEG_DEF_53(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_52(__VA_ARGS__))
#define PEG_DEF_54(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_53(__VA_ARGS__))
#define PEG_DEF_55(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_54(__VA_ARGS__))
#define PEG_DEF_56(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_55(__VA_ARGS__))
#define PEG_DEF_57(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_56(__VA_ARGS__))
#define PEG_DEF_58(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_57(__VA_ARGS__))
#define PEG_DEF_59(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_58(__VA_ARGS__))
#define PEG_DEF_60(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_59(__VA_ARGS__))
#define PEG_DEF_61(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_60(__VA_ARGS__))
#define PEG_DEF_62(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_61(__VA_ARGS__))
#define PEG_DEF_63(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_62(__VA_ARGS__))
#define PEG_DEF_64(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_63(__VA_ARGS__))
#define PEG_DEF_65(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_64(__VA_ARGS__))
#define PEG_DEF_66(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_65(__VA_ARGS__))
#define PEG_DEF_67(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_66(__VA_ARGS__))
#define PEG_DEF_68(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_67(__VA_ARGS__))
#define PEG_DEF_69(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_68(__VA_ARGS__))
#define PEG_DEF_70(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_69(__VA_ARGS__))
#define PEG_DEF_71(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_70(__VA_ARGS__))
#define PEG_DEF_72(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_71(__VA_ARGS__))
#define PEG_DEF_73(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_72(__VA_ARGS__))
#define PEG_DEF_74(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_73(__VA_ARGS__))
#define PEG_DEF_75(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_74(__VA_ARGS__))
#define PEG_DEF_76(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_75(__VA_ARGS__))
#define PEG_DEF_77(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_76(__VA_ARGS__))
#define PEG_DEF_78(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_77(__VA_ARGS__))
#define PEG_DEF_79(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_78(__VA_ARGS__))
#define PEG_DEF_80(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_79(__VA_ARGS__))
#define PEG_DEF_81(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_80(__VA_ARGS__))
#define PEG_DEF_82(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_81(__VA_ARGS__))
#define PEG_DEF_83(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_82(__VA_ARGS__))
#define PEG_DEF_84(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_83(__VA_ARGS__))
#define PEG_DEF_85(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_84(__VA_ARGS__))
#define PEG_DEF_86(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_85(__VA_ARGS__))
#define PEG_DEF_87(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_86(__VA_ARGS__))
#define PEG_DEF_88(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_87(__VA_ARGS__))
#define PEG_DEF_89(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_88(__VA_ARGS__))
#define PEG_DEF_90(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_89(__VA_ARGS__))
#define PEG_DEF_91(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_90(__VA_ARGS__))
#define PEG_DEF_92(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_91(__VA_ARGS__))
#define PEG_DEF_93(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_92(__VA_ARGS__))
#define PEG_DEF_94(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_93(__VA_ARGS__))
#define PEG_DEF_95(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_94(__VA_ARGS__))
#define PEG_DEF_96(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_95(__VA_ARGS__))
#define PEG_DEF_97(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_96(__VA_ARGS__))
#define PEG_DEF_98(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_97(__VA_ARGS__))
#define PEG_DEF_99(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_98(__VA_ARGS__))
#define PEG_DEF_100(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_99(__VA_ARGS__))

#define AST_DEFINITIONS(...)                                                   \
  PEG_EXPAND(PEG_CONCAT2(PEG_DEF_, PEG_COUNT(__VA_ARGS__))(__VA_ARGS__))

/*-----------------------------------------------------------------------------
 *  parser
 *---------------------------------------------------------------------------*/

class PEGLIB_API parser {
public:
  parser() = default;

  parser(const char *s, size_t n, const Rules &rules,
         std::string_view start = {});

  parser(const char *s, size_t n, std::string_view start = {});

  parser(std::string_view sv, const Rules &rules, std::string_view start = {});

  parser(std::string_view sv, std::string_view start = {});

#if defined(__cpp_lib_char8_t)
  parser(std::u8string_view sv, const Rules &rules,
         std::string_view start = {});

  parser(std::u8string_view sv, std::string_view start = {});
#endif

  operator bool() const;

  bool load_grammar(const char *s, size_t n, const Rules &rules,
                    std::string_view start = {});

  bool load_grammar(const char *s, size_t n, std::string_view start = {});

  bool load_grammar(std::string_view sv, const Rules &rules,
                    std::string_view start = {});

  bool load_grammar(std::string_view sv, std::string_view start = {});

  // Serialize the loaded grammar to a portable byte blob (see GrammarBlob).
  // Semantic callbacks are not included; throws if the grammar is not
  // serializable (uses the `User` operator or a Capture with a match action).
  std::vector<uint8_t> serialize_grammar() const;

  // Load a grammar from a blob produced by serialize_grammar() / GrammarBlob,
  // skipping the meta-parse. Re-apply enable_ast() etc. afterwards as needed.
  bool load_blob(const std::vector<uint8_t> &blob);

  bool parse_n(const char *s, size_t n, const char *path = nullptr) const;

  bool parse_n(const char *s, size_t n, std::any &dt,
               const char *path = nullptr) const;

  template <typename T>
  bool parse_n(const char *s, size_t n, T &val,
               const char *path = nullptr) const {
    if (grammar_ != nullptr) {
      const auto &rule = (*grammar_)[start_];
      auto result =
          rule.parse_and_get_value(s, n, val, path, log_, error_reporter_);
      return post_process(s, n, result);
    }
    return false;
  }

  template <typename T>
  bool parse_n(const char *s, size_t n, std::any &dt, T &val,
               const char *path = nullptr) const {
    if (grammar_ != nullptr) {
      const auto &rule = (*grammar_)[start_];
      auto result =
          rule.parse_and_get_value(s, n, dt, val, path, log_, error_reporter_);
      return post_process(s, n, result);
    }
    return false;
  }

  bool parse(std::string_view sv, const char *path = nullptr) const;

  bool parse(std::string_view sv, std::any &dt,
             const char *path = nullptr) const;

  template <typename T>
  bool parse(std::string_view sv, T &val, const char *path = nullptr) const {
    return parse_n(sv.data(), sv.size(), val, path);
  }

  template <typename T>
  bool parse(std::string_view sv, std::any &dt, T &val,
             const char *path = nullptr) const {
    return parse_n(sv.data(), sv.size(), dt, val, path);
  }

#if defined(__cpp_lib_char8_t)
  bool parse(std::u8string_view sv, const char *path = nullptr) const;

  bool parse(std::u8string_view sv, std::any &dt,
             const char *path = nullptr) const;

  template <typename T>
  bool parse(std::u8string_view sv, T &val, const char *path = nullptr) const {
    return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), val,
                   path);
  }

  template <typename T>
  bool parse(std::u8string_view sv, std::any &dt, T &val,
             const char *path = nullptr) const {
    return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), dt,
                   val, path);
  }
#endif

  Definition &operator[](const char *s);

  const Definition &operator[](const char *s) const;

  const Grammar &get_grammar() const;

  void disable_eoi_check();

  void enable_left_recursion(bool enable = true);

  void enable_packrat_parsing();

  void enable_trace(TracerEnter tracer_enter, TracerLeave tracer_leave);

  void enable_trace(TracerEnter tracer_enter, TracerLeave tracer_leave,
                    TracerStartOrEnd tracer_start,
                    TracerStartOrEnd tracer_end);

  void set_verbose_trace(bool verbose_trace);

  template <typename T = Ast> parser &enable_ast() {
    for (auto &[_, rule] : *grammar_) {
      if (!rule.action) { add_ast_action<T>(rule); }
    }
    return *this;
  }

  template <typename T>
  std::shared_ptr<T> optimize_ast(std::shared_ptr<T> ast,
                                  bool opt_mode = true) const {
    return AstOptimizer(opt_mode, get_no_ast_opt_rules()).optimize(ast);
  }

  void set_logger(Log log);

  // Receive structured error information instead of (or in addition to) the
  // formatted string passed to the logger.
  void set_error_reporter(ErrorReporter reporter);

  void set_logger(
      std::function<void(size_t line, size_t col, const std::string &msg)>
          log);

private:
  bool post_process(const char *s, size_t n, Definition::Result &r) const;

  std::vector<std::string> get_no_ast_opt_rules() const;

  std::shared_ptr<Grammar> grammar_;
  std::string start_;
  bool enableLeftRecursion_ = true;
  bool enablePackratParsing_ = false;
  Log log_;
  ErrorReporter error_reporter_;
};

PEGLIB_API void enable_tracing(parser &parser, std::ostream &os);
PEGLIB_API void enable_profiling(parser &parser, std::ostream &os);
} // namespace peg
